//==============================================================================
/**
@file       Common.h

@brief		Common structs

@copyright  (c) 2020, Zongyi Yang.

**/
//==============================================================================

#pragma once

#include "pch.h"
#include <string>
#include <optional>
#include <unordered_set>

// the type of download to perform
enum DL_TYPE
{
	VIDEO,
	AUDIO_ONLY,
	VIDEO_ONLY
};

struct contextData_t
{
	std::optional<std::string> label = std::nullopt;
	std::optional<std::string> youtubeDlExePath = std::nullopt;
	std::optional<std::string> outputFolder = std::nullopt;
	std::optional <uint32_t> maxDownloads = std::nullopt;
	std::unordered_set <DL_TYPE> downloadFormats = {};
	std::optional<std::string> customCommand = std::nullopt;
	bool attemptImageDl = false;
};