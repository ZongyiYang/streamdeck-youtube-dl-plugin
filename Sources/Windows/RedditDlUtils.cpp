
#pragma once
#include "pch.h"
#include "RedditDlUtils.h"
#include <stdexcept>
#include <memory>
#include <filesystem>

#include "../Vendor/json/src/json.hpp"

/**
 * Gets the json from a url
 *
 * @throws runtime_error if could not read url for json, json::exception on bad json parse, invalid_argument if reddit content is not of image type
 */
void redditdlutils::getRedditImage(const std::string& url, const std::string& outputFolder, curlutils::MemoryStruct chunk)
{
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

		curlutils::downloadFile(path.string(), outputFolder + "/" + imgFileName, chunk);
	}
	else
		throw std::invalid_argument("Error: reddit webpage does not contain image data.");
}