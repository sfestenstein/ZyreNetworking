#include "HighBandwidthPublisher.h"
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

    // High-bandwidth UDP multicast publisher
    HighBandwidthPublisher pub("TestZyre", "239.192.1.1", 5670);

    int count = 0;
    while (running)
    {
        auto now = std::chrono::system_clock::now();
        auto epochSeconds = std::chrono::duration_cast<std::chrono::seconds>(
            now.time_since_epoch()).count();

        // Create a large message string to test fragmentation (> 1400 bytes MTU)
        std::string largePayload(3000, 'X');  // 3KB of data
        largePayload = "Large message #" + std::to_string(++count) + " [" + largePayload + "]";

        MessageOne msg1;
        msg1.set_mcmessagestring(largePayload);
        msg1.set_mntime(epochSeconds);
        pub.publish("MessageOne", msg1);
        std::cout << "Published MessageOne #" << count << " (size: " << largePayload.size() << " bytes)" << std::endl;

        MessageTwo msg2;
        msg2.set_mcmessagestring("Hello from Message Two #" + std::to_string(count));
        msg2.set_mntime(epochSeconds);
        pub.publish("MessageTwo", msg2);
        std::cout << "Published MessageTwo #" << count << std::endl;

        std::this_thread::sleep_for(std::chrono::seconds(2));
    }

    std::cout << "Shutting down..." << std::endl;
    return 0;
}
