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

std::string youtubedlutils::getLastErrorAsString()
{
	//Get the error message, if any.
	DWORD errorMessageID = ::GetLastError();
	if (errorMessageID == 0)
		return std::string(); //No error message has been recorded

	LPSTR messageBuffer = nullptr;
	size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)& messageBuffer, 0, NULL);

	std::string message(messageBuffer, size);

	//Free the buffer.
	LocalFree(messageBuffer);

	return message;
}

/**
 * Launch a youtube-dl process with given command
 *
 * @param[in] optyoutubeDlExePath optional path to youtube-dl.exe. Default is youtube-dl.exe, found in the same folder as this program.
 * @param[in] cmd the command line command passed to youtube-dl.exe
 * @throws invalid_argument if youtube-dl.exe missing from path, runtime_error if process could not launch, could not retrive exit code, or download failed.
 *         filesystem_error if filesystem exists fails
 */
PROCESS_INFORMATION youtubedlutils::startDownload(const std::optional<std::string>& optyoutubeDlExePath, const std::string& cmd)
{
	std::string youtubeDlExePath = getYoutubeDlExePath(optyoutubeDlExePath);
	if (!std::filesystem::exists(youtubeDlExePath))
		throw std::invalid_argument("Cannot find youtube dl at path: " + youtubeDlExePath);

	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	// Start the child process. 
	LPTSTR szAppName = CA2T(youtubeDlExePath.c_str());

	if (!CreateProcess(szAppName,
		CA2T(cmd.c_str()),        // Command line
		NULL,           // Process handle not inheritable
		NULL,           // Thread handle not inheritable
		FALSE,          // Set handle inheritance to FALSE
		0,              // No creation flags
		NULL,           // Use parent's environment block
		NULL,           // Use parent's starting directory 
		&si,            // Pointer to STARTUPINFO structure
		&pi)           // Pointer to PROCESS_INFORMATION structure
		)
	{
		throw std::runtime_error("Cannot run youtube-dl.\nExe: " + youtubeDlExePath + "\nCommand:\n" + cmd + "\n" + getLastErrorAsString());
	}

	return pi;
}

/**
 * Wait for process to complete
 *
 * @param[in] pi the process information struct
 */
void youtubedlutils::waitForProcess(PROCESS_INFORMATION pi)
{
	// Wait until child process exits.
	// Note: this is only safe for processes that do not create windows. Otherwise MsgWaitForMultipleObjects may be needed.
	//WaitForSingleObject(pi.hProcess, INFINITE);

	DWORD exit_code = STILL_ACTIVE;
	while (exit_code == STILL_ACTIVE)
	{
		if (FALSE == GetExitCodeProcess(pi.hProcess, &exit_code))
			throw std::runtime_error("Cannot get exit code.");
		Sleep(1000);
	}
}

/**
 * Closes a process
 *
 * @param[in] pi the process information struct
 */
void youtubedlutils::closeProcess(PROCESS_INFORMATION pi)
{
	DWORD exit_code;
	if (FALSE == GetExitCodeProcess(pi.hProcess, &exit_code))
		throw std::runtime_error("Cannot get exit code.");

	// Close process and thread handles. 
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

	if (exit_code != 0)
		throw std::runtime_error("Youtube-dl returned non-zero error code: " + std::to_string(exit_code));
}