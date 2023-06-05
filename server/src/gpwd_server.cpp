/**
 * Wang Jiadong <jiadong.wang.94@outlook.com>
 */

#include "gpwd_server.hpp"

#include <signal.h>

#include "log.hpp"

namespace GPWD {

bool GPWDServer::init(const std::string& ip, uint64_t port) {
    this->ubus_master_ = std::make_shared<UBusMaster>();
    LDEBUG(GPWDServer) << "Initiating UBusMaster";
    if (!this->ubus_master_->init(ip, port)) {
        return false;
    }
    LDEBUG(GPWDServer) << "UBusMaster initiated";
    this->ubus_master_thread_ =
        std::make_shared<std::thread>([this]() { this->ubus_master_->run(); });

    this->ubus_master_thread_->detach();
    LDEBUG(GPWDServer) << "UBusMaster launched";
    sleep(1);

    LDEBUG(GPWDServer) << "Initiating UBusRuntime";
    this->ubus_runtime_ = std::make_shared<UBusRuntime>();
    if (!this->ubus_runtime_->init("GPWDServer", ip, port)) {
        return false;
    }
    LDEBUG(GPWDServer) << "UBusRuntime initiated";
    return true;
}

void GPWDServer::run() {
    this->ubus_runtime_->provide_method<Int64Msg, NullMsg>(
        "watchdog/feed_endpoint", [this](const Int64Msg& req, NullMsg* resp) {
            this->feed_callback(req, resp);
        });
    this->ubus_runtime_->provide_method<RegistrationMsg, ResponseMsg>(
        "watchdog/registration",
        [this](const RegistrationMsg& req, ResponseMsg* resp) {
            this->registration_callback(req, resp);
        });
    this->ubus_runtime_->provide_method<StringMsg, ResponseMsg>(
        "watchdog/deregistration",
        [this](const StringMsg& req, ResponseMsg* resp) {
            this->deregistration_callback(req, resp);
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
                        this->kill_process(process_info.second);
                        this->launch_process(process_info.second);
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
    process_info->executable = req.executable;
    process_info->args = req.args;
    process_info->envs = req.envs;
    process_info->restart_threshold = req.restart_threshold;
    process_info->start_restrain = req.start_restrain;
    // validate request
    if (process_info->restart_threshold <= 2) {
        LWARN(GPWDServer) << "Threshold too small : "
                          << process_info->restart_threshold;
        resp->ret = 1;
        resp->err_msg = "Threshold too small";
        return;
    }
    std::lock_guard<std::mutex> lock(this->process_map_mtx_);
    if (this->process_map_.find(req.name) != this->process_map_.end()) {
        LWARN(GPWDServer) << "Duplicate process " << req.name;
        resp->ret = 1;
        resp->err_msg =
            "Duplicate process, please deregister before register again";
        return;
    }
    if (this->launch_process(process_info)) {
        LINFO(GPWDServer) << "Process " << req.name
                          << " registered and launched";
        this->process_map_[req.name] = process_info;
        resp->ret = 0;
        resp->err_msg = "Success";
        return;
    } else {
        LERROR(GPWDServer) << "Failed to launch " << req.name;
        this->process_map_[req.name] = process_info;
        resp->ret = 1;
        resp->err_msg =
            "Failed to launch process, please check your command and arguments";
        return;
    }
}

void GPWDServer::deregistration_callback(const StringMsg& req,
                                         ResponseMsg* resp) {
    std::lock_guard<std::mutex> lock(this->process_map_mtx_);
    auto ite = this->process_map_.find(req.data);
    if (ite != this->process_map_.end()) {
        if (this->kill_process(ite->second)) {
            LINFO(GPWDServer)
                << "Deregistered process " << req.data << ", process of pid "
                << std::to_string(ite->second->process_id) << " killed";
        } else {
            LWARN(GPWDServer)
                << "Failed to kill process " << req.data << " with pid "
                << std::to_string(ite->second->process_id);
        }
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

void GPWDServer::feed_callback(const Int64Msg& req, NullMsg* resp) {
    LDEBUG(GPWDServer) << "Feed callback for pid " << std::to_string(req.data);
    std::lock_guard<std::mutex> lock(this->process_map_mtx_);
    for (auto& process_info : this->process_map_) {
        if (process_info.second->process_id == req.data) {
            process_info.second->current_count = 0;
            return;
        }
    }
    LWARN(GPWDServer) << "Process with pid " << std::to_string(req.data)
                      << " is not registered";
    return;
}

void GPWDServer::list_callback(const NullMsg& req, StringMsg* resp) {
    resp->data = "Not implemented";
    return;
}

bool GPWDServer::launch_process(std::shared_ptr<ProcessInfo> process_info) {
    if (process_info == nullptr) {
        return false;
    }
    pid_t pid = fork();
    if (pid == 0) {
        char* args[process_info->args.size() + 2];
        char* c_arg = new char[process_info->executable.length() + 1];
        strcpy(c_arg, process_info->executable.c_str());
        args[0] = c_arg;
        for (int i = 0; i < process_info->args.size(); ++i) {
            auto& arg = process_info->args[i];
            char* c_arg = new char[arg.length() + 1];
            strcpy(c_arg, arg.c_str());
            args[i + 1] = c_arg;
        }
        args[process_info->args.size() + 1] = nullptr;
        char* envs[process_info->envs.size() + 1];
        for (int i = 0; i < process_info->envs.size(); ++i) {
            auto& env = process_info->envs[i];
            char* c_env = new char[env.length() + 1];
            strcpy(c_env, env.c_str());
            envs[i] = c_env;
        }
        envs[process_info->envs.size()] = nullptr;
        int ret = execve(process_info->executable.c_str(), args, envs);
        if (ret == -1) {
            LERROR(GPWDServer)
                << "Failed to launch process " << process_info->executable;
            return false;
        }
    } else {
        process_info->process_id = pid;
    }
    return true;
}

bool GPWDServer::kill_process(std::shared_ptr<ProcessInfo> process_info) {
    if (process_info == nullptr) {
        return false;
    }
    if (process_info->process_id > 0) {
        return 0 == kill(process_info->process_id, SIGINT);
    }
    return false;
}

}  // namespace GPWD
