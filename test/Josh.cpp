#include <cstdio>
#include <resources/protobuf/ProtobufStream.h>
#include <resources/protobuf/ProtobufWrapper.h>
#include <resources/networking/cluster/ClusterEndpoint.h>
#include <resources/networking/cluster/ClusterManager.h>
#include <messages.pb.h>

#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp>

#include <string>
#include <thread>
#include <memory>
#include <queue>
#include <vector>
#include <unistd.h>

using namespace networking::protobuf;
using namespace networking::cluster;

template <typename T, int r, int c>
cv::Mat readInMatrix(std::string path){
	FILE *arr;
	double k[3][3]={0.0};
	arr = fopen(path.c_str(),"r");
	int i,j;
	cv::Matx<T, r, c> m;
	for(i=0 ;i<3; i++)
	{
		for(j=0; j<3; j++)
		{
			fscanf(arr, "%lf",&k[i][j]);
			m(i,j) = k[i][j];
		}
	}
	return cv::Mat(m);
}

int main(int argc, char * argv[]) {
	GOOGLE_PROTOBUF_VERIFY_VERSION;
	
	auto manager = std::make_shared<ClusterManager>();
	std::queue<std::vector<uint8_t>> aInboundMessages;
	std::queue<std::vector<uint8_t>> bInboundMessages;
	manager->setLocalServerPort(5000);
	auto a = manager->createEndpoint({"127.0.0.1", 0}, [&](auto && msg) { bInboundMessages.emplace(msg); });
	auto b = manager->createEndpoint({"127.0.0.1", 5001}, [&](auto && msg) { aInboundMessages.emplace(msg); });
	
	auto t = std::thread([&]() {
		std::cout << "loading data...\n";
		
		cv::Mat imgL = cv::imread("../GroundTest/12_Dec_19/approach1/EO/3L_eo.bmp", 0);
		cv::Mat imgR = cv::imread("../GroundTest/12_Dec_19/approach1/EO/3R_eo.bmp", 0);
		cv::Mat mtxL = readInMatrix<double, 3, 3>("../GroundTest/perfect_calibrations_4K/M1.dat");
		cv::Mat mtxR = readInMatrix<double, 3, 3>("../GroundTest/perfect_calibrations_4K/M2.dat");
		cv::Mat distL = readInMatrix<double, 5, 1>("../GroundTest/perfect_calibrations_4K/d1.dat");
		cv::Mat distR = readInMatrix<double, 5, 1>("../GroundTest/perfect_calibrations_4K/d2.dat");
		cv::Mat R = readInMatrix<double, 3, 3>("../GroundTest/perfect_calibrations_4K/R.dat");
		cv::Mat T = readInMatrix<double, 3, 1>("../GroundTest/perfect_calibrations_4K/T.dat");
		
		// One time operations to fix the stereo images
		std::cout << "Initializing StereoBM...\n";
		cv::Size_<int> imageSize(imgL.cols, imgL.rows);
		cv::Mat R1(3, 3, CV_64FC1), R2(3, 3, CV_64FC1), P1(3, 4, CV_64FC1), P2(3, 4, CV_64FC1), Q(4, 4, CV_64FC1);
		cv::stereoRectify(mtxL, distL, mtxR, distR, imageSize, R, T, R1, R2, P1, P2, Q);
		cv::Mat mapLx, mapLy;
		cv::initUndistortRectifyMap(mtxL, distL, R1, P1, imageSize, CV_16SC2, mapLx, mapLy);
		cv::Mat mapRx, mapRy;
		cv::initUndistortRectifyMap(mtxR, distR, R2, P2, imageSize, CV_16SC2, mapRx, mapRy);
		int numDisparities=256;
		int blockSize=21;
		auto stereo = cv::StereoBM::create(numDisparities, blockSize);
		
		// Fix the individual image single-threaded
		std::cout << "Fixing input images...\n";
		cv::Mat remapL, remapR;
		cv::remap(imgL, remapL, mapLx, mapLy, cv::INTER_LINEAR);
		cv::remap(imgR, remapR, mapRx, mapRy, cv::INTER_LINEAR);
		
		cv::Mat disparity(imageSize, CV_16S), disparityNormalized;
		manager->getSBM()->doSBM(remapL, remapR, disparity, numDisparities, blockSize);
		cv::normalize(disparity, disparityNormalized, 0, 1, cv::NORM_MINMAX, CV_64FC1);
		cv::imshow("Disparity", disparityNormalized);
		cv::waitKey(0);
	});
	
	usleep(500'000);
	while (!aInboundMessages.empty() || !bInboundMessages.empty()) {
		while (!aInboundMessages.empty()) {
			auto rx = aInboundMessages.front();
			aInboundMessages.pop();
			a->onDataReceived(rx, rx.size());
		}
		while (!bInboundMessages.empty()) {
			auto rx = bInboundMessages.front();
			bInboundMessages.pop();
			b->onDataReceived(rx, rx.size());
		}
	}
	t.join();
	return 0;
}
