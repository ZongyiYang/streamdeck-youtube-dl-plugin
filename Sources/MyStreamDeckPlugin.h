//==============================================================================
/**
@file       MyStreamDeckPlugin.h

@brief      Plugin for calling youtube-dl

@copyright  (c) 2020, Zongyi Yang

**/
//==============================================================================

#include "Common/ESDBasePlugin.h"
#include "Windows/Common.h"
#include "Windows/DownloadThread.h"
#include "Windows/TimerThread.h"
#include <mutex>
#include <atomic>
#include <optional>
#include <queue>
#include <unordered_map>
#include <unordered_set>


class DownloadThread;
class TimerThread;

class MyStreamDeckPlugin : public ESDBasePlugin
{
public:
	
	MyStreamDeckPlugin();
	virtual ~MyStreamDeckPlugin();
	
	void KeyDownForAction(const std::string& inAction, const std::string& inContext, const json &inPayload, const std::string& inDeviceID) override;
	void KeyUpForAction(const std::string& inAction, const std::string& inContext, const json &inPayload, const std::string& inDeviceID) override;
	
	void WillAppearForAction(const std::string& inAction, const std::string& inContext, const json &inPayload, const std::string& inDeviceID) override;
	void WillDisappearForAction(const std::string& inAction, const std::string& inContext, const json &inPayload, const std::string& inDeviceID) override;
	
	void DeviceDidConnect(const std::string& inDeviceID, const json &inDeviceInfo) override;
	void DeviceDidDisconnect(const std::string& inDeviceID) override;
	
	void SendToPlugin(const std::string& inAction, const std::string& inContext, const json &inPayload, const std::string& inDeviceID) override;

private:
	
	bool initYoutubeDl();
	
	struct buttonData_t
	{
		contextData_t data = {};
		std::unique_ptr<TimerThread> buttonTimer = nullptr;
		std::optional<std::string> lastErrorMsg = std::nullopt;
	};
	std::mutex mVisibleContextsMutex;
	std::unordered_map<std::string, buttonData_t> mVisibleContexts;
	
	std::thread mDlMonitor;
	std::atomic<bool> mIsRunning = false;
	std::atomic<bool> mIsUpdating = false;

	// data struct holding download threads per context
	struct downloadData_t
	{
		std::vector<std::shared_ptr<DownloadThread>> threads = {};
		uint32_t successCount = 0;
		uint32_t failureCount = 0;
	};
	std::unordered_map <std::string, downloadData_t> mActiveDownloads;

	// these are used for waking main thread for updates
	std::mutex cvMutex;
	std::condition_variable cv;
	std::queue<DownloadThread::threadData_t> results;

	void readPayload(contextData_t& data, const json& inPayload, const std::unique_lock<std::mutex>& lk);
	void runPICommands(const std::string& inContext, const json& inPayload, const std::unique_lock<std::mutex>& lk);

	void downloadMonitor();
	void submitDownloadTask(const std::string& url, const contextData_t& data, const std::string& inContext, const bool doUpdate, const std::unique_lock<std::mutex>& lk);
	void cleanupDownloads(const std::string& context, const std::unique_lock<std::mutex>& lk);
	std::unordered_set <std::string> getModifiedContexts(const std::unique_lock<std::mutex>& lk);
	void updateUI(const std::string & inContext, const std::unique_lock<std::mutex>& lk);
};
