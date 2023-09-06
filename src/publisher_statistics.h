#pragma once

#include <vector>
#include <chrono>
#include <iostream>
#include <numeric>
#include <sstream>
#include <iomanip>

struct PublishedMessage
{
  std::chrono::steady_clock::time_point publish_time;
  std::chrono::steady_clock::duration   send_call_duration;
};

using PublisherStatistics = std::vector<PublishedMessage>;

inline void printStatistics(const PublisherStatistics& statistics)
{
  // Compute entire entire_duration from first to last message and the mean
  auto entire_duration         = statistics.back().publish_time - statistics.front().publish_time;

  auto loop_time_mean          = entire_duration / (statistics.size() - 1);
  auto loop_frequency          = 1.0 / std::chrono::duration_cast<std::chrono::duration<double>>(loop_time_mean).count();

  auto mean_send_call_duration = std::accumulate(statistics.begin(), statistics.end(), std::chrono::steady_clock::duration(0), [](auto sum, auto& msg){ return sum + msg.send_call_duration; }) / statistics.size();
  
  // Print mean entire_duration and rmse in a single line in milliseconds
  std::stringstream ss;
  ss << std::right << std::fixed;
  ss << "["                       << std::setprecision(3) << std::setw(10) << std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::steady_clock::now().time_since_epoch()).count() << "]";
  ss << " | cnt: "                << std::setprecision(3) << std::setw(6) << statistics.size();
  ss << " | mean_loop_dt (ms): "  << std::setprecision(3) << std::setw(7) << std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(loop_time_mean).count();
  ss << " | loop_freq (Hz): "     << std::setprecision(1) << std::setw(7) << loop_frequency;
  ss << " | mean_snd_dt (ms): "    << std::setprecision(3) << std::setw(7) << std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(mean_send_call_duration).count();

  std::cerr << ss.str() << std::endl;

}
