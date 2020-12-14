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
void DownloadThread::launchDownloadProcess(const std::string url, const contextData_t data, const bool doUpdate,
										   std::mutex& cvMutex, std::condition_variable& cv,
										   std::queue<threadData_t>& results)
{
	// helper function to update mData, push to results, and exit download process
	auto exitDownloadProcess = [&](const std::optional<std::string>& logMsg,
		const std::optional<std::string>& errMsg,
		const status_t newState)
	{
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
		exitDownloadProcess("Youtube-dl failed:\n" + std::string(e.what()),
			(doUpdate ? std::string("Update") : std::string("Download")) + "\nfailed", FAILED);
		return;
	}

	// execute each command sequentially
	for (const auto cmd : cmds)
	{
		// check if output folder exists (we check each time in case this changes between downloads)
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

		// start the download process
		try
		{
			{
				std::unique_lock<std::mutex> lk{ mCommandMutex };
				if (mCommand.load() != KILL)
				{
					std::string youtubeDlExePath = youtubedlutils::getYoutubeDlExePath(data.youtubeDlExePath);
					pi = windowsprocessutils::startProcess(youtubeDlExePath, cmd);
					mState = RUNNING;
				}
			}

			windowsprocessutils::waitForProcess(pi);

			{
				std::unique_lock<std::mutex> lk{ mCommandMutex };
				mState = STOPPING;
				windowsprocessutils::closeProcess(pi);
			}
		}
		catch (std::filesystem::filesystem_error& e)
		{
			exitDownloadProcess("Youtube-dl failed:\n" + std::string(e.what()),
				"Invalid path to\nyoutube-dl.exe", FAILED);
			return;
		}
		catch (std::invalid_argument& e)
		{
			exitDownloadProcess("Youtube-dl failed:\n" + std::string(e.what()),
				"Missing\nyoutube-dl.exe", FAILED);
			return;
		}
		catch (std::exception& e)
		{
			exitDownloadProcess("Youtube-dl failed:\n" + std::string(e.what()),
				(doUpdate ? std::string("Update") : std::string("Download")) + "\nfailed", FAILED);
			return;
		}

		// don't process any further commands if we are given a KILL command
		if (mCommand.load() == KILL)
			break;
	}

	if (doUpdate)
		if (mCommand.load() != KILL)
			exitDownloadProcess("Youtube-dl updated.", "Update\nfinished", UPDATED);
		else
			exitDownloadProcess("Warning! Youtube-dl update was interrupted.", "Update\ninterrupted", FAILED);
	else
		exitDownloadProcess(std::nullopt, std::nullopt, SUCCESS);
}