//==============================================================================
/**
@file       VideoDownloadUtils.hpp
@brief      Utility functions for running youtube-dl.exe
@copyright  (c) 2020, Zongyi Yang
**/
//==============================================================================

#pragma once

#include "FileUtils.h"

#include <string>
#include <optional>


namespace videodownloadutils
{
	enum DL_TYPE;

	std::string getOutputFolderName(const std::optional<std::string>& optOutputFolder);

	std::string getDownloadCommand(const std::string& url,
		const std::optional<std::string>& optOutputFolder,
		const std::optional<std::string>& optFilename,
		const std::optional<uint32_t>& optMaxDownloads,
		const std::optional<uint32_t> optType);

	std::string getYoutubeDlExePath(const std::optional<std::string>& optyoutubeDlExePath);

	PROCESS_INFORMATION startDownload(const std::optional<std::string>& optyoutubeDlExePath, const std::string& cmd);
	PROCESS_INFORMATION startDownload(const std::string& url,
		const std::optional<std::string>& optOutputFolder,
		const std::optional<std::string>& optyoutubeDlExePath,
		const std::optional<std::string>& optFilename,
		const std::optional<uint32_t>& optMaxDownloads,
		const std::optional<uint32_t> type);

	void waitForProcess(PROCESS_INFORMATION pi);
	void closeProcess(PROCESS_INFORMATION pi);

	std::string getLastErrorAsString();
}