//==============================================================================
/**
@file       MyStreamDeckPlugin.cpp
@brief      Plugin for calling youtube-dl
@copyright  (c) 2020, Zongyi Yang
**/
//==============================================================================

#include "MyStreamDeckPlugin.h"
#include "Common/ESDConnectionManager.h"

#include "Windows/ResourceUtils.hpp"
#include "Windows/ClipboardUtils.hpp"
#include "Windows/UrlUtils.hpp"
#include "Windows/VideoDownloadUtils.h"



MyStreamDeckPlugin::MyStreamDeckPlugin()
{
	mIsRunning = initYoutubeDl();
	mDlMonitor = std::thread(&MyStreamDeckPlugin::downloadMonitor, this);
}

MyStreamDeckPlugin::~MyStreamDeckPlugin()
{
	// send stop signal to UI thread
	mIsRunning = false;
	{
		std::unique_lock<std::mutex>cvLk(cvMutex);
		cv.notify_all();
	}

	if (mDlMonitor.joinable())
	{
		mDlMonitor.join();
	}

	std::unique_lock<std::mutex>lk(mVisibleContextsMutex);
	for (const auto& dls : mActiveDownloads)
	{
		for (const auto& thd : dls.second.threads)
		{
			thd->detach();
		}
	}
}

/**
 * Unpack youtube-dl resources from this exe.
 *
 * @return true on success, false on failure
 */
bool MyStreamDeckPlugin::initYoutubeDl()
{
	try
	{
		resourceutils::extractResource(IDR_EXE1, "EXE", "youtube-dl.exe");
		resourceutils::extractResource(IDR_EXE2, "EXE", "ffmpeg.exe");
	}
	catch (std::runtime_error& e)
	{
		std::cerr << e.what() << std::endl;
		return false;
	}

	return true;
}

/**
 * Helper function for downloadMonitor to clean up download threads if all threads are completed.
 *
 * @param[in] context the context to clean up
 * @param[in] lk the lock for mutex mVisibleContextsMutex
 * @relatesalso downloadMonitor
 */
void MyStreamDeckPlugin::cleanupDownloads(const std::string& context, const std::unique_lock<std::mutex> & lk)
{
	assert(lk.owns_lock());
	assert(lk.mutex() == &mVisibleContextsMutex);

	if (mActiveDownloads.find(context) != mActiveDownloads.end())
	{
		// cleanup if all threads are done
		bool allDone = true;
		for (const auto& thd : mActiveDownloads.at(context).threads)
		{
			if (!thd->isComplete())
			{
				allDone = false;
				break;
			}
		}
		if (allDone)
			mActiveDownloads.erase(context);
	}
}

/**
 * Helper function for downloadMonitor. Reads results queue and updates the data for contexts that are modified
 *
 * @param[in] lk the lock for mutex mVisibleContextsMutex
 * @return set of contexts that had something changed
 * @relatesalso downloadMonitor
 */
std::unordered_set <std::string> MyStreamDeckPlugin::getModifiedContexts(const std::unique_lock<std::mutex>& lk)
{
	assert(lk.owns_lock());
	assert(lk.mutex() == &mVisibleContextsMutex);

	std::unordered_set <std::string> modifiedContexts;
	while (!results.empty())
	{
		DownloadThread::threadData_t threadData = results.front();
		results.pop();
		modifiedContexts.insert(threadData.context);

		switch (threadData.status)
		{
		case DownloadThread::UPDATED:
			mIsUpdating = false;
			mActiveDownloads.at(threadData.context).successCount++;
			if (mConnectionManager != nullptr)
				mConnectionManager->LogMessage("Youtube-dl updated.");
			break;
		case DownloadThread::SUCCESS:
			mActiveDownloads.at(threadData.context).successCount++;
			break;
		case DownloadThread::FAILED:
			mActiveDownloads.at(threadData.context).failureCount++;
			if (mConnectionManager != nullptr)
			{
				mConnectionManager->LogMessage("Failed youtube-dl at context: " + threadData.context);
				if (threadData.log)
					mConnectionManager->LogMessage("Log: " + *threadData.log);
			}
			break;
		}

		// update error message for this context if there are any
		if (threadData.buttonMsg && mVisibleContexts.find(threadData.context) != mVisibleContexts.end())
			mVisibleContexts.at(threadData.context).lastErrorMsg = threadData.buttonMsg;
	}

	return modifiedContexts;
}

