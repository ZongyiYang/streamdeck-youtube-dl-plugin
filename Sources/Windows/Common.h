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

struct contextData_t
{
	std::optional<std::string> label = std::nullopt;
	std::optional<std::string> youtubeDlExePath = std::nullopt;
	std::optional<std::string> outputFolder = std::nullopt;
	std::optional <uint32_t> maxDownloads = std::nullopt;
	std::optional <uint32_t> type = std::nullopt;
	std::optional<std::string> customCommand = std::nullopt;
};