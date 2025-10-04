#include "zyre_node.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>
#include <csignal>
#include <mutex>

// Global atomic to indicate process-level termination (set in signal handler)
static std::atomic<bool> g_terminate{false};

static void signal_handler(int) {
    g_terminate.store(true);
}

ZyreNode::ZyreNode(const std::string &name) : node_name_(name) {
    node_ = zyre_new(name.c_str());
}

ZyreNode::~ZyreNode() {
    // Request termination and join terminator thread if running
    g_terminate.store(true);
    if (terminator_thread_.joinable()) {
        terminator_thread_.join();
    }

    if (node_) {
        zyre_stop(node_);
        zyre_destroy(&node_);
        node_ = nullptr;
    }
}

bool ZyreNode::start() {
    if (!node_) return false;
    // Install signal handler once (safe to call multiple times)
    static std::once_flag sig_once;
    std::call_once(sig_once, [](){
        struct sigaction sa;
        sa.sa_handler = signal_handler;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = 0;
        sigaction(SIGINT, &sa, nullptr);
        sigaction(SIGTERM, &sa, nullptr);
    });

    // start zyre
    if (zyre_start(node_) != 0) return false;

    // Start terminator thread that watches g_terminate and calls zyre_stop
    terminator_thread_ = std::thread([this](){
        while (!g_terminate.load()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
        // signal received: request zyre stop to unblock event loops
        if (this->node_) zyre_stop(this->node_);
        // also set running flag so loops exit
        this->running_flag_.store(false);
    });

    return true;
}

void ZyreNode::stop() {
    if (node_) zyre_stop(node_);
    running_flag_.store(false);
    if (terminator_thread_.joinable()) {
        // make sure we don't hang if terminator is waiting for signal
        g_terminate.store(true);
        terminator_thread_.join();
    }
}

Publisher::Publisher(const std::string &name, const std::string &topic)
    : ZyreNode(name), topic_(topic) {}

Publisher::~Publisher()
{
    stop();
}

void Publisher::run() {
    if (!start()) {
        std::cerr<<"Failed to start publisher node"<<std::endl;
        return;
    }

    // Announce presence to group (optional)
    zyre_shouts(node_, topic_.c_str(), "%s", "publisher-online");

    int count = 0;
    while (running_flag_.load()) {
        char payload[256];
        snprintf(payload, sizeof(payload), "[%s] message %d", topic_.c_str(), ++count);

        zmsg_t *zmsg = zmsg_new();
        zmsg_addstr(zmsg, payload);
         std::cerr<<"shouting!"<<std::endl;
        if (zyre_shout(node_, topic_.c_str(), &zmsg) != 0) {
            std::cerr<<"Failed to shout"<<std::endl;
            if (zmsg) zmsg_destroy(&zmsg);
        }

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

Subscriber::Subscriber(const std::string &name, const std::string &topic)
    : ZyreNode(name), topic_(topic) {}

Subscriber::~Subscriber() {}

void Subscriber::run() {
    if (!start()) {
        std::cerr<<"Failed to start subscriber node"<<std::endl;
        return;
    }

    zyre_join(node_, topic_.c_str());

    while (running_flag_.load()) {
        zyre_event_t *event = zyre_event_new(node_);
        if (!event) break;

        const char *type = zyre_event_type(event);
        const char *name = zyre_event_peer_name(event);

        if (type && strcmp(type, "SHOUT") == 0) {
            const char *group = zyre_event_group(event);
            zmsg_t *zmsg = zyre_event_get_msg(event);
            if (zmsg) {
                char *payload = zmsg_popstr(zmsg);
                if (payload) {
                    std::cout<<"Received from "<< (name?name:"unknown") <<" on '"<<(group?group:topic_.c_str())<<"': "<<payload<<std::endl;
                    zstr_free(&payload);
                } else {
                    zmsg_print(zmsg);
                }
                zmsg_destroy(&zmsg);
            }
        }

        zyre_event_destroy(&event);
    }
}
