//==============================================================================
/**
@file       ClipboardUtils.hpp
@brief      Utility functions for windows clipboard
@copyright  (c) 2020, Zongyi Yang
**/
//==============================================================================
#pragma once

#include <string>
#include <tchar.h>

namespace clipboardutils
{
	/**
	 * Get current clipboard text
	 *
	 * @throws std::runtime_error on failure to open clipboard, get clipboard handle, or locking clipboard
	 * @return clipboard text
	 */
	std::string getClipboardText()
	{
		// Try opening the clipboard
		if (!OpenClipboard(nullptr))
			throw std::runtime_error("Cannot open clipboard");

		// Get handle of clipboard object for ANSI text
		HANDLE hData = GetClipboardData(CF_TEXT);
		if (hData == nullptr)
		{
			CloseClipboard();
			throw std::runtime_error("Cannot get clipboard handle");
		}

		// Lock the handle to get the actual text pointer
		char* pszText = static_cast<char*>(GlobalLock(hData));
		if (pszText == nullptr)
		{
			CloseClipboard();
			throw std::runtime_error("Cannot lock clipboard");
		}

		// Save text in a string class instance
		std::string clipboardText(pszText);

		// Release the lock
		GlobalUnlock(hData);

		// Release the clipboard
		CloseClipboard();

		return clipboardText;
	}
}