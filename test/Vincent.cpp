#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp>
#include <iostream>
#include <mutex>
#include <memory>
#include <resources/disparity/StereoSegmentation.h>
#include <resources/disparity/DirectStereoBlockMatcher.h>


////just for debugging
std::string type2str(int type) {
  std::string r;

  uchar depth = type & CV_MAT_DEPTH_MASK;
  uchar chans = 1 + (type >> CV_CN_SHIFT);

  switch ( depth ) {
    case CV_8U:  r = "8U"; break;
    case CV_8S:  r = "8S"; break;
    case CV_16U: r = "16U"; break;
    case CV_16S: r = "16S"; break;
    case CV_32S: r = "32S"; break;
    case CV_32F: r = "32F"; break;
    case CV_64F: r = "64F"; break;
    default:     r = "User"; break;
  }

  r += "C";
  r += (chans+'0');

  return r;
}

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

int main( int argc, char** argv )
{

    // Load Data
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

    // Testing the algorithm!
    std::cout <<"Running StereoBM\n";
    cv::Mat disparity, Ndisparity;
    auto start1 = std::chrono::high_resolution_clock::now();
    stereo->compute(remapL,remapR, disparity);
	auto end1 = std::chrono::high_resolution_clock::now();
    cv::normalize(disparity, Ndisparity, 0, 1, cv::NORM_MINMAX, CV_64FC1);
    //cv::imshow("Disparity", Ndisparity);

    std::cout << "Segmenting Image\n";
    int segments = 4;
    cv::Size_<int> segmentedSize(imageSize.width / segments, imageSize.height / segments);
    cv::Mat disparityTest(imageSize.height, imageSize.width, CV_64FC1);  //in yours this is the opposite of image size. why? as in width are height are switched

    std::cout << "Image Size:     " << imageSize << "\n";
    std::cout << "Segmented Size: " << segmentedSize << "\n";
    std::cout << "Disparity Size: " << disparityTest.rows << " , " << disparityTest.cols << "\n";

//    for(int yInd = 0; yInd < segments; yInd++){
//    	for(int xInd = 0; xInd < segments; xInd++){
//    		auto transfer = cv::Rect2i{xInd*segmentedSize.width, yInd*segmentedSize.height, segmentedSize.width, segmentedSize.height};
//    		auto segmentWindow = calculateImageSizeForSegment(cv::Rect2i{0, 0, imageSize.width, imageSize.height}, transfer, numDisparities, blockSize);
//
//            cv::Mat segment;
//    		stereo->compute(cv::Mat(remapL, segmentWindow.originalImageView), cv::Mat(remapR, segmentWindow.originalImageView), segment);
//    		cv::Mat(segment, segmentWindow.disparityImageView).copyTo(cv::Mat(disparityTest, transfer));
//        }
//    }
	auto threadedMatcher = sbm::DirectStereoBlockMatcher();
	auto start2 = std::chrono::high_resolution_clock::now();
	threadedMatcher.doSBM(remapL, remapR, disparityTest, numDisparities, blockSize);
	auto end2 = std::chrono::high_resolution_clock::now();
	fprintf(stdout, "Before: %ldns\n", std::chrono::duration_cast<std::chrono::nanoseconds>(end1 - start1).count());
	fprintf(stdout, "After:  %ldns\n", std::chrono::duration_cast<std::chrono::nanoseconds>(end2 - start2).count());

    cv::Mat NdisparityTest , output;
    cv::normalize(disparityTest, NdisparityTest, 0, 1, cv::NORM_MINMAX, CV_64FC1);
    
    cv::bitwise_xor(Ndisparity, NdisparityTest, output);        
    if(cv::countNonZero(output) > 0){
    	std::mutex mutex;
    	
    	std::unique_lock<std::mutex> lk(mutex);
    	// do stuff
    	
        std::cout<< "False";
    }
    else{
        std::cout <<"True";
    }
    
    // for xInd in range(0, segments):
    // 	cv2.line(disparityTest, (0, int(segmentedSize[1]) * xInd), (imageSize[0], int(segmentedSize[1]) * xInd), 255, 1)
    // for yInd in range(0, segments):
    // 	cv2.line(disparityTest, (int(segmentedSize[0]) * yInd, 0), (int(segmentedSize[0]) * yInd, imageSize[1]), 255, 1)

    cv::imshow("Disparity Test", NdisparityTest);
    cv::waitKey(0);
//    cv2.imshow('Left', remapL)
//    cv2.imshow('Right', remapR)
//    cv2.waitKey()
//    cv2.destroyAllWindows()
    
    return 0;
}











