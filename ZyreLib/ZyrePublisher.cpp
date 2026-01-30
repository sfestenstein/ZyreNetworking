#include "ZyrePublisher.h"

#include <iostream>
#include <thread>

#include <zyre.h>


ZyrePublisher::ZyrePublisher(const std::string &name,
                             const std::string &topic) :
    ZyreNode(name), 
    _topic(topic) 
{

}

ZyrePublisher::~ZyrePublisher()
{
    stop();
}

void ZyrePublisher::run() 
{
    if (!start()) 
    {
        std::cerr<<"Failed to start publisher node"<<std::endl;
        return;
    }

    // Announce presence to group (optional)
    zyre_shouts(_node, _topic.c_str(), "%s", "publisher-online");

    int count = 0;
    while (_isRunning.load()) 
    {
        char payload[256];
        snprintf(payload, sizeof(payload), "[%s] message %d", _topic.c_str(), ++count);

        zmsg_t *zmsg = zmsg_new();
        zmsg_addstr(zmsg, payload);
         std::cerr<<"shouting!"<<std::endl;
        if (zyre_shout(_node, _topic.c_str(), &zmsg) != 0) 
        {
            std::cerr<<"Failed to shout"<<std::endl;
            if (zmsg) zmsg_destroy(&zmsg);
        }

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}