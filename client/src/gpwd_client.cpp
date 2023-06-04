#include "gpwd_client.hpp"

#include <unistd.h>

#include "gpwd_message.hpp"
#include "ubus_runtime.hpp"
#include "log.hpp"

namespace GPWD {

class GPWDClient::Impl {
 public:
    bool Init(const std::string& server_ip, uint32_t port);
    bool Feed();

 private:
    UBusRuntime ubus_runtime_;
    pid_t process_pid_ = 0;
};

bool GPWDClient::Init(const std::string& server_ip, uint32_t port) {
    this->p_impl_ = std::make_unique<Impl>();
    return this->p_impl_->Init(server_ip, port);
}

bool GPWDClient::Feed() {
    if (this->p_impl_ != nullptr) {
        return false;
    }
    return this->p_impl_->Feed();
}

bool GPWDClient::Impl::Init(const std::string& server_ip, uint32_t port) {
    process_pid_ = getpid();
    if (!ubus_runtime_.init("gpwd_client_" + std::to_string(process_pid_),
                            server_ip, port)) {
        LERROR(GPWDClient) << "Failed to initiate UBusRumtime";
        return false;
    } else {
        LDEBUG(GPWDClient) << "Registered to UBus as gpwd_client_" +
                                  std::to_string(process_pid_);
    }
    return true;
}

bool GPWDClient::Impl::Feed() {
    StringMsg req;
    req.data = std::to_string(process_pid_);
    NullMsg resp;
    return ubus_runtime_.call_method<StringMsg, NullMsg>(
        "watchdog/feed_endpoint", req, &resp);
}

}  // namespace GPWD