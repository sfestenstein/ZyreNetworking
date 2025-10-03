#include "zyre_node.h"
#include <iostream>

int main(int argc, char **argv) {
    std::string name = (argc>1)?argv[1]:"publisher";
    std::string topic = (argc>2)?argv[2]:"news";

    Publisher pub(name, topic);
    pub.run();
    return 0;
}
