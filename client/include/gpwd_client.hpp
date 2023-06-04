#include <stdint.h>
#include <string>
#include <memory>

namespace GPWD {

class GPWDClient {
 public:
    ~GPWDClient();
    bool init(const std::string& server_ip, uint32_t port);
    bool feed();

 private:
    class Impl;

 private:
    std::shared_ptr<Impl> p_impl_ = nullptr;
};

}  // namespace GPWD