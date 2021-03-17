//==============================================================================
/**
@file       RedditDlUtils.h
@brief      Utility functions for downloading from reddit
@copyright  (c) 2021, Zongyi Yang
**/
//==============================================================================

#pragma once

#include "CurlUtils.hpp"
#include <string>

namespace redditdlutils
{
	/**
	 * Download image from reddit using curl
	 *
	 * @param[in] url the url to the reddit post
	 * @param[in] outputFolder the output location
	 * @param[in] chunk the memory chunk required by curl
	 * @throws runtime_error if could not read url for json, json::exception on bad json parse, invalid_argument if reddit content is not of image type
	 */
	void getRedditImage(const std::string& url, const std::string& outputFolder, curlutils::MemoryStruct chunk);
}