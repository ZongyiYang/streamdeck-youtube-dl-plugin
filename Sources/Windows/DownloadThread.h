//==============================================================================
/**
@file       DownloadThread.h

@brief		Runs a youtube-dl download process

@copyright  (c) 2020, Zongyi Yang.

**/
//==============================================================================

#pragma once
#include "Common.h"

#include <string>
#include <atomic>
#include <mutex>
#include <thread>
#include <filesystem>
#include <optional>
#include <queue>

#include "../Vendor/json/src/json.hpp"
using json = nlohmann::json;

class DownloadThread : public std::enable_shared_from_this<DownloadThread>
{
public:
	enum flags_t
	{
		CONTINUE,
		DETACH,
		KILL,
	};

	enum status_t
	{
		NEW,
		SETUP,
		RUNNING,
		STOPPING,
		FAILED,
		SUCCESS,
		UPDATED,
		UNKNOWN,
	};

	struct threadData_t
	{
		std::optional<std::string> buttonMsg = std::nullopt;
		std::optional<std::string> log = std::nullopt;
		status_t status = UNKNOWN;
		std::string context = "";
	};

	~DownloadThread()
	{
		kill();
		if (t.joinable())
			t.join();
	}

	/**
	 * Start download thread
	 *
	 * @param[in] url the url to download from
	 * @param[in] data the metadata stored by the context
	 * @param[in] inContext the button context for this thread
	 * @param[in] doUpdate update youtube-dl
	 * @param[in] cvMutex the mutex to lock for the cv
	 * @param[in] cv the condition variable to wake on completion
	 * @param[in] results the queue to place finished results data
	 */
	void start(const std::string& url, const contextSettings_t& data, const std::string& inContext, const bool doUpdate,
		       std::mutex& cvMutex, std::condition_variable& cv,
		       std::queue<threadData_t>& results)
	{
		// can't be already running
		status_t testVal = NEW;
		bool started = mState.compare_exchange_strong(testVal, SETUP);
		if (!started)
			return;

		mData.context = inContext;

		t = std::thread(&DownloadThread::launchDownloadProcess, this, url, data, doUpdate,
			            std::ref(cvMutex), std::ref(cv), std::ref(results));
	}

	void detach()
	{
		std::unique_lock<std::mutex> lk{ mCommandMutex };
		mCommand = DETACH;
		mPtr = shared_from_this();

		t.detach();
	}

	void kill()
	{
		{
			std::unique_lock<std::mutex> lk{ mCommandMutex };
			mCommand = KILL;
			if (mState.load() == RUNNING)
				if (pi.hProcess != NULL)
					TerminateProcess(pi.hProcess, 0);
		}
	}

	bool isComplete()
	{
		status_t currState = mState.load();
		return (currState == SUCCESS) || (currState == FAILED) || (currState == UPDATED);
	}
private:
	// pointer to self which is used to keep alive if detached;
	std::shared_ptr<DownloadThread> mPtr = nullptr;

	// current download state
	std::atomic<status_t> mState = NEW;

	// command to exit download loop
	std::mutex mCommandMutex;
	std::atomic<flags_t> mCommand = CONTINUE;
	PROCESS_INFORMATION pi;

	std::thread t;
	std::mutex mDataMutex;
	threadData_t mData;

	void launchDownloadProcess(const std::string url, const contextSettings_t data, const bool doUpdate,
		std::mutex& cvMutex, std::condition_variable& cv,
		std::queue <threadData_t> & results);
};
