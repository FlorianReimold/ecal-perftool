#include "publisher.h"

#include <iostream>

#include <ecal/ecal.h>

#ifdef WIN32
  #define NOMINMAX
  #define WIN32_LEAN_AND_MEAN
  #include <Windows.h>
#else
  #include <unistd.h>
#endif // WIN32

Publisher::Publisher(const std::string& topic_name, double frequency, std::size_t payload_size)
  : ecal_pub        (topic_name)
  , frequency_      (frequency)
  , is_interrupted_ (false)
  , payload_        (payload_size)
  , next_deadline_  (std::chrono::steady_clock::now() + period_)
  , period_         (std::chrono::nanoseconds(static_cast<long long>(1e9 / frequency)))
{
  statistics_.reserve(static_cast<size_t>((frequency + 1.0) * 1.2));

  // Start the thread
  publisher_thread_ = std::make_unique<std::thread>([this](){ this->loop(); });
  statistics_thread_ = std::make_unique<std::thread>([this](){ this->statisticsLoop(); });
}

// Destructor
Publisher::~Publisher()
{
  // Interrupt the thread
  {
    std::unique_lock<std::mutex> lock(mutex_);
    is_interrupted_ = true;
    condition_variable_.notify_all();
  }

  // Join the thread
  publisher_thread_->join();
  statistics_thread_->join();
}

void Publisher::loop()
{
  while (!is_interrupted_)
  {
    PublishedMessage message_info;

    auto timepoint_snd_start = std::chrono::steady_clock::now();
    ecal_pub.Send(payload_.data(), payload_.size());
    auto timepoint_snd_end = std::chrono::steady_clock::now();

    if (next_deadline_ > std::chrono::steady_clock::now())
    {
      preciseWaitUntil(next_deadline_);
      next_deadline_ += period_;
    }
    else
    {
      next_deadline_ = std::chrono::steady_clock::now() + period_;
    }
    auto timepoint_wait_end = std::chrono::steady_clock::now();

    message_info.publish_time       = timepoint_snd_start;
    message_info.send_call_duration = timepoint_snd_end - timepoint_snd_start;

    {
      std::lock_guard<std::mutex>lock (mutex_);
      statistics_.push_back(message_info);
    }
  }
}

void Publisher::statisticsLoop()
{
  while (!is_interrupted_)
  {
    PublisherStatistics statistics;
    statistics.reserve(static_cast<size_t>((frequency_ + 1.0) * 1.2));

    {
      std::unique_lock<std::mutex>lock (mutex_);
    
      condition_variable_.wait_for(lock, std::chrono::seconds(1), [this]() { return bool(is_interrupted_); });
    
      if (is_interrupted_)
        return;

      if(statistics_.size() > 1)
        statistics_.swap(statistics);

      // Initialize the new statistics vector with the last element of the old one. This is important for properly computing the loop time of the actual first message.
      statistics_.push_back(statistics.back());
    }

    if (statistics.size() > 1)
      printStatistics(statistics);
    else
      std::cerr << "Not enough data" << std::endl;
  }
}
  
bool Publisher::preciseWaitUntil(std::chrono::steady_clock::time_point time) const
{
  constexpr auto max_time_to_poll_wait = std::chrono::milliseconds(20);
  constexpr auto max_time_to_busy_wait = std::chrono::microseconds(3);

  while(true)
  {
    auto remaining_time_to_wait = time - std::chrono::steady_clock::now();
    
    if (remaining_time_to_wait <= max_time_to_busy_wait)
    {
      while (std::chrono::steady_clock::now() < time)
      {
        // Busy wait
      }
      if (is_interrupted_)
        return false;
      else
        return true;
    }
    else if (remaining_time_to_wait <= max_time_to_poll_wait)
    {
      while (std::chrono::steady_clock::now() < (time - max_time_to_busy_wait))
      {
#ifdef WIN32
        Sleep(0);
#else
        usleep(1);
#endif
        if (is_interrupted_)
          return false;
      }
    }
    else
    {
      std::unique_lock<std::mutex> lock(mutex_);
      condition_variable_.wait_for(lock, remaining_time_to_wait, [this](){ return bool(is_interrupted_); });

      if (is_interrupted_)
        return false;
    }

  }
}