/**
 * Thead function that waits for download completion signals, processes results, and updates UI
 */
void MyStreamDeckPlugin::downloadMonitor()
{
	while (mIsRunning.load())
	{
		// wait for wake signal from threads
		std::unique_lock<std::mutex>cvLk(cvMutex);
		cv.wait(cvLk, [&] {return !results.empty(); });

		std::unique_lock<std::mutex>lk(mVisibleContextsMutex);
		// set of contexts that changed
		std::unordered_set <std::string> modifiedContexts = getModifiedContexts(lk);

		for (const auto& context : modifiedContexts)
		{
			updateUI(context, lk);
			cleanupDownloads(context, lk);
		}
	}

	if (mConnectionManager != nullptr)
	{
		mConnectionManager->LogMessage("Shutting down mDlMonitor");
	}
}

/**
 * Updates the title text of a button
 *
 * @param[in] context the button's context
 * @param[in] lk the lock for mutex mVisibleContextsMutex
 */
void MyStreamDeckPlugin::updateUI(const std::string& inContext, const std::unique_lock<std::mutex>& lk)
{
	assert(lk.owns_lock());
	assert(lk.mutex() == &mVisibleContextsMutex);

	if (mConnectionManager != nullptr && mVisibleContexts.find(inContext) != mVisibleContexts.end())
	{
		std::string label = "";
		std::string errMsg = "\n";
		if (mVisibleContexts.at(inContext).data.label)
			label = *mVisibleContexts.at(inContext).data.label;
		if (mVisibleContexts.at(inContext).lastErrorMsg)
			errMsg = *mVisibleContexts.at(inContext).lastErrorMsg;

		uint32_t totalThreads = 0;
		uint32_t successfulThreads = 0;
		uint32_t failedThreads = 0;
		uint32_t pendingThreads = 0;
		if (mActiveDownloads.find(inContext) != mActiveDownloads.end())
		{
			totalThreads = mActiveDownloads.at(inContext).threads.size();
			successfulThreads = mActiveDownloads.at(inContext).successCount;
			failedThreads = mActiveDownloads.at(inContext).failureCount;
			pendingThreads = totalThreads - successfulThreads - failedThreads;
		}
		mConnectionManager->SetTitle(label + "\nPending: " + std::to_string(pendingThreads) + "\n" + errMsg, inContext, kESDSDKTarget_HardwareAndSoftware);
	}
}

/**
 * Creates a new download task thread
 *
 * @param[in] url the url to download from
 * @param[in] data the metadata stored by the context
 * @param[in] inContext the button's context
 * @param[in] doUpdate update youtube-dl
 * @param[in] lk the lock for mutex mVisibleContextsMutex
 */
void MyStreamDeckPlugin::submitDownloadTask(const std::string & url, const contextData_t & data,
	const std::string& inContext, const bool doUpdate, const std::unique_lock<std::mutex>& lk)
{
	assert(lk.owns_lock());
	assert(lk.mutex() == &mVisibleContextsMutex);

	if (mActiveDownloads.find(inContext) == mActiveDownloads.end())
		mActiveDownloads.insert({ inContext, {} });
	std::shared_ptr<DownloadThread> dl = std::make_shared<DownloadThread>();
	dl->start(url, data, inContext, doUpdate, cvMutex, cv, results);
	mActiveDownloads.at(inContext).threads.push_back(std::move(dl));
}

