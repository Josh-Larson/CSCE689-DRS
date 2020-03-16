#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <resources/networking/server.h>
#include <resources/networking/connection.h>

#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp>
#include <resources/sbm/DirectStereoBlockMatcher.h>

constexpr uint16_t DEFAULT_PORT = 30781;

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

int main(int argc, char *argv[]) {
	auto serverPort = ((argc >= 2) ? uint16_t(std::stoi(argv[1])) : DEFAULT_PORT);
	auto clientPort = std::to_string(((argc >= 4) ? uint16_t(std::stoi(argv[3])) : DEFAULT_PORT));
	auto io = boost::asio::io_context();
	auto clusterManager = std::make_shared<networking::cluster::ClusterManager>();
	auto serv = server(io, serverPort, clusterManager);
	auto client = ((argc >= 3) ? connection::create(io, clusterManager, argv[2], clientPort.c_str()) : nullptr);
	std::thread t;
	if (client != nullptr) {
		t = std::thread([&]() {
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
			sbm::AggregatedStereoBlockMatcher({
				clusterManager->getSBM(),
				std::make_shared<sbm::DirectStereoBlockMatcher>()
			}).doSBM(remapL, remapR, disparity, numDisparities, blockSize);
			cv::normalize(disparity, disparityNormalized, 0, 1, cv::NORM_MINMAX, CV_64FC1);
			cv::imshow("Disparity", disparityNormalized);
			cv::waitKey(0);
			cv::destroyAllWindows();
		});
	}
	io.run();
	return 0;
}
