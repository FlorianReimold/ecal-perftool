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

inline void printStatistics(const SubscriberStatistics& statistics, bool print_verbose_times)
{
  // Compute entire entire_duration from first to last message and the mean
  auto entire_duration         = statistics.back().local_receive_time - statistics.front().local_receive_time;

  // The first message is from the previous loop run and only exists to count lost messages properly and to compute the delay of the actual first message.
  int received_msgs        = statistics.size() - 1;

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

  // Get the minimum and maximum delay time between two messages
  auto msg_dt_min = std::chrono::steady_clock::duration::max();
  auto msg_dt_max = std::chrono::steady_clock::duration::min();
  for (size_t i = 1; i < statistics.size(); ++i)
  {
    auto delay = statistics[i].local_receive_time - statistics[i - 1].local_receive_time;
    if (delay < msg_dt_min)
      msg_dt_min = delay;
    if (delay > msg_dt_max)
      msg_dt_max = delay;
  }
  auto msg_dt_mean             = entire_duration / (statistics.size() - 1);

  auto msg_frequency           = 1.0 / std::chrono::duration_cast<std::chrono::duration<double>>(msg_dt_mean).count();

  // Compute mean delay time based on ecal send and receive time
  //auto delay_mean              = std::accumulate(statistics.begin(), statistics.end(), std::chrono::steady_clock::duration(0), [](auto sum, auto& msg){ return sum + (msg.ecal_receive_time - msg.ecal_send_time); }) / statistics.size();


  
  // Print mean entire_duration and rmse in a single line in milliseconds
  {
    std::stringstream ss;
    ss << std::right << std::fixed;
    ss << "["                     << std::setprecision(3) << std::setw(10) << std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::steady_clock::now().time_since_epoch()).count() << "]";
    ss << " | cnt:"               << std::setprecision(3) << std::setw(5) << received_msgs;
    ss << " | lost:"              << std::setprecision(3) << std::setw(5) << (ecal_counter_is_monotinc ? std::to_string(lost_msgs) : "???");
    ss << " | msg_dt(ms) mean:"   << std::setprecision(3) << std::setw(8) << std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(msg_dt_mean).count();
    ss << " ["                    << std::setprecision(3) << std::setw(8) << std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(msg_dt_min).count();
    ss << ","                     << std::setprecision(3) << std::setw(8) << std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(msg_dt_max).count();
    ss << "]";
    ss << " | msg_freq(Hz):"      << std::setprecision(1) << std::setw(7) << msg_frequency;

    std::cerr << ss.str() << std::endl;
  }

  // Print verbose times
  if (print_verbose_times)
  {
    std::stringstream ss;
    ss << std::right << std::fixed;
    ss << "  msg_dt(ms): ";
    for (size_t i = 1; i < statistics.size(); ++i)
    {
      if (i > 1)
        ss << " ";

      auto delay = statistics[i].local_receive_time - statistics[i - 1].local_receive_time;
      ss << std::setprecision(1) << std::setw(5) << std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(delay).count();
    }

    std::cerr << ss.str() << std::endl;
  }
}
