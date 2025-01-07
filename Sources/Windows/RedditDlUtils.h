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
	 * @throws runtime_error if could not read url for json or download image, json::exception on bad json parse, invalid_argument if reddit content is not of image type
	 */
	void downloadRedditContent(const std::string& url, const std::string& outputFolder);
}