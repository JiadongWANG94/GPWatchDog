/**
 * Wang Jiadong <jiadong.wang.94@outlook.com>
 */

#include <memory>
#include <thread>
#include <atomic>
#include <mutex>
#include "ubus_master.hpp"
#include "ubus_runtime.hpp"

#include "gpwd_message.hpp"

namespace GPWD {

class GPWDServer {
 public:
    bool init(const std::string &ip, uint64_t port);
    void run();

 private:
    void master_thread_worker();
    void registration_callback(const RegistrationMsg& req, ResponseMsg* resp);
    void unregistration_callback(const StringMsg& req, ResponseMsg* resp);
    void feed_callback(const StringMsg& req, NullMsg* resp);
    void list_callback(const NullMsg& req, StringMsg *resp);

 private:
    struct ProcessInfo {
        std::atomic<uint32_t> current_count = 0;
        std::string launch_cmd;
        uint32_t restart_threshold = 0;
        uint32_t start_restrain = 0;
        bool is_initiating = false;
        uint64_t process_id = 0;
    };

 private:
    std::shared_ptr<std::thread> ubus_master_thread_;
    std::shared_ptr<UBusMaster> ubus_master_;
    std::shared_ptr<UBusRuntime> ubus_runtime_;
    std::mutex process_map_mtx_;
    std::map<std::string, std::shared_ptr<ProcessInfo> > process_map_;
};

}  // namespace GPWD
