#include "zyre_node.h"
#include <iostream>

int main(int argc, char **argv) {
    std::string name = (argc>1)?argv[1]:"subscriber";
    std::string topic = (argc>2)?argv[2]:"news";

    Subscriber sub(name, topic);
    sub.run();
    return 0;
}
