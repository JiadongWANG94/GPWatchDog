#include "gpwd_client.hpp"

#include <unistd.h>
#include <iostream>

using namespace GPWD;

int main() {
    std::cout << "my_app launched" << std::endl;
    GPWDClient client;
    client.init("127.0.0.1", 8101);
    for (int i = 0; i < 20; ++i) {
        std::cout << "Feed dog in loop " << i << std::endl;
        client.feed();
        sleep(1);
    }
    std::cout << "Process exit" << std::endl;
    return 0;
}