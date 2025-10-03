Federated Pub-Sub Demo using Zyre and ZeroMQ

Overview

This small demo shows a publisher and subscriber implemented in C++ using Zyre (which uses CZMQ/ZeroMQ).
Zyre provides peer discovery so there's no central broker/proxy: subscribers join a named "topic" group and publishers "shout" to that group.

Build

This project expects system libraries for czmq/zyre/libzmq to be installed and pkg-config to find them.

On Debian/Ubuntu you can try:

sudo apt-get install -y libczmq-dev libzyre-dev libzmq3-dev pkg-config cmake build-essential

Build steps:

mkdir build && cd build
cmake ..
make -j

Run

Start a subscriber (listens for shouts on topic "news"):

./subscriber sub1 news

Start a publisher (shouts on topic "news"):

./publisher pub1 news

You should see the subscriber automatically receive messages from the publisher.

Notes

- This is a minimal demo for learning. Production code should add proper error handling, signal handling, and configuration.
- Zyre uses UDP multicast for discovery; ensure your network permits that or run both processes on the same host.

*** End Patch