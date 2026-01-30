#include "HighBandwidthSubscriber.h"
#include "HighBandwidthPublisher.h"  // For FragmentHeader definition

#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <sys/socket.h>
#include <unistd.h>
#include <poll.h>

HighBandwidthSubscriber::HighBandwidthSubscriber(const std::string &name,
                                                 const std::string &multicastAddr,
                                                 uint16_t port,
                                                 int reassemblyTimeoutMs) :
    _name(name),
    _multicastAddr(multicastAddr),
    _port(port),
    _reassemblyTimeoutMs(reassemblyTimeoutMs),
    _socket(-1)
{
}

HighBandwidthSubscriber::~HighBandwidthSubscriber()
{
    stop();

    if (_socket >= 0)
    {
        close(_socket);
    }
}

void HighBandwidthSubscriber::subscribe(const std::string &topic, MessageHandler handler)
{
    if (_running.load())
    {
        std::cerr << "Cannot subscribe after start()" << std::endl;
        return;
    }

    // Create namespaced topic
    std::string namespacedTopic = _name + "/" + topic;

    std::lock_guard<std::mutex> lock(_handlersMutex);
    _handlers[namespacedTopic] = std::move(handler);
}

bool HighBandwidthSubscriber::start()
{
    if (_running.load())
    {
        return true;
    }

    // Create UDP socket
    _socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (_socket < 0)
    {
        std::cerr << "Failed to create UDP socket: " << strerror(errno) << std::endl;
        return false;
    }

    // Allow multiple sockets to use the same port
    int reuse = 1;
    if (setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0)
    {
        std::cerr << "Failed to set SO_REUSEADDR: " << strerror(errno) << std::endl;
    }

    // Bind to the multicast port
    struct sockaddr_in localAddr;
    memset(&localAddr, 0, sizeof(localAddr));
    localAddr.sin_family = AF_INET;
    localAddr.sin_port = htons(_port);
    localAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(_socket, reinterpret_cast<struct sockaddr*>(&localAddr), sizeof(localAddr)) < 0)
    {
        std::cerr << "Failed to bind socket to port " << _port << ": " << strerror(errno) << std::endl;
        close(_socket);
        _socket = -1;
        return false;
    }

    // Join the multicast group
    struct ip_mreq mreq;
    if (inet_pton(AF_INET, _multicastAddr.c_str(), &mreq.imr_multiaddr) != 1)
    {
        std::cerr << "Invalid multicast address: " << _multicastAddr << std::endl;
        close(_socket);
        _socket = -1;
        return false;
    }
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);

    if (setsockopt(_socket, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0)
    {
        std::cerr << "Failed to join multicast group: " << strerror(errno) << std::endl;
        close(_socket);
        _socket = -1;
        return false;
    }

    std::cout << "HighBandwidthSubscriber joined multicast group " << _multicastAddr 
              << ":" << _port << std::endl;

    _shouldStop.store(false);
    _running.store(true);

    _receiveThread = std::thread(&HighBandwidthSubscriber::receiveLoop, this);

    return true;
}

void HighBandwidthSubscriber::stop()
{
    if (!_running.load())
    {
        return;
    }

    _shouldStop.store(true);
    _running.store(false);

    if (_receiveThread.joinable())
    {
        _receiveThread.join();
    }
}

void HighBandwidthSubscriber::receiveLoop()
{
    std::vector<uint8_t> buffer(65535);  // Max UDP packet size
    auto lastCleanup = std::chrono::steady_clock::now();

    while (_running.load() && !_shouldStop.load())
    {
        // Poll with timeout to allow checking _running flag
        struct pollfd pfd;
        pfd.fd = _socket;
        pfd.events = POLLIN;
        
        int ret = poll(&pfd, 1, 100);  // 100ms timeout
        
        if (ret < 0)
        {
            if (errno != EINTR)
            {
                std::cerr << "poll() failed: " << strerror(errno) << std::endl;
            }
            continue;
        }

        if (ret == 0)
        {
            // Timeout - clean up stale partial messages
            auto now = std::chrono::steady_clock::now();
            if (std::chrono::duration_cast<std::chrono::milliseconds>(now - lastCleanup).count() > 500)
            {
                cleanupStaleMessages();
                lastCleanup = now;
            }
            continue;
        }

        // Receive packet
        ssize_t received = recv(_socket, buffer.data(), buffer.size(), 0);
        if (received < 0)
        {
            if (errno != EINTR && errno != EAGAIN)
            {
                std::cerr << "recv() failed: " << strerror(errno) << std::endl;
            }
            continue;
        }

        if (received >= static_cast<ssize_t>(sizeof(FragmentHeader)))
        {
            processFragment(buffer.data(), static_cast<size_t>(received));
        }
    }
}

void HighBandwidthSubscriber::cleanupStaleMessages()
{
    std::lock_guard<std::mutex> lock(_reassemblyMutex);
    auto now = std::chrono::steady_clock::now();
    
    auto it = _partialMessages.begin();
    while (it != _partialMessages.end())
    {
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            now - it->second.firstFragmentTime).count();
        
        if (elapsed > _reassemblyTimeoutMs)
        {
            // Discard incomplete message
            it = _partialMessages.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void HighBandwidthSubscriber::processFragment(const uint8_t *data, size_t len)
{
    const FragmentHeader *header = reinterpret_cast<const FragmentHeader*>(data);
    const uint8_t *payload = data + sizeof(FragmentHeader);
    size_t payloadLen = len - sizeof(FragmentHeader);

    uint32_t messageId = header->messageId;
    uint16_t fragNum = header->fragmentNum;
    uint16_t totalFrags = header->totalFragments;
    uint16_t topicLen = header->topicLen;

    std::lock_guard<std::mutex> lock(_reassemblyMutex);

    // Get or create partial message entry
    auto &partial = _partialMessages[messageId];
    
    if (partial.fragments.empty())
    {
        // First fragment for this message ID
        partial.totalFragments = totalFrags;
        partial.fragments.resize(totalFrags);
        partial.firstFragmentTime = std::chrono::steady_clock::now();
    }

    // Check consistency
    if (partial.totalFragments != totalFrags)
    {
        // Inconsistent fragment count - discard
        _partialMessages.erase(messageId);
        return;
    }

    // Check if we already have this fragment
    if (partial.receivedFragments.count(fragNum))
    {
        return;  // Duplicate
    }

    // Process fragment content
    if (fragNum == 0)
    {
        // First fragment contains topic
        if (topicLen > payloadLen)
        {
            _partialMessages.erase(messageId);
            return;
        }
        partial.topic = std::string(reinterpret_cast<const char*>(payload), topicLen);
        partial.fragments[0] = std::string(reinterpret_cast<const char*>(payload + topicLen), 
                                            payloadLen - topicLen);
    }
    else
    {
        partial.fragments[fragNum] = std::string(reinterpret_cast<const char*>(payload), payloadLen);
    }

    partial.receivedFragments.insert(fragNum);

    // Check if message is complete
    if (partial.receivedFragments.size() == totalFrags)
    {
        // Reassemble payload
        std::string fullPayload;
        for (const auto &frag : partial.fragments)
        {
            fullPayload += frag;
        }

        std::string topic = partial.topic;
        _partialMessages.erase(messageId);

        // Release lock before calling handler
        _reassemblyMutex.unlock();
        deliverMessage(topic, fullPayload);
        _reassemblyMutex.lock();
    }
}

void HighBandwidthSubscriber::deliverMessage(const std::string &topic, const std::string &payload)
{
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
        handler(topic, payload);
    }
}
