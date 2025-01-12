//==============================================================================
/**
@file       FileUtils.h
@brief      Utility functions for windows filesystem
@copyright  (c) 2020, Zongyi Yang
**/
//==============================================================================
#pragma once
#include <string>
#include <filesystem>

namespace fileutils
{
	std::string convertToValidNewFilePath(const std::string& folder, const std::string& filename, const std::string& extension);
	std::string getDesktopPath();
	std::filesystem::path getFolder(const std::filesystem::path& path);
	void openFolder(const std::filesystem::path& path);
	std::filesystem::path getCurrentExeFolder();
}