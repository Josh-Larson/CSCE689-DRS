#pragma once

#include "StereoBlockMatcher.h"

#include <CTPL/ctpl.h>

#include <utility>
#include <vector>
#include <numeric>
#include <memory>

namespace sbm {

class AggregatedStereoBlockMatcher : public StereoBlockMatcher {
	// TODO: Add some form of thread-safety
	std::vector<std::shared_ptr<StereoBlockMatcher>> matchers;
	ctpl::thread_pool threadPool;
	
	public:
	AggregatedStereoBlockMatcher() = default;
	AggregatedStereoBlockMatcher(const AggregatedStereoBlockMatcher & copy) : matchers(copy.matchers) {}
	AggregatedStereoBlockMatcher(AggregatedStereoBlockMatcher && move) noexcept : matchers(std::move(move.matchers)) {}
	explicit AggregatedStereoBlockMatcher(std::vector<std::shared_ptr<StereoBlockMatcher>> l) : matchers(std::move(l)), threadPool(matchers.size()) {}
	~AggregatedStereoBlockMatcher() override = default;
	
	void doSBM(const cv::Mat &leftImage, const cv::Mat &rightImage, cv::Mat &disparityMap, int numDisparities, int blockSize) noexcept override;
	
	[[nodiscard]] double getComplexity() const noexcept override { return getTotalChildComplexity() / (matchers.size() * matchers.size()); }
	
	protected:
	void addChild(std::shared_ptr<StereoBlockMatcher> matcher) { matchers.emplace_back(std::move(matcher)); threadPool.resize(matchers.size()); };
	// TODO: Create children removers
	
	private:
	/** Accumulates (Sums) all of the childrens' complexity */
	[[nodiscard]] double getTotalChildComplexity() const noexcept { return std::accumulate(matchers.begin(), matchers.end(), 0.0, [](const double val, const auto & next) { return val + next->getComplexity(); }); }
};

} // namespace sbm