void MyStreamDeckPlugin::KeyDownForAction(const std::string& inAction, const std::string& inContext, const json &inPayload, const std::string& inDeviceID)
{
	std::unique_lock<std::mutex>lk(mVisibleContextsMutex);
	if (mConnectionManager != nullptr && mVisibleContexts.find(inContext) != mVisibleContexts.end())
	{
		mVisibleContexts.at(inContext).buttonTimer->stop();

		// get output folder name
		std::string folder = videodownloadutils::getOutputFolderName(mVisibleContexts.at(inContext).data.outputFolder);

		// start timer that opens this folder once time is reached
		const uint32_t LONG_PRESS_TIME_MILLIS = 500;
		mVisibleContexts.at(inContext).buttonTimer->start(LONG_PRESS_TIME_MILLIS, [folder]()
			{
				if (std::filesystem::exists(folder))
				{
					fileutils::openFolder(folder);
					return true;
				}
				else
					return false;
			});
	}
}


void MyStreamDeckPlugin::KeyUpForAction(const std::string& inAction, const std::string& inContext, const json &inPayload, const std::string& inDeviceID)
{
	std::unique_lock<std::mutex>lk(mVisibleContextsMutex);

	if (mConnectionManager != nullptr && mVisibleContexts.find(inContext) != mVisibleContexts.end())
	{
		if (!mIsRunning.load())
			return;

		const contextData_t& data = mVisibleContexts.at(inContext).data;
		std::optional<std::string>& lastErrorMsg = mVisibleContexts.at(inContext).lastErrorMsg;

		// check if button timer completed
		if (mVisibleContexts.at(inContext).buttonTimer != nullptr)
		{
			mVisibleContexts.at(inContext).buttonTimer->stop();

			// If button press timer completed, it means the press was longer than LONG_PRESS_TIME_MILLIS
			// This indicates that a long press was done and folder was opened. In this case don't execute anything, just return
			if (mVisibleContexts.at(inContext).buttonTimer->isCompleted())
				if (mVisibleContexts.at(inContext).buttonTimer->isSuccessful())
					return;
				else
				{
					mConnectionManager->LogMessage("Error: cannot open folder: " + videodownloadutils::getOutputFolderName(data.outputFolder));
					lastErrorMsg = "Error: cannot\nopen folder";
					updateUI(inContext, lk);
					return;
				}
		}

		// Cannot launch a new download if we are updating
		if (mIsUpdating.load())
		{
			mConnectionManager->LogMessage("Error: cannot start download, update in progress.");
			lastErrorMsg = "Error: update\nin progress";
			updateUI(inContext, lk);
			return;
		}

		// get clipboard text
		std::string clipboardText;
		try
		{
			clipboardText = clipboardutils::getClipboardText();
		}
		catch (std::runtime_error& e)
		{
			mConnectionManager->LogMessage("Invalid clipboard:");
			mConnectionManager->LogMessage(e.what());
			lastErrorMsg = "Invalid\nclipboard";
			updateUI(inContext, lk);
			return;
		}

		// check if clipboard text is a URL
		if (!urlutils::isValidUrl(clipboardText.c_str()))
		{
			mConnectionManager->LogMessage("Invalid URL: " + clipboardText);
			lastErrorMsg = "Invalid\nURL";
			updateUI(inContext, lk);
			return;
		}

		// load settings
		contextData_t settings{};
		if (inPayload.find("settings") != inPayload.end())
			readPayload(settings, inPayload["settings"], lk);
		else
		{
			mConnectionManager->LogMessage("KeyUpForAction Error: No Settings");
			lastErrorMsg = "Failed to\nreceive settings";
			updateUI(inContext, lk);
			return;
		}
		// spawn a new download task
		lastErrorMsg = std::nullopt; // clear error
		submitDownloadTask(clipboardText, settings, inContext, false, lk);
		updateUI(inContext, lk);
	}
}

