
#include <stdint.h>
#include <iostream>

namespace GPWD {

class Client {
public:
    bool Init(const std::string& server_ip, uint32_t port);
    bool Feed();
};

}  // namespace GPWD