// Copyright (c) Continental. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for details.

#pragma once

#include <ecal/ecal_subscriber.h>
#include <chrono>

#include <memory>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include "subscriber_statistics.h"

class Subscriber
{
//////////////////////////////////////
/// Publisher, Destructor
//////////////////////////////////////
public:
  // Constructor that gets a frequency in Hz
  Subscriber(const std::string&                   topic_name
            , std::chrono::nanoseconds            time_to_waste
            , bool                                busy_wait
            , bool                                hickup
            , std::chrono::steady_clock::duration wait_before_hickup
            , std::chrono::steady_clock::duration hickup_delay
            , bool                                quiet
            , bool                                log_print_verbose_times);

  // Destructor
  ~Subscriber();

//////////////////////////////////////
/// Implementation
//////////////////////////////////////
private:
  void callback(const char* topic_name_, const eCAL::SReceiveCallbackData* data_);

  void statisticsLoop();

//////////////////////////////////////
/// Member variables
//////////////////////////////////////
private:
  eCAL::CSubscriber            ecal_sub;
  std::chrono::nanoseconds     time_to_waste_;
  bool                         busy_wait_;

  bool                                        hickup_;
  const std::chrono::steady_clock::duration   wait_before_hickup_;
  std::chrono::steady_clock::time_point       hickup_time_;
  const std::chrono::steady_clock::duration   hickup_delay_;

  std::unique_ptr<std::thread>    statistics_thread_;
  mutable std::mutex              mutex_;
  mutable std::condition_variable condition_variable_;
  std::atomic<bool>               is_interrupted_;
  SubscriberStatistics            statistics_;
  size_t                          statistics_size_;

  const bool                      log_print_verbose_times_;
};
