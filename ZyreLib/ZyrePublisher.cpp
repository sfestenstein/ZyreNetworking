#include "ZyrePublisher.h"

#include <iostream>

#include <zyre.h>


ZyrePublisher::ZyrePublisher(const std::string &name) :
    ZyreNode(name)
{
}

ZyrePublisher::~ZyrePublisher()
{
    _isRunning.store(false);
    stop();
}

bool ZyrePublisher::publish(const std::string &topic, const google::protobuf::Message &message)
{
    if (!_node || !_isRunning.load()) 
    {
        std::cerr << "Publisher not running" << std::endl;
        return false;
    }

    // Serialize the protobuf message
    std::string serialized;
    if (!message.SerializeToString(&serialized)) 
    {
        std::cerr << "Failed to serialize protobuf message" << std::endl;
        return false;
    }

    // Create namespaced group name
    std::string namespacedTopic = _nodeName + "/" + topic;

    // Create zmsg and add the serialized data
    zmsg_t *zmsg = zmsg_new();
    zmsg_addmem(zmsg, serialized.data(), serialized.size());

    if (zyre_shout(_node, namespacedTopic.c_str(), &zmsg) != 0) 
    {
        std::cerr << "Failed to shout on topic: " << topic << std::endl;
        if (zmsg) zmsg_destroy(&zmsg);
        return false;
    }

    return true;
}