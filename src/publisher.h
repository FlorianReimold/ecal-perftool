#pragma once

#include <ecal/ecal_publisher.h>
#include <thread>
#include <memory>
#include <mutex>
#include <vector>
#include <string>

#include "publisher_statistics.h"

class Publisher
{
//////////////////////////////////////
/// Publisher, Destructor
//////////////////////////////////////
public:
  // Constructor that gets a frequency in Hz
  Publisher(const std::string& topic_name, double frequency, size_t payload_size);

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
};
