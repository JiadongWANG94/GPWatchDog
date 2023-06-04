#include "gpwd_client.hpp"

#include <unistd.h>

#include "gpwd_message.hpp"
#include "ubus_runtime.hpp"
#include "log.hpp"

namespace GPWD {

class GPWDClient::Impl {
 public:
    bool init(const std::string& server_ip, uint32_t port);
    bool feed();

 private:
    UBusRuntime ubus_runtime_;
    pid_t process_pid_ = 0;
};

GPWDClient::~GPWDClient() {}

bool GPWDClient::init(const std::string& server_ip, uint32_t port) {
    this->p_impl_ = std::make_shared<Impl>();
    return this->p_impl_->init(server_ip, port);
}

bool GPWDClient::feed() {
    if (this->p_impl_ == nullptr) {
        return false;
    }
    return this->p_impl_->feed();
}

bool GPWDClient::Impl::init(const std::string& server_ip, uint32_t port) {
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

bool GPWDClient::Impl::feed() {
    Int64Msg req;
    req.data = process_pid_;
    NullMsg resp;
    if (!ubus_runtime_.call_method<Int64Msg, NullMsg>("watchdog/feed_endpoint",
                                                      req, &resp)) {
        LERROR(GPWDClient) << "Failed to feed dog";
        return false;
    }
    return true;
}

}  // namespace GPWD