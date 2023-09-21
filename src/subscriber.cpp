#include "subscriber.h"

#include <iostream>
#include <thread>

Subscriber::Subscriber(const std::string& topic_name, std::chrono::nanoseconds time_to_waste, bool busy_wait)
  : ecal_sub(topic_name)
  , time_to_waste_(time_to_waste)
  , busy_wait_(busy_wait_)
  , is_interrupted_(false)
  , statistics_size_(100)
{
  statistics_.reserve(statistics_size_);
  
  // create statistics thread
  statistics_thread_ = std::make_unique<std::thread>([this](){ this->statisticsLoop(); });

  ecal_sub.AddReceiveCallback([this](const char* topic_name_, const eCAL::SReceiveCallbackData* data_) { callback(topic_name_, data_); });
}

// Destructor
Subscriber::~Subscriber()
{
  // Interrupt the thread
  {
    std::unique_lock<std::mutex> lock(mutex_);
    is_interrupted_ = true;
    condition_variable_.notify_all();
  }

  // Join the thread
  statistics_thread_->join();
}

void Subscriber::callback(const char* topic_name_, const eCAL::SReceiveCallbackData* data_)
{
  SubscribedMessage message_info;
  message_info.local_receive_time = std::chrono::steady_clock::now();
  message_info.ecal_receive_time  = eCAL::Time::ecal_clock::now();
  message_info.ecal_send_time     = eCAL::Time::ecal_clock::time_point(std::chrono::microseconds(data_->time));
  message_info.ecal_counter       = data_->clock;
  message_info.size_bytes         = data_->size;

  if (time_to_waste_ >= std::chrono::nanoseconds::zero())
  {
    if (busy_wait_)
    {
      auto start = std::chrono::high_resolution_clock::now();
      while (std::chrono::high_resolution_clock::now() - start < time_to_waste_)
      {
        // Do nothing
      }
    }
    else
    {
      std::this_thread::sleep_for(time_to_waste_);
    }
  }

  {
    std::unique_lock<std::mutex>lock(mutex_);
    statistics_.push_back(message_info);
  }
}

void Subscriber::statisticsLoop()
{
  while (!is_interrupted_)
  {
    SubscriberStatistics statistics;
    statistics.reserve(statistics_size_ * 2);

    {
      std::unique_lock<std::mutex>lock (mutex_);
    
      condition_variable_.wait_for(lock, std::chrono::seconds(1), [this]() { return bool(is_interrupted_); });
    
      if (is_interrupted_)
        return;

      if(statistics_.size() > 1)
      {
        statistics_size_ = std::max(statistics_size_, statistics_.size());
        statistics_.swap(statistics);

        // Initialize the new statistics vector with the last element of the old one. This is important for detecting drops and properly computing the delay of the actual first message.
        statistics_.push_back(statistics.back());
      }
    }

    if (statistics.size() > 1)
      printStatistics(statistics);
    else
      std::cerr << "Not enough data" << std::endl;
  }
}
