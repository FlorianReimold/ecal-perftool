#pragma once

#include <vector>
#include <chrono>
#include <iostream>
#include <numeric>
#include <sstream>
#include <iomanip>

#include <ecal/ecal_time.h>

struct SubscribedMessage
{
  std::chrono::steady_clock::time_point local_receive_time;
  eCAL::Time::ecal_clock::time_point    ecal_send_time;
  eCAL::Time::ecal_clock::time_point    ecal_receive_time;
  unsigned long long                    size_bytes;
  unsigned long long                    ecal_counter;
};

using SubscriberStatistics = std::vector<SubscribedMessage>;

inline void printStatistics(const SubscriberStatistics& statistics)
{
  // Compute entire entire_duration from first to last message and the mean
  auto entire_duration         = statistics.back().local_receive_time - statistics.front().local_receive_time;

  auto msg_dt_mean             = entire_duration / (statistics.size() - 1);
  auto msg_frequency           = 1.0 / std::chrono::duration_cast<std::chrono::duration<double>>(msg_dt_mean).count();

  // Compute mean delay time based on ecal send and receive time
  auto delay_mean              = std::accumulate(statistics.begin(), statistics.end(), std::chrono::steady_clock::duration(0), [](auto sum, auto& msg){ return sum + (msg.ecal_receive_time - msg.ecal_send_time); }) / statistics.size();

  // Check if the ecal_counter is continous. If not, we have lost messages. Count them.
  bool ecal_counter_is_monotinc = true;
  unsigned long long lost_msgs  = 0;
  for (size_t i = 1; i < statistics.size(); ++i)
  {
    if (statistics[i].ecal_counter <= statistics[i - 1].ecal_counter)
    {
      ecal_counter_is_monotinc = false;
      break;
    }
    lost_msgs += statistics[i].ecal_counter - statistics[i - 1].ecal_counter - 1;
  }

  
  // Print mean entire_duration and rmse in a single line in milliseconds
  std::stringstream ss;
  ss << std::right << std::fixed;
  ss << "["                       << std::setprecision(3) << std::setw(10) << std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::steady_clock::now().time_since_epoch()).count() << "]";
  ss << " | cnt: "                << std::setprecision(3) << std::setw(6) << statistics.size();
  ss << " | lost: "               << std::setprecision(3) << std::setw(6) << (ecal_counter_is_monotinc ? std::to_string(lost_msgs) : "???");
  ss << " | msg_dt_mean (ms): "  << std::setprecision(3) << std::setw(7) << std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(msg_dt_mean).count();
  ss << " | msg_freq (Hz): "     << std::setprecision(1) << std::setw(7) << msg_frequency;

  std::cerr << ss.str() << std::endl;

}
