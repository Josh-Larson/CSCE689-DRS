#pragma once

#include "AggregatedStereoBlockMatcher.h"
#include "DirectStereoBlockMatcher.h"

#include <CTPL/ctpl.h>
#include <functional>

namespace sbm {

class ThreadedStereoBlockMatcher : public AggregatedStereoBlockMatcher {
	const unsigned int threadCount = std::thread::hardware_concurrency();
	
	public:
	ThreadedStereoBlockMatcher() {
		for (unsigned int i = 0; i < threadCount; i++) {
			addChild(std::make_unique<DirectStereoBlockMatcher>());
		}
	}
	~ThreadedStereoBlockMatcher() override = default;
	
};

} // namespace sbm
