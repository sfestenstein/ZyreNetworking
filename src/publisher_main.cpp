#include "ZyrePublisher.h"
#include <chrono>
#include <iostream>
#include <thread>

#include <MessageOne.pb.h>
#include <MessageTwo.pb.h>

int main(int argc, char **argv) 
{
    ZyrePublisher pub("TestZyre");
    pub.start();

    while (1)
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
    pub.stop();
    return 0;
}
