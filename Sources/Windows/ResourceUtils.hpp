//==============================================================================
/**
@file       ResourceUtils.hpp
@brief      Utility functions for extracting resources embedded in the exe
@copyright  (c) 2020, Zongyi Yang
**/
//==============================================================================

#pragma once
#include <string>
#include <tchar.h>
#include <atlbase.h>
#include <filesystem>
#include <fstream>
#include "Windows/resource.h"
#include "Windows/FileUtils.h"

namespace resourceutils
{
	/**
	 * Extract resource embedded in exe and write it to a file
	 *
	 * @param[in] id the resource ID
	 * @param[in] type the resource type
	 * @param[in] path the path to write the resource to
	 * @throws std::runtime_error on failure to find resource, load resource, lock resource, or open file for writing
	 */
	void extractResource(const int32_t id, const std::string& type, const std::string& path)
	{
		// check if already unpacked
		if (std::filesystem::exists(path))
			return;

		HGLOBAL hResLoad;          // handle to loaded resource
		HRSRC hRes;                // handle/ptr. to res. info.
		void* ptr;                 // pointer to resource data
		std::size_t sizeBytes = 0; // size of resource

		auto throwError = [&](const std::string & errorMsg)
		{
			throw std::runtime_error(errorMsg + "\nid: " + std::to_string(id) + " type: " + type + " path: " + path);
		};

		hRes = FindResource(nullptr, MAKEINTRESOURCE(id), CA2T(type.c_str()));
		if (hRes == NULL)
			throwError("Could not find resource:");

		hResLoad = LoadResource(nullptr, hRes);
		if (hResLoad == NULL)
			throwError("Could not load resource:");

		ptr = LockResource(hResLoad);
		if (ptr == NULL)
			throwError("Could not lock resource:");

		sizeBytes = SizeofResource(nullptr, hRes);

		// write memory to file
		std::ofstream fout;
		fout.open(path, std::ios::binary | std::ios::out);
		if (fout.is_open())
		{
			fout.write(static_cast<const char*>(ptr), sizeBytes);
			fout.close();
		}
		else
		{
			throwError("Could not write resource to file:");
		}
	}
}