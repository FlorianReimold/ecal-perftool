// Copyright (c) Continental. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for details.

#pragma once

#include <ecal/ecal_publisher.h>
#include <thread>
#include <memory>
#include <mutex>
#include <vector>
#include <string>
#include <atomic>
#include <condition_variable>

#include "publisher_statistics.h"

class Publisher
{
//////////////////////////////////////
/// Publisher, Destructor
//////////////////////////////////////
public:
  // Constructor that gets a frequency in Hz
  Publisher(const std::string& topic_name, double frequency, std::size_t payload_size, bool quiet, bool log_print_verbose_times);

  // Destructor
  ~Publisher();

//////////////////////////////////////
/// Implementation
//////////////////////////////////////
private:
  void loop();
  void statisticsLoop();

  bool preciseWaitUntil(std::chrono::steady_clock::time_point time) const;

//////////////////////////////////////
/// Member variables
//////////////////////////////////////
private:
  eCAL::CPublisher                      ecal_pub;
  const double                          frequency_;
  std::vector<char>                     payload_;

  std::unique_ptr<std::thread>          publisher_thread_;
  std::unique_ptr<std::thread>          statistics_thread_;

  std::chrono::nanoseconds              period_;
  std::chrono::steady_clock::time_point next_deadline_;

  mutable std::mutex                    mutex_;
  mutable std::condition_variable       condition_variable_;
  std::atomic<bool>                     is_interrupted_;
  PublisherStatistics                   statistics_;

  const bool                            log_print_verbose_times_;
};
