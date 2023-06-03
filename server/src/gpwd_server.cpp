/**
 * Wang Jiadong <jiadong.wang.94@outlook.com>
 */

#include "gpwd_server.hpp"

#include "log.hpp"

namespace GPWD {

bool GPWDServer::init(const std::string& ip, uint64_t port) {
    this->ubus_master_ = std::make_shared<UBusMaster>();
    LDEBUG(GPWDServer) << "Initiating UBusMaster";
    if (!this->ubus_master_->init(ip, port)) {
        return false;
    }
    LDEBUG(GPWDServer) << "UBusMaster initiated";
    this->ubus_master_thread_ = std::make_shared<std::thread>([this]() {
                                    this->ubus_master_->run();
                                });
    
    this->ubus_master_thread_->detach();
    LDEBUG(GPWDServer) << "UBusMaster launched";

    LDEBUG(GPWDServer) << "Initiating UBusRuntime";
    this->ubus_runtime_ = std::make_shared<UBusRuntime>();
    if (!this->ubus_runtime_->init("GPWDServer", ip, port)) {
        return false;
    }
    LDEBUG(GPWDServer) << "UBusRuntime initiated";
    return true;
}

void GPWDServer::run() {
    this->ubus_runtime_->provide_method<StringMsg, NullMsg>(
        "watchdog/feed_endpoint", [this](const StringMsg& req, NullMsg* resp) {
            this->feed_callback(req, resp);
        });
    this->ubus_runtime_->provide_method<RegistrationMsg, ResponseMsg>(
        "watchdog/registration",
        [this](const RegistrationMsg& req, ResponseMsg* resp) {
            this->registration_callback(req, resp);
        });
    this->ubus_runtime_->provide_method<StringMsg, ResponseMsg>(
        "watchdog/unregistration",
        [this](const StringMsg& req, ResponseMsg* resp) {
            this->unregistration_callback(req, resp);
        });
    this->ubus_runtime_->provide_method<NullMsg, StringMsg>(
        "watchdog/list", [this](const NullMsg& req, StringMsg* resp) {
            this->list_callback(req, resp);
        });
    while (1) {
        {
            std::lock_guard<std::mutex> lock(this->process_map_mtx_);
            for (auto& process_info : this->process_map_) {
                process_info.second->current_count++;
                if (process_info.second->is_initiating) {
                    if (process_info.second->current_count >=
                        process_info.second->start_restrain) {
                        process_info.second->is_initiating = false;
                        process_info.second->current_count = 0;
                    }
                } else {
                    if (process_info.second->current_count >=
                        process_info.second->restart_threshold) {
                        LINFO(GPWDServer)
                            << "Restarting process " << process_info.first;
                        // TODO: kill current process and restart
                        process_info.second->current_count = 0;
                    }
                }
            }
        }
        sleep(1);
    }
}

void GPWDServer::registration_callback(const RegistrationMsg& req,
                                       ResponseMsg* resp) {
    std::shared_ptr<ProcessInfo> process_info = std::make_shared<ProcessInfo>();
    process_info->current_count = 0;
    process_info->launch_cmd = req.cmd;
    process_info->restart_threshold = req.restart_threshold;
    process_info->start_restrain = req.start_restrain;
    // validate request
    if (process_info->restart_threshold <= 2) {
        LWARN(GPWDServer) << "Threshold too small : " << process_info->restart_threshold;
        resp->ret = 1;
        resp->err_msg = "Threshold too small";
        return;
    }
    std::lock_guard<std::mutex> lock(this->process_map_mtx_);
    if (this->process_map_.find(req.name) != this->process_map_.end()) {
        LWARN(GPWDServer) << "Duplicate process " << req.name;
        resp->ret = 1;
        resp->err_msg =
            "Duplicate process, please unregister before register again";
        return;
    }
    // TODO: run subprocess to launch the target
    this->process_map_[req.name] = process_info;
}

void GPWDServer::unregistration_callback(const StringMsg& req,
                                         ResponseMsg* resp) {
    std::lock_guard<std::mutex> lock(this->process_map_mtx_);
    auto ite = this->process_map_.find(req.data);
    if (ite != this->process_map_.end()) {
        // TODO: kill process
        LINFO(GPWDServer) << "Unregistered process " << req.data;
        this->process_map_.erase(ite);
        resp->ret = 0;
        resp->err_msg = "Success";
        return;
    } else {
        LWARN(GPWDServer) << "No such process " << req.data;
        resp->ret = 1;
        resp->err_msg = "No such process";
        return;
    }
}

void GPWDServer::feed_callback(const StringMsg& req, NullMsg* resp) {
    std::lock_guard<std::mutex> lock(this->process_map_mtx_);
    auto ite = this->process_map_.find(req.data);
    if (ite != this->process_map_.end()) {
        ite->second->current_count = 0;
        return;
    } else {
        LWARN(GPWDServer) << "No such process " << req.data;
        return;
    }
}

void GPWDServer::list_callback(const NullMsg& req, StringMsg* resp) {
    resp->data = "Not implemented";
    return;
}

}  // namespace GPWD
