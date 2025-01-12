//==============================================================================
/**
@file       DownloadThread.cpp

@brief		Runs a youtube-dl download process

@copyright  (c) 2020, Zongyi Yang.

**/
//==============================================================================

#include "pch.h"

#include "DownloadThread.h"
#include "YoutubeDlUtils.h"
#include "RedditDlUtils.h"
#include "CurlUtils.hpp"
#include "WindowsProcessUtils.h"

/**
 * Launch a new youtube-dl process.
 *
 * @param[in] url the url to download from
 * @param[in] data the metadata stored by the context
 * @param[in] doUpdate update youtube-dl
 * @param[in] cvMutex the mutex to lock for the cv
 * @param[in] cv the condition variable to wake on completion
 * @param[in] results the queue to place finished results data
 */
void DownloadThread::launchDownloadProcess(const std::string& url, const contextSettings_t& data, const bool doUpdate,
										   std::mutex& cvMutex, std::condition_variable& cv,
										   std::queue<threadData_t>& results)
{
	bool exited = false;
	// helper function to update mData, push to results, and exit download process
	auto exitDownloadProcess = [&](const std::optional<std::string>& logMsg,
		const std::optional<std::string>& errMsg,
		const status_t newState)
	{
		// We cannot call this twice otherwise it will cause a deadlock.
		// This is because we will wake cvLk condition variable, which on first call will cause main
		// thread to clean up and join this thread while holding cvLk.
		// But if we call this a second time, this thread will try to hold cvLk again,
		// and main thread will wait for join while also holding cvLk, which causes a deadlock.
		assert(exited == false);
		exited = true;

		std::unique_lock<std::mutex>lk(mDataMutex);

		// update mData;
		if (logMsg)
			mData.log = *logMsg;
		if (errMsg)
			mData.buttonMsg = *errMsg;
		mData.status = newState;

		mState = newState;

		// wake cv if not detached
		{
			std::unique_lock<std::mutex>cmdLk(mCommandMutex);
			flags_t flag = mCommand.load();
			if (flag != DETACH)
			{
				std::unique_lock<std::mutex>cvLk(cvMutex);
				results.push(mData);
				cv.notify_all();
			}
		}

		// if this was detached, mPtr was assigned to self to keep object alive.
		// since we are done now, release shared_ptr to self, causing destructor
		mPtr = nullptr;
	};

	// construct command strings
	std::vector<std::string> cmds;
	try
	{
		if (doUpdate)
			cmds.push_back(" --update");
		else
		{
			cmds = youtubedlutils::getCommandQueue(url, data.outputFolder, std::nullopt, data.maxDownloads, data.downloadFormats, data.customCommand);
		}
	}
	catch (std::runtime_error& e)
	{
		exitDownloadProcess("Download failed building commands:\n" + std::string(e.what()),
			(doUpdate ? std::string("Update") : std::string("Download")) + "\nfailed", FAILED);
		return;
	}

	// check if output folder exists
	// std::filesystem can throw an error, so catch that too
	if (!doUpdate)
	{
		bool doesOutputFolderExist = true;
		try
		{
			if (data.outputFolder && !std::filesystem::exists(*data.outputFolder))
				doesOutputFolderExist = false;
		}
		catch (std::filesystem::filesystem_error& e)
		{
			exitDownloadProcess("Output folder filesystem error: " + *data.outputFolder + "\n" + std::string(e.what()),
				"Invalid\noutput folder", FAILED);
			return;
		}
		if (!doesOutputFolderExist)
		{
			exitDownloadProcess("Invalid output folder: " + *data.outputFolder,
				"Missing\noutput folder", FAILED);
			return;
		}
	}

	// try to download reddit link
	if (!doUpdate && data.attemptRedditDl)
	{
		bool success = false;
		std::string errMsg;
		try
		{
			mState = RUNNING;
			redditdlutils::downloadRedditContent(url, youtubedlutils::getOutputFolderName(data.outputFolder));
			success = true;
		}
		catch (std::exception& e)
		{
			errMsg = "Image download failed:\n" + std::string(e.what());
		}

		// if image dl was successful, no need to call youtube-dl, just exit as success
		if (success == true)
		{
			mState = STOPPING;
			exitDownloadProcess(std::nullopt, std::nullopt, SUCCESS);
			return;
		}
		else if (cmds.size() == 0)
		{
			// if we are only trying to download images and are not calling youtube-dl, but we failed,
			// then return error messages
			mState = STOPPING;
			exitDownloadProcess(errMsg,
				"Image download\nfailed", FAILED);
			return;
		}
	}

	// execute each command sequentially
	for (const auto cmd : cmds)
	{
		// start the download process
		try
		{
			{
				std::unique_lock<std::mutex> lk{ mCommandMutex };
				if (mCommand.load() != KILL)
				{
					const std::filesystem::path youtubeDlExePath = youtubedlutils::getDownloaderExePath(data.youtubeDlExePath);
					mPi = windowsprocessutils::startProcess(youtubeDlExePath, cmd);
					mState = RUNNING;
				}
			}

			windowsprocessutils::waitForProcess(mPi);

			{
				std::unique_lock<std::mutex> lk{ mCommandMutex };
				mState = STOPPING;
				windowsprocessutils::closeProcess(mPi);
			}
		}
		catch (std::filesystem::filesystem_error& e)
		{
			exitDownloadProcess("yt-dlp failed:\n" + std::string(e.what()),
				"Invalid path to\nyt-dlp.exe", FAILED);
			return;
		}
		catch (std::invalid_argument& e)
		{
			exitDownloadProcess("yt-dlp failed:\n" + std::string(e.what()),
				"Missing\nyt-dlp.exe", FAILED);
			return;
		}
		catch (std::exception& e)
		{
			exitDownloadProcess("yt-dlp failed:\n" + std::string(e.what()),
				(doUpdate ? std::string("Update") : std::string("Download")) + "\nfailed", FAILED);
			return;
		}

		// don't process any further commands if we are given a KILL command
		if (mCommand.load() == KILL)
			break;
	}

	if (doUpdate)
		if (mCommand.load() != KILL)
			exitDownloadProcess("yt-dlp updated.", "Update\nfinished", UPDATED);
		else
			exitDownloadProcess("Warning! yt-dlp update was interrupted.", "Update\ninterrupted", FAILED);
	else
		exitDownloadProcess(std::nullopt, std::nullopt, SUCCESS);
}