/**
 * Reads data from a json payload from PI and stores into data struct
 *
 * @param[out] data the data to update
 * @param[in] inPayload the json payload to read
 * @param[in] lk the lock for mutex mVisibleContextsMutex
 */
void MyStreamDeckPlugin::readPayload(contextData_t& data, const json& inPayload, const std::unique_lock<std::mutex>& lk)
{
	assert(lk.owns_lock());
	assert(lk.mutex() == &mVisibleContextsMutex);

	// helper to convert string to std::nullopt if it's empty
	auto convertToNullIfEmpty = [](json data)
	{
		return (data == "") ? std::optional<std::string>(std::nullopt) : data.get<std::string>();
	};

	// helper to convert optional string to optional uint32_t
	auto convertToUint32Option = [](std::optional<std::string> data)
	{
		return (data) ? std::stoi(*data) : std::optional<uint32_t>(std::nullopt);
	};

	try
	{
		if (inPayload.find("label") != inPayload.end())
			data.label = inPayload["label"].get<std::string>();
		if (inPayload.find("youtubeDlExePath") != inPayload.end())
			data.youtubeDlExePath = convertToNullIfEmpty(inPayload["youtubeDlExePath"]);
		if (inPayload.find("outputFolder") != inPayload.end())
			data.outputFolder = convertToNullIfEmpty(inPayload["outputFolder"]);
		if (inPayload.find("maxDownloads") != inPayload.end())
			data.maxDownloads = convertToUint32Option(convertToNullIfEmpty(inPayload["maxDownloads"]));
		if (inPayload.find("videoDl") != inPayload.end())
		{
			data.downloadFormats.erase(VIDEO);
			data.downloadFormats.erase(VIDEO_ONLY);
			if (inPayload["videoDl"].get<std::string>() == "on")
				data.downloadFormats.insert(VIDEO);
			else if (inPayload["videoDl"].get<std::string>() == "on_muted")
				data.downloadFormats.insert(VIDEO_ONLY);
		}
		if (inPayload.find("audioDl") != inPayload.end())
		{
			if (inPayload["audioDl"].get<std::string>() == "on")
				data.downloadFormats.insert(AUDIO_ONLY);
			else
				data.downloadFormats.erase(AUDIO_ONLY);
		}
		if (inPayload.find("imageDl") != inPayload.end())
			if (inPayload["imageDl"].get<std::string>() == "on")
				data.attemptImageDl = true;
		if (inPayload.find("customCommand") != inPayload.end())
			data.customCommand = convertToNullIfEmpty(inPayload["customCommand"]);
	}
	catch (std::exception& e)
	{
		if (mConnectionManager != nullptr)
		{
			mConnectionManager->LogMessage("readPayload failed:");
			mConnectionManager->LogMessage(inPayload.dump(4));
			mConnectionManager->LogMessage(e.what());
		}
	}
}

void MyStreamDeckPlugin::WillAppearForAction(const std::string& inAction, const std::string& inContext, const json &inPayload, const std::string& inDeviceID)
{
	// On key appearing, remember the context and store the settings
	std::unique_lock<std::mutex>lk(mVisibleContextsMutex);
	buttonData_t newButtonData{};
	if (inPayload.find("settings") != inPayload.end())
		readPayload(newButtonData.data, inPayload["settings"], lk);
	newButtonData.buttonTimer.reset(new TimerThread());

	mVisibleContexts.emplace( inContext, std::move(newButtonData) );

	if (!mIsRunning.load())
		newButtonData.lastErrorMsg = "Error: Bad\nInitialization";
	updateUI(inContext, lk);
}

void MyStreamDeckPlugin::WillDisappearForAction(const std::string& inAction, const std::string& inContext, const json &inPayload, const std::string& inDeviceID)
{
	// Remove the context
	std::unique_lock<std::mutex>lk(mVisibleContextsMutex);
	mVisibleContexts.erase(inContext);
}

