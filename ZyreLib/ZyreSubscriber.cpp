#include "ZyreSubscriber.h"

#include <cstring>
#include <iostream>

ZyreSubscriber::ZyreSubscriber(const std::string &name) :
    ZyreNode(name)
{
    if (!start()) 
    {
        std::cerr << "Failed to start subscriber node" << std::endl;
        return;
    }

    // Spawn background thread to handle incoming messages
    _receiveThread = std::thread(&ZyreSubscriber::receiveLoop, this);
}

ZyreSubscriber::~ZyreSubscriber() 
{
    stop();
    if (_receiveThread.joinable())
    {
        _receiveThread.join();
    }
}

void ZyreSubscriber::subscribe(const std::string &topic, MessageHandler handler)
{
    {
        std::lock_guard<std::mutex> lock(_handlersMutex);
        _handlers[topic] = std::move(handler);
    }

    // Join the zyre group for this topic if node is running
    if (_node)
    {
        zyre_join(_node, topic.c_str());
    }
}

void ZyreSubscriber::receiveLoop() 
{
    while (true) 
    {
        zyre_event_t *event = zyre_event_new(_node);
        if (!event) break;  // zyre_stop() was called

        const char *type = zyre_event_type(event);

        if (type && strcmp(type, "SHOUT") == 0) 
        {
            const char *group = zyre_event_group(event);
            zmsg_t *zmsg = zyre_event_get_msg(event);
            
            if (zmsg && group) 
            {
                std::string topic(group);
                
                // Get raw binary data from the message
                zframe_t *frame = zmsg_first(zmsg);
                if (frame)
                {
                    std::string data(reinterpret_cast<const char*>(zframe_data(frame)), 
                                     zframe_size(frame));
                    
                    // Find and invoke the handler
                    MessageHandler handler;
                    {
                        std::lock_guard<std::mutex> lock(_handlersMutex);
                        auto it = _handlers.find(topic);
                        if (it != _handlers.end())
                        {
                            handler = it->second;
                        }
                    }
                    
                    if (handler)
                    {
                        handler(topic, data);
                    }
                }
                zmsg_destroy(&zmsg);
            }
        }

        zyre_event_destroy(&event);
    }
}
