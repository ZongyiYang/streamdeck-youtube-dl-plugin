//==============================================================================
/**
@file       CurlUtils.hpp
@brief      Utility functions for curl
@copyright  (c) 2020, Zongyi Yang
**/
//==============================================================================

#pragma once
#define CURL_STATICLIB
#include <curl\curl.h>

#include "UrlUtils.h"

#include <string>
#include <fstream>

namespace curlutils
{
	/**
	 * Callback function used for curl html read
	**/
	static std::size_t callback(
		const char* in,
		std::size_t size,
		std::size_t num,
		std::string* out)
	{
		const std::size_t totalBytes(size * num);
		out->append(in, totalBytes);
		return totalBytes;
	}

	/**
	 * Struct for curl download
	**/
	struct MemoryStruct {
		unsigned char* memory;
		size_t size;
	};

	/**
	 * Callback function used for curl download
	**/
	static std::size_t WriteMemoryCallback(void* contents, std::size_t size, std::size_t nmemb, void* userp)
	{
		std::size_t realsize = size * nmemb;
		struct MemoryStruct* mem = (struct MemoryStruct*)userp;

		mem->memory = (unsigned char*)realloc(mem->memory, mem->size + realsize + 1);
		if (mem->memory == NULL) {
			/* out of memory! */
			throw std::runtime_error("Error: not enough memory (realloc returned NULL)\n");
			return 0;
		}

		memcpy(&(mem->memory[mem->size]), contents, realsize);
		mem->size += realsize;
		mem->memory[mem->size] = 0;

		return realsize;
	}


	/**
	 * Download url as string
	 *
	 * @param[in] url the url to download from
	 * @param[out] data the html string from the url
	 * @return true if success
	**/
	static bool readHTML(const std::string& url, std::string* data)
	{
		if (!urlutils::isValidUrl(url))
			return false;

		CURL* curl;
		data->clear();

		curl = curl_easy_init();
		curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1);
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10);

		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, callback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, data);

		curl_easy_perform(curl);

		long httpCode;
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);

		curl_easy_cleanup(curl);

		if (httpCode == 200)
			return true;
		return false;
	}

	/**
	 * Download url as to file
	 *
	 * @param[in] url the url to download from
	 * @param[in] path the output path
	 * @param[in] chunk the MemoryStruct used by the callback. Must be allocated and deallocated outside this function
	 * @throws runtime_error on failure to download file
	**/
	static void downloadFile(const std::string& url, const std::string& path, MemoryStruct chunk)
	{
		CURL* curl;

		curl = curl_easy_init();
		curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1);
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10);

		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)& chunk);

		CURLcode res = curl_easy_perform(curl);

		if (res != CURLE_OK)
			throw std::runtime_error("Image download failed: " + std::string(curl_easy_strerror(res)));

		curl_easy_cleanup(curl);

		// write data to file
		std::ofstream ofs(path, std::ofstream::out | std::ofstream::binary);
		ofs.write((char*)& chunk.memory[0], chunk.size);
		ofs.close();
	}
}