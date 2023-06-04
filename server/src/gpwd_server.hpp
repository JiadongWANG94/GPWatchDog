/**
 * Wang Jiadong <jiadong.wang.94@outlook.com>
 */

#include <unistd.h>
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
    bool init(const std::string& ip, uint64_t port);
    void run();

 private:
    void master_thread_worker();
    void registration_callback(const RegistrationMsg& req, ResponseMsg* resp);
    void deregistration_callback(const StringMsg& req, ResponseMsg* resp);
    void feed_callback(const Int64Msg& req, NullMsg* resp);
    void list_callback(const NullMsg& req, StringMsg* resp);

 private:
    struct ProcessInfo {
        std::atomic<uint32_t> current_count = 0;
        std::string executable;
        std::vector<std::string> args;
        std::vector<std::string> envs;
        uint32_t restart_threshold = 0;
        uint32_t start_restrain = 0;
        bool is_initiating = false;
        pid_t process_id = 0;
    };

 private:
    bool launch_process(std::shared_ptr<ProcessInfo> process_info);
    bool kill_process(std::shared_ptr<ProcessInfo> process_info);

 private:
    std::shared_ptr<std::thread> ubus_master_thread_;
    std::shared_ptr<UBusMaster> ubus_master_;
    std::shared_ptr<UBusRuntime> ubus_runtime_;
    std::mutex process_map_mtx_;
    std::map<std::string, std::shared_ptr<ProcessInfo> > process_map_;
};

}  // namespace GPWD
