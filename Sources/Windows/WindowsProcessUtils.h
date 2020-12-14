//==============================================================================
/**
@file       WindowsProcessUtils.h
@brief      Utility functions for windows processes
@copyright  (c) 2020, Zongyi Yang
**/
//==============================================================================

#pragma once

#include <tchar.h>

#include <string>


namespace windowsprocessutils
{
	PROCESS_INFORMATION startProcess(const std::string& exePath, const std::string& cmd);
	void waitForProcess(PROCESS_INFORMATION pi);
	void closeProcess(PROCESS_INFORMATION pi);

	std::string getLastErrorAsString();
}