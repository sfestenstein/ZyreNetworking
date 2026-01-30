#include "ZyreSubscriber.h"

#include <iostream>

ZyreSubscriber::ZyreSubscriber(const std::string &name, 
                               const std::string &topic) :
    ZyreNode(name), 
    _topic(topic) 
{
    
}

ZyreSubscriber::~ZyreSubscriber() 
{

}

void ZyreSubscriber::run() 
{
    if (!start()) 
    {
        std::cerr<<"Failed to start subscriber node"<<std::endl;
        return;
    }

    zyre_join(_node, _topic.c_str());

    while (_isRunning.load()) 
    {
        zyre_event_t *event = zyre_event_new(_node);
        if (!event) break;

        const char *type = zyre_event_type(event);
        const char *name = zyre_event_peer_name(event);

        if (type && strcmp(type, "SHOUT") == 0) 
        {
            const char *group = zyre_event_group(event);
            zmsg_t *zmsg = zyre_event_get_msg(event);
            if (zmsg) 
            {
                char *payload = zmsg_popstr(zmsg);
                if (payload) 
                {
                    std::cout<<"Received from "<< (name) <<" on '" <<(_topic.c_str()) << "': " <<payload<<std::endl;
                    zstr_free(&payload);
                } 
                else 
                {
                    zmsg_print(zmsg);
                }
                zmsg_destroy(&zmsg);
            }
        }

        zyre_event_destroy(&event);
    }
}
