#include "ZyreNode.h"

#include <zyre.h>

ZyreNode::ZyreNode(const std::string &name) : 
    _nodeName(name),
    _node(zyre_new(name.c_str()))
{
    
}

ZyreNode::~ZyreNode() 
{
    stop();

    if (_node) 
    {
        zyre_destroy(&_node);
        _node = nullptr;
    }
}

bool ZyreNode::start() 
{
    if (!_node) return false;

    // Reset stop state in case of restart
    {
        std::lock_guard<std::mutex> lock(_terminateMutex);
        _stopRequested = false;
    }
    _isRunning.store(true);

    // start zyre
    return zyre_start(_node) == 0;
}

void ZyreNode::stop() 
{
    {
        std::lock_guard<std::mutex> lock(_terminateMutex);
        if (_stopRequested) return;  // Already stopped
        _stopRequested = true;
    }
    _terminateCV.notify_all();

    _isRunning.store(false);

    if (_node) 
    {
        zyre_stop(_node);
    }
}
