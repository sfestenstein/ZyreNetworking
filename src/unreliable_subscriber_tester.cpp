#include "HighBandwidthSubscriber.h"
#include <chrono>
#include <csignal>
#include <iostream>
#include <thread>

#include <MessageOne.pb.h>
#include <MessageTwo.pb.h>

static volatile bool running = true;

void signalHandler(int signum) {
    std::cout << "\nReceived signal " << signum << ", shutting down..." << std::endl;
    running = false;
}

int main() 
{
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    // High-bandwidth UDP multicast subscriber
    HighBandwidthSubscriber sub("TestZyre", "239.192.1.1", 5670);

    // Subscribe to topics
    sub.subscribe("MessageOne", [](const std::string &topic, const std::string &data)
    {
        MessageOne msg;
        if (msg.ParseFromString(data)) 
        {
            std::cout << "Received on " << topic << " (size: " << msg.mcmessagestring().size() 
                      << " bytes): " << msg.mcmessagestring().substr(0, 50) << "..."
                      << " at " << msg.mntime() << std::endl;
        } 
        else 
        {
            std::cerr << "Failed to parse MessageOne" << std::endl;
        }
    });

    sub.subscribe("MessageTwo", [](const std::string &topic, const std::string &data)
    {
        MessageTwo msg;
        if (msg.ParseFromString(data)) 
        {
            std::cout << "Received on " << topic << ": " << msg.mcmessagestring() 
                      << " at " << msg.mntime() << std::endl;
        } 
        else 
        {
            std::cerr << "Failed to parse MessageTwo" << std::endl;
        }
    });

    // Start receiving
    if (!sub.start())
    {
        std::cerr << "Failed to start subscriber" << std::endl;
        return 1;
    }

    std::cout << "Subscriber running. Press Ctrl+C to exit." << std::endl;

    // Keep the main thread alive
    while (running)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    sub.stop();
    std::cout << "Subscriber stopped." << std::endl;

    return 0;
}
