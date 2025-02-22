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
		uint8_t* memory;
		size_t size;

		MemoryStruct() {
			memory = reinterpret_cast<unsigned char*>(malloc(1)); /* will be grown as needed by the realloc in the callback */
			size = 0;
		}

		~MemoryStruct() {
			if (memory)
				free(memory);
		}
	};

	/**
	 * Callback function used for curl download
	**/
	static std::size_t WriteMemoryCallback(void* contents, std::size_t size, std::size_t nmemb, void* userp)
	{
		std::size_t realsize = size * nmemb;
		struct MemoryStruct* mem = static_cast<struct MemoryStruct*>(userp);

		uint8_t * newMem = static_cast<uint8_t*>(realloc(mem->memory, mem->size + realsize + 1));
		if (newMem == NULL) {
			/* out of memory! */
			throw std::runtime_error("Error: not enough memory (realloc returned NULL)\n");
		}
		
		mem->memory = newMem;
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
		curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/117.0.5938.132 Safari/537.36");
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
	 * @throws runtime_error on failure to download file or out of memory
	**/
	static void downloadFile(const std::string& url, const std::string& path)
	{
		CURL* curl;

		curl = curl_easy_init();
		curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1);
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10);

		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
		struct curlutils::MemoryStruct chunk;
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, static_cast<void*>(&chunk));

		CURLcode res = curl_easy_perform(curl);

		if (res != CURLE_OK)
			throw std::runtime_error("Image download failed: " + std::string(curl_easy_strerror(res)));

		curl_easy_cleanup(curl);

		// write data to file
		std::ofstream ofs(path, std::ofstream::out | std::ofstream::binary);
		ofs.write(reinterpret_cast<const char*>(chunk.memory), chunk.size);
		ofs.close();
	}
}