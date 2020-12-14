//==============================================================================
/**
@file       VideoDownloadUtils.hpp
@brief      Utility functions for running youtube-dl.exe
@copyright  (c) 2020, Zongyi Yang
**/
//==============================================================================

#pragma once

#include "FileUtils.h"
#include "Common.h"

#include <string>
#include <optional>
#include <unordered_set>


namespace videodownloadutils
{
	std::string getOutputFolderName(const std::optional<std::string>& optOutputFolder);

	std::string getDownloadCommand(const std::string& url,
		const std::optional<std::string>& optOutputFolder,
		const std::optional<std::string>& optFilename,
		const std::optional<uint32_t>& optMaxDownloads,
		const std::optional<uint32_t> optType);
	std::vector <std::string> getCommandQueue(const std::string& url,
		const std::optional<std::string>& optOutputFolder,
		const std::optional<std::string>& optFilename,
		const std::optional<uint32_t>& optMaxDownloads,
		const std::unordered_set<DL_TYPE>& optType,
		const std::optional<std::string>& optCustomCommand);

	std::string getYoutubeDlExePath(const std::optional<std::string>& optyoutubeDlExePath);

	PROCESS_INFORMATION startDownload(const std::optional<std::string>& optyoutubeDlExePath, const std::string& cmd);

	void waitForProcess(PROCESS_INFORMATION pi);
	void closeProcess(PROCESS_INFORMATION pi);

	std::string getLastErrorAsString();
}