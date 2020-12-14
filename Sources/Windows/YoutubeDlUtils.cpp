//==============================================================================
/**
@file       YoutubeDlUtils.cpp
@brief      Utility functions for running youtube-dl.exe
@copyright  (c) 2020, Zongyi Yang
**/
//==============================================================================

#pragma once
#include "pch.h"

#include "YoutubeDlUtils.h"
#include "WindowsProcessUtils.h"
#include <filesystem>
#include <atlbase.h>

/**
 * Process optional output folder to return its string or just get the default output folder which is the desktop folder
 *
 * @param[in] optOutputFolder optional output folder. Defaults to desktop if not provided.
 * @return string containing the output folder
 * @throws runtime error if it cannot get desktop path
 */
std::string youtubedlutils::getOutputFolderName(const std::optional<std::string>& optOutputFolder)
{
	if (optOutputFolder)
		return *optOutputFolder;
	else
		return fileutils::getDesktopPath();
}

/**
 * Construct a youtube-dl command string that is passed as command line arguments to youtube-dl
 *
 * @param[in] url the url to download from
 * @param[in] optOutputFolder optional output folder. Defaults to desktop if not provided.
 * @param[in] optFilename optional filename. Defaults to "%(title)s.%(ext)s" if not provided.
 * @param[in] optMaxDownloads optional max downloads count. Defaults to 1 if not provided. Set to 0 for infinity.
 * @param[in] optType the type of download to perform
 * @return string containing the command
 */
std::string youtubedlutils::getDownloadCommand(const std::string& url,
	const std::optional<std::string>& optOutputFolder,
	const std::optional<std::string>& optFilename,
	const std::optional<uint32_t>& optMaxDownloads,
	const std::optional<uint32_t> optType)
{
	// default values
	std::string outputFolder = getOutputFolderName(optOutputFolder);
	std::string filename = "%(title)s.%(ext)s";
	uint32_t maxDownloads = 1;
	DL_TYPE type = VIDEO;

	// check for other optional inputs
	if (optFilename)
		filename = *optFilename;
	if (optMaxDownloads)
		maxDownloads = *optMaxDownloads;
	if (optType)
		type = static_cast<DL_TYPE>(*optType);

	//setup command
	std::string cmd;
	switch (type)
	{
	case VIDEO_ONLY:
		cmd = " -f bestvideo[ext!=webm]/mp4";
		break;
	case AUDIO_ONLY:
		cmd = " -f bestaudio/best -v --extract-audio --audio-quality 320k --audio-format mp3";
		break;
	default:
		cmd = " -f bestvideo[ext!=webm]+bestaudio[ext!=webm]/mp4";
		break;
	}
	if (maxDownloads != 0)
		cmd += " --max-downloads " + std::to_string(maxDownloads);
	cmd += " -o \"" + outputFolder + "/" + filename + "\"";
	cmd += " " + url;

	return cmd;
}

/**
 * Construct a queue of youtube-dl command strings that is passed as command line arguments to youtube-dl
 *
 * @param[in] url the url to download from
 * @param[in] optOutputFolder optional output folder. Defaults to desktop if not provided.
 * @param[in] optFilename optional filename. Defaults to "%(title)s.%(ext)s" if not provided.
 * @param[in] optMaxDownloads optional max downloads count. Defaults to 1 if not provided. Set to 0 for infinity.
 * @param[in] optType set of types of downloads to perform. A command will be created per type.
 * @param[in] optCustomCommand optional custom command.
 * @return vector containing all the commands
 */
std::vector <std::string> youtubedlutils::getCommandQueue(const std::string& url,
	const std::optional<std::string>& optOutputFolder,
	const std::optional<std::string>& optFilename,
	const std::optional<uint32_t>& optMaxDownloads,
	const std::unordered_set<DL_TYPE>& optType,
	const std::optional<std::string>& optCustomCommand)
{
	std::vector <std::string> cmds;
	for (const auto& format : optType)
		cmds.push_back(youtubedlutils::getDownloadCommand(url, optOutputFolder, optFilename, optMaxDownloads, format));

	if (optCustomCommand && !(*optCustomCommand).empty())
		cmds.push_back(" " + *optCustomCommand + " " + url);

	return cmds;
}


/**
 * Convert an optional youtube-dl exe path to actual string path.
 *
 * @return path to youtube-dl.exe
 */
std::string youtubedlutils::getYoutubeDlExePath(const std::optional<std::string>& optyoutubeDlExePath)
{
	const std::string defaultYoutubeDlExePath = "youtube-dl.exe";
	if (optyoutubeDlExePath)
		return *optyoutubeDlExePath;
	else
		return defaultYoutubeDlExePath;
}