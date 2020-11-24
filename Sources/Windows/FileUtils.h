//==============================================================================
/**
@file       FileUtils.h
@brief      Utility functions for windows filesystem
@copyright  (c) 2020, Zongyi Yang
**/
//==============================================================================
#pragma once
#include <string>

namespace fileutils
{
	std::string convertToValidNewFilePath(const std::string& folder, const std::string& filename, const std::string& extension);
	std::string getDesktopPath();
	void openFolder(const std::string& path);
}