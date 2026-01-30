#ifndef ZYREPUBLISHER_H
#define ZYREPUBLISHER_H

#include "ZyreNode.h"
class ZyrePublisher : public ZyreNode
{
public:
    ZyrePublisher(const std::string &name, const std::string &topic);
    ~ZyrePublisher();
    void run();
private:
    std::string _topic;
};

#endif // ZYREPUBLISHER_H
