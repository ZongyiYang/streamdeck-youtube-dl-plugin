#pragma once

#include "CurlUtils.hpp"
#include <string>

namespace redditdlutils
{
	void getRedditImage(const std::string& url, const std::string& outputFolder, curlutils::MemoryStruct chunk);
}