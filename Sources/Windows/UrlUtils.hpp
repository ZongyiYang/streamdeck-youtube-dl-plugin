//==============================================================================
/**
@file       UrlUtils.hpp
@brief      Utility functions for manipulating urls
@copyright  (c) 2020, Zongyi Yang
**/
//==============================================================================

#pragma once

#include <string>
#include <urlmon.h> // for IsValidURL
#pragma comment(lib, "urlmon.lib")

#include <atlbase.h> // for CA2T

namespace urlutils
{
	/**
	 * Check if a url is valid
	 *
	 * @param[in] url the url to check
	 * @return true if valid, false if not
	 */
	bool isValidUrl(const std::string& url)
	{
		return (IsValidURL(NULL, CA2T(url.c_str()), 0) == S_OK);
	}
}