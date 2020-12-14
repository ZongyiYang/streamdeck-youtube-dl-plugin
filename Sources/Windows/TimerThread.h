//==============================================================================
/**
@file       TimerThread.h
@brief      Thread that executes a selected function once after a certain amount of time
@copyright  (c) 2020, Zongyi Yang
**/
//==============================================================================

#pragma once

#include <atomic>
#include <mutex>
#include <functional>
#include <chrono>

class TimerThread
{
public:
	TimerThread()
	{
		lock();
	}

	~TimerThread()
	{
		stop();
	}

	/**
		@brief Unlocks mutex and stops/joins the thread
	**/
	void stop()
	{
		running = false;
		if (locked)
		{
			unlock();
		}
		if (thd.joinable())
			thd.join();
	}

	/**
		@brief waits for interval then calls the function

		@param[in] waitTime the milliseconds to wait
		@param[in] func the function to trigger
	**/
	void start(uint32_t waitTime, std::function<bool(void)> func)
	{
		// can't be already running
		if (running == true)
		{
			return;
		}

		running = true;
		completed = false;
		success = false;

		// start the timer thread
		thd = std::thread([this, waitTime, func]()
			{
				std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
				uint64_t timeLeft = waitTime;
				while (running)
				{
					// wait
					if (timerMutex.try_lock_for(std::chrono::milliseconds(timeLeft)))
					{
						unlock();
					}
					if (!locked)
					{
						lock();
					}
					std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
					uint64_t elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
					if (timeLeft > elapsedTime)
					{
						timeLeft -= elapsedTime;
					}
					else if (running)
					{
						completed = true;
						success = func();
						break;
					}
				}
			});
	};

	bool isCompleted()
	{
		return completed;
	}
	bool isSuccessful()
	{
		return success;
	}
private:
	std::atomic_bool running = false;
	std::thread thd;

	std::timed_mutex timerMutex;
	std::atomic_bool locked = false;

	std::atomic_bool completed = false;
	std::atomic_bool success = false;

	void lock()
	{
		timerMutex.lock();
		locked = true;
	}

	void unlock()
	{
		locked = false;
		timerMutex.unlock();
	}
};