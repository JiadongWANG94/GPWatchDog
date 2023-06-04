#include <stdint.h>
#include <string>
#include <memory>

namespace GPWD {

class GPWDClient {
 public:
    bool Init(const std::string& server_ip, uint32_t port);
    bool Feed();

 private:
    class Impl;

 private:
    std::unique_ptr<Impl> p_impl_;
};

}  // namespace GPWD