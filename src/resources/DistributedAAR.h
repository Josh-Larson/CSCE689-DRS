#pragma once

#include <boost/asio.hpp>
#include <resources/networking/server.h>
#include <resources/networking/connection.h>
#include <resources/networking/cluster/ClusterManager.h>
#include <resources/disparity/StereoBlockMatcher.h>

#include <memory>

class DistributedAAR : public sbm::StereoBlockMatcher {
	std::shared_ptr<networking::cluster::ClusterManager> clusterManager = std::make_shared<networking::cluster::ClusterManager>();
	boost::asio::io_context                              ioContext      = {};
	std::shared_ptr<server>                              serv           = nullptr;
	std::shared_ptr<connection>                          conn           = nullptr;
	std::thread                                          ioThread       = std::thread(std::bind(&DistributedAAR::run, this));
	std::shared_ptr<sbm::StereoBlockMatcher>             localMatcher   = nullptr;
	
	public:
	explicit DistributedAAR(int serverPort = 30781, const std::string& remoteAddress = "", int remotePort = 30781);
	~DistributedAAR() override;
	
	void waitForServer();
	
	void doSBM(const cv::Mat &leftImage, const cv::Mat &rightImage, cv::Mat &disparityMap, int numDisparities, int blockSize) noexcept override;
	[[nodiscard]] double getComplexity() const noexcept override;
	
	private:
	void run();
};
