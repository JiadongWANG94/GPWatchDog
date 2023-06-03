#include "gpwd_server.hpp"

#include "test.hpp"

using namespace GPWD;

int main() {
    InitFailureHandle();
    GPWDServer server;
    server.init("127.0.0.1", 8101);
    server.run();
    return 0;
}