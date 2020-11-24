#pragma once
#include "pch.h"
#include "FileUtils.h"
#include <filesystem>
#include <assert.h>
#include <shlobj.h> //for SHGetKnownFolderPath
#include <winerror.h> //for HRESULT
#include <atlbase.h> // for CA2T

#include <regex>


// TODO: delete if unused
std::string fileutils::convertToValidNewFilePath(const std::string& folder, const std::string& filename, const std::string& extension)
{
	if (!std::filesystem::exists(folder))
		throw std::invalid_argument("Folder does not exist: " + folder);

	uint32_t pathLength = folder.length() + extension.length() + 1; // +1 for extra '/'
	if (pathLength >= MAX_PATH)
		throw std::invalid_argument("Path length plus extension too long: " + folder + "/" + " +" + extension + " length = " + std::to_string(pathLength) + " >= MAX_PATH " + std::to_string(MAX_PATH));

	std::string validFilename = filename;
	std::replace(validFilename.begin(), validFilename.end(), '"', '\'');

	// remove excess length to filename
	uint32_t maxFilenameLength = MAX_PATH - pathLength;
	validFilename = validFilename.substr(0, maxFilenameLength);

	// construct path to file
	std::string path = folder + "/" + validFilename + extension;

	// check if exists, append (#) to name if it does
	const uint32_t MAX_DUPLICATES = 10;
	for (uint32_t i = 0; i < MAX_DUPLICATES; i++)
	{
		if (!std::filesystem::exists(path))
			break;

		std::string duplicateNumbering = " (" + std::to_string(i) + ")";

		std::string numberedValidFilename = validFilename + duplicateNumbering;
		path = folder + "/" + numberedValidFilename + extension;

		if (path.length() > MAX_PATH)
			throw std::invalid_argument("Path length plus extension too long, cannot append duplicate numbering string: " + path + " is longer than MAX_PATH " + std::to_string(MAX_PATH));
	}

	if (std::filesystem::exists(path))
		throw std::invalid_argument("File path already exists: " + path + " \n Tried max " + std::to_string(MAX_DUPLICATES) + " duplicates.");

	assert(path.length() <= MAX_PATH);
	return path;
}

/**
 * Get path to desktop
 *
 * @throws std::runtime_error on failure to get path
 * @return path to desktop as string
 */
std::string fileutils::getDesktopPath()
{
	wchar_t* p;
	if (S_OK != SHGetKnownFolderPath(FOLDERID_Desktop, 0, NULL, &p))
		throw std::runtime_error("Cannot get desktop path.");
	std::filesystem::path result = p;
	CoTaskMemFree(p);
	return result.string();
}

/**
 * Open a folder with explorer
 *
 * @param[in] path the path to the folder
 */
void fileutils::openFolder(const std::string& path)
{
	// explorer seems to only work with backwards slashes, so need to replace all forward slashes
	std::string params = std::regex_replace(path, std::regex("\\/"), "\\");;
	ShellExecute(NULL, NULL, L"explorer.exe", CA2T(params.c_str()), NULL, SW_SHOWNORMAL);
}