//==============================================================================
/**
@file       RedditDlUtils.h
@brief      Utility functions for downloading from reddit
@copyright  (c) 2021, Zongyi Yang
**/
//==============================================================================

#pragma once
#include "pch.h"
#include "RedditDlUtils.h"
#include <stdexcept>
#include <memory>
#include <filesystem>

#include "../Vendor/json/src/json.hpp"


void redditdlutils::getRedditImage(const std::string& url, const std::string& outputFolder, curlutils::MemoryStruct chunk)
{
	// first get the json metadata from the reddit page using curl
	std::unique_ptr<std::string> htmlData;
	htmlData.reset(new std::string());
	bool success = curlutils::readHTML(url + ".json", htmlData.get());
	if (!success)
		throw std::runtime_error("Error: could not curl url for json data: " + url + ".json");
	nlohmann::json j = nlohmann::json::parse(*htmlData);
	
	// get metadata if it's an image
	nlohmann::json postHint = j[0]["data"]["children"][0]["data"]["post_hint"];
	if (postHint == "image")
	{
		nlohmann::json title = j[0]["data"]["children"][0]["data"]["title"];
		nlohmann::json imgUrl = j[0]["data"]["children"][0]["data"]["url"];

		std::filesystem::path path = imgUrl.get<std::string>();
		std::string imgFileName = title.get<std::string>() + path.extension().string();

		// download the file
		curlutils::downloadFile(path.string(), outputFolder + "/" + imgFileName, chunk);
	}
	else
		throw std::invalid_argument("Error: reddit webpage does not contain image data.");
}