#include <ecal/ecal.h>

#include "publisher.h"
#include "subscriber.h"

#include <iostream>
#include <thread>

void printUsage(const std::string& arg0)
{
  std::cout << "Usage:" << std::endl;
  std::cout << "  " << arg0 << " pub <topic_name> <frequency_hz> <payload_size_bytes>" << std::endl;
  std::cout << "or:" << std::endl;
  std::cout << "  " << arg0 << " sub <topic_name> [callback_delay_ms]" << std::endl;
}

int main(int argc, char** argv)
{
  // convert argc, argv to vector of strings
  std::vector<std::string> args;
  args.reserve(static_cast<size_t>(argc));
  for (int i = 0; i < argc; ++i)
  {
    args.emplace_back(argv[i]);
  }

  // Check for -h / --help
  if (args.size() < 2
      || std::find(args.begin(), args.end(), "-h") != args.end()
      || std::find(args.begin(), args.end(), "--help") != args.end())
  {
    printUsage(args[0]);
    return 0;
  }
  else if (args[1] == "pub")
  {
    if (args.size() != 5)
    {
      std::cerr << "Invalid number of parameters" << std::endl;
      printUsage(args[0]);
      return 1;
    }
    std::string        topic_name         = args[2];
    double             frequency_hz       = std::stod(args[3]);
    unsigned long long payload_size_bytes = std::stoull(args[4]);

    // Initialize eCAL
    eCAL::Initialize(argc, argv, "ecal-perftool");
    eCAL::Util::EnableLoopback(true);
    
    Publisher publisher(topic_name, frequency_hz, payload_size_bytes);
    
    // Just don't exit
    while (eCAL::Ok())
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    // finalize eCAL API
    eCAL::Finalize();
  }
  else if (args[1] == "sub")
  {
    if (args.size() < 3 || args.size() > 4)
    {
      std::cerr << "Invalid number of parameters" << std::endl;
      printUsage(args[0]);
      return 1;
    }

    std::string               topic_name = args[2];
    std::chrono::milliseconds callback_delay((args.size() >= 4 ? std::stoull(args[3]) : 0));

    
    // Initialize eCAL
    eCAL::Initialize(argc, argv, "ecal-perftool");
    eCAL::Util::EnableLoopback(true);
    
    Subscriber subscriber(topic_name, callback_delay, false);

    // Just don't exit
    while (eCAL::Ok())
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    // finalize eCAL API
    eCAL::Finalize();
  }
  else
  {
    std::cerr << "Invalid parameter: " << args[1] << std::endl;
    printUsage(args[0]);
    return 1;
  }

  return 0;
}
