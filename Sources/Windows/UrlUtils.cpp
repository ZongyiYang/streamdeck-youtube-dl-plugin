//==============================================================================
/**
@file       UrlUtils.cpp
@brief      Utility functions for manipulating urls
@copyright  (c) 2021, Zongyi Yang
**/
//==============================================================================

#pragma once
#include "pch.h"

#include "UrlUtils.h"

bool urlutils::isValidUrl(const std::string& url)
{
	return (IsValidURL(NULL, CA2T(url.c_str()), 0) == S_OK);
}