void MyStreamDeckPlugin::DeviceDidConnect(const std::string& inDeviceID, const json &inDeviceInfo)
{
	// Nothing to do
}

void MyStreamDeckPlugin::DeviceDidDisconnect(const std::string& inDeviceID)
{
	// Nothing to do
}

/**
 * Reads inPayload for commands from PI and runs them
 *
 * @param[in] inContext the context the payload belongs to
 * @param[in] inPayload the json payload to read
 * @param[in] lk the lock for mutex mVisibleContextsMutex
 */
void MyStreamDeckPlugin::runPICommands(const std::string& inContext, const json& inPayload, const std::unique_lock<std::mutex>& lk)
{
	assert(lk.owns_lock());
	assert(lk.mutex() == &mVisibleContextsMutex);

	if (mConnectionManager == nullptr)
		return;

	if (inPayload.find("command") != inPayload.end())
	{
		if (mVisibleContexts.find(inContext) == mVisibleContexts.end())
			return;

		std::optional<std::string>& lastErrorMsg = mVisibleContexts.at(inContext).lastErrorMsg;

		if (inPayload["command"] == "getSampleCommand")
		{
			// construct the command string and send it back to PI

			json j;
			const contextData_t& data = mVisibleContexts.at(inContext).data;
			std::string exe = videodownloadutils::getYoutubeDlExePath(data.youtubeDlExePath);

			// first grab all the cmds based on selected options
			std::vector<std::string> cmds;
			try
			{
				cmds = videodownloadutils::getCommandQueue("url",
					data.outputFolder,
					std::nullopt,
					data.maxDownloads,
					data.downloadFormats,
					data.customCommand);
			}
			catch (std::runtime_error e)
			{
				mConnectionManager->LogMessage("Error: context " + inContext + " cannot get download command.");
				mConnectionManager->LogMessage(e.what());
			}

			// group all commands into json string
			std::string allCmds;
			for (const auto& cmd : cmds)
			{
				allCmds += exe + cmd + "\n";
			}
			j["sampleCommand"] = allCmds;

			// send
			mConnectionManager->SendToPropertyInspector("", inContext, j);
		}
		else if (inPayload["command"] == "update")
		{
			contextData_t & data = mVisibleContexts.at(inContext).data;
			if (mActiveDownloads.size() > 0)
			{
				lastErrorMsg = "youtube-dl\nin use.";
				mConnectionManager->LogMessage("Error: context " + inContext + " requested update but downloads are still pending.");
			}
			else
			{
				mIsUpdating = true;
				lastErrorMsg = "Updating\n";
				submitDownloadTask("", data, inContext, true, lk);
			}
		}
		else if (inPayload["command"] == "killContext")
		{
			mConnectionManager->LogMessage("Killing threads spawned by context: " + inContext);
			lastErrorMsg = "Stopping\nDownloads";
			if (mActiveDownloads.find(inContext) != mActiveDownloads.end())
			{
				for (const auto& thd : mActiveDownloads.at(inContext).threads)
				{
					thd->kill();
				}
			}
		}
		else if (inPayload["command"] == "killAll")
		{
			mConnectionManager->LogMessage("Killing all threads");
			lastErrorMsg = "Stopping All\nDownloads";
			for (const auto ctx : mActiveDownloads)
			{
				for (const auto& thd : ctx.second.threads)
				{
					thd->kill();
				}
			}
		}
	}
}

void MyStreamDeckPlugin::SendToPlugin(const std::string& inAction, const std::string& inContext, const json &inPayload, const std::string& inDeviceID)
{
	std::unique_lock<std::mutex>lk(mVisibleContextsMutex);
	// on settings change, store the new settings
	if (mVisibleContexts.find(inContext) != mVisibleContexts.end())
		readPayload(mVisibleContexts.at(inContext).data, inPayload, lk);

	runPICommands(inContext, inPayload, lk);

	updateUI(inContext, lk);
}