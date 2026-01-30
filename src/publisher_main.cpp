#include "ZyrePublisher.h"
#include <chrono>
#include <iostream>
#include <thread>

#include <czmq.h>

#include <MessageOne.pb.h>
#include <MessageTwo.pb.h>

int main() 
{
    // Disable CZMQ's signal handling so we can use our own
    zsys_handler_set(NULL);

    ZyrePublisher pub("TestZyre");
    pub.start();

    while (true)
    {
        auto now = std::chrono::system_clock::now();
        auto epochSeconds = std::chrono::duration_cast<std::chrono::seconds>(
            now.time_since_epoch()).count();

        MessageOne msg1;
        msg1.set_mcmessagestring("Hello from Message One");
        msg1.set_mntime(epochSeconds);
        pub.publish("MessageOne", msg1);
        std::cout << "Published MessageOne " << std::endl;

        MessageTwo msg2;
        msg2.set_mcmessagestring("Hello from Message Two");
        msg2.set_mntime(epochSeconds);
        pub.publish("MessageTwo", msg2);
        std::cout << "Published MessageTwo " << std::endl;

        std::this_thread::sleep_for(std::chrono::seconds(2));
    }

    std::cout << "Shutting down..." << std::endl;
    pub.stop();
    return 0;
}
