
#include "ZyreNode.h"

class ZyreSubscriber : public ZyreNode 
{
public:
    ZyreSubscriber(const std::string &name, const std::string &topic);
    ~ZyreSubscriber();
    void run();

private:
    std::string _topic;
};