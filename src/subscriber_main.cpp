#include "ZyreSubscriber.h"
#include <chrono>
#include <iostream>
#include <thread>

#include <MessageOne.pb.h>
#include <MessageTwo.pb.h>

int main(int argc, char **argv) 
{
    ZyreSubscriber sub("TestZyre");

    sub.subscribe("MessageOne", [](const std::string &topic, const std::string &data)
    {
        MessageOne msg;
        if (msg.ParseFromString(data)) 
        {
            std::cout << "Received on " << topic << ": " << msg.mcmessagestring() 
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

    // Keep the main thread alive - subscriber runs in background
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}
