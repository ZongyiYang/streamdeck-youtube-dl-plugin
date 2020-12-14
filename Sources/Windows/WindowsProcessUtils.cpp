//==============================================================================
/**
@file       WindowsProcessUtils.cpp
@brief      Utility functions for windows processes
@copyright  (c) 2020, Zongyi Yang
**/
//==============================================================================
#pragma once
#include "pch.h"
#include "WindowsProcessUtils.h"

#include <atlbase.h> // for CA2T
#include <filesystem>

/**
 * Launch a exe with given commmand
 *
 * @param[in] exePath path to exe
 * @param[in] cmd the command line command passed to exe
 * @throws runtime_error if process could not launch,
 *         filesystem_error if filesystem exists fails,
 *         invalid_argument if exe path does not exist
 */
PROCESS_INFORMATION windowsprocessutils::startProcess(const std::string& exePath, const std::string& cmd)
{
	if (!std::filesystem::exists(exePath))
		throw std::invalid_argument("Cannot find exe at path: " + exePath);

	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	// Start the child process. 
	LPTSTR szAppName = CA2T(exePath.c_str());

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
		throw std::runtime_error("Cannot run exe.\nExe path: " + exePath + "\nCommand:\n" + cmd + "\n" + getLastErrorAsString());
	}

	return pi;
}

/**
 * Wait for process to complete
 *
 * @param[in] pi the process information struct
 * @throws runtime_error if failed to get exit code
 */
void windowsprocessutils::waitForProcess(PROCESS_INFORMATION pi)
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
 * @throws runtime_error if failed to get error code, or returns non-zero error code
 */
void windowsprocessutils::closeProcess(PROCESS_INFORMATION pi)
{
	DWORD exit_code;
	if (FALSE == GetExitCodeProcess(pi.hProcess, &exit_code))
		throw std::runtime_error("Cannot get exit code from process.");

	// Close process and thread handles. 
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

	if (exit_code != 0)
		throw std::runtime_error("Process returned non-zero error code: " + std::to_string(exit_code));
}

/**
* Gets the last windows error message as a string
*
* @returns error message as string
*/
std::string windowsprocessutils::getLastErrorAsString()
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