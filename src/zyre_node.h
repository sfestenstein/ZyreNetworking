#pragma once

#include <zyre.h>
#include <czmq.h>
#include <string>
#include <atomic>
#include <thread>

class ZyreNode {
public:
    explicit ZyreNode(const std::string &name);
    virtual ~ZyreNode();

    // start the node; returns true on success
    bool start();
    // request stop (calls zyre_stop)
    void stop();
    const std::string &name() const { return node_name_; }

protected:
    zyre_t *node_ = nullptr;
    std::string node_name_;

    // Running flag that derived classes should check
    std::atomic<bool> running_flag_{true};

    // Thread that watches for process-level termination and calls zyre_stop
    std::thread terminator_thread_;
};

class Publisher : public ZyreNode {
public:
    Publisher(const std::string &name, const std::string &topic);
    ~Publisher();
    void run();

private:
    std::string topic_;
};

class Subscriber : public ZyreNode {
public:
    Subscriber(const std::string &name, const std::string &topic);
    ~Subscriber();
    void run();

private:
    std::string topic_;
};
