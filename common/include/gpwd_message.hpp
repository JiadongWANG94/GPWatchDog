#include <vector>
#include <string>

#include "nlohmann/json.hpp"

#include "message.hpp"

namespace GPWD {

class RegistrationMsg {
 public:
    static const uint32_t id;

 public:
    virtual void serialize(std::string *data) const {
        nlohmann::json json_struct = {
            {"name", this->name},
            {"restart_threshold", this->restart_threshold},
            {"start_restrain", this->start_restrain},
            {"executable", this->executable},
            {"args", this->args},
            {"envs", this->envs},
        };
        *data = json_struct.dump();
    }
    virtual void deserialize(const std::string &data) {
        if (!nlohmann::json::accept(data)) {
            return;
        }
        nlohmann::json json_struct = nlohmann::json::parse(data);
        if (json_struct.contains("name") &&
            json_struct.at("name").is_string()) {
            this->name = json_struct.at("name").get<std::string>();
        }
        if (json_struct.contains("executable") &&
            json_struct.at("executable").is_string()) {
            this->executable = json_struct.at("executable").get<std::string>();
        }
        if (json_struct.contains("restart_threshold") &&
            json_struct.at("restart_threshold").is_number_unsigned()) {
            this->restart_threshold =
                json_struct.at("restart_threshold").get<uint32_t>();
        }
        if (json_struct.contains("start_restrain") &&
            json_struct.at("start_restrain").is_number_unsigned()) {
            this->start_restrain =
                json_struct.at("start_restrain").get<uint32_t>();
        }
        this->args.clear();
        if (json_struct.contains("args") && json_struct.at("args").is_array()) {
            for (auto &arg : json_struct.at("args")) {
                if (arg.is_string()) {
                    this->args.push_back(arg.get<std::string>());
                }
            }
        }
        this->envs.clear();
        if (json_struct.contains("envs") && json_struct.at("envs").is_array()) {
            for (auto &env : json_struct.at("envs")) {
                if (env.is_string()) {
                    this->args.push_back(env.get<std::string>());
                }
            }
        }
    }

 public:
    std::string name;
    std::string executable;
    std::vector<std::string> args;
    std::vector<std::string> envs;
    uint32_t restart_threshold = 0;
    uint32_t start_restrain = 0;
};

class ResponseMsg {
 public:
    static const uint32_t id;

 public:
    virtual void serialize(std::string *data) const {
        nlohmann::json json_struct = {
            {"ret", this->ret},
            {"err_msg", this->err_msg},
        };
        *data = json_struct.dump();
    }
    virtual void deserialize(const std::string &data) {
        if (!nlohmann::json::accept(data)) {
            return;
        }
        nlohmann::json json_struct = nlohmann::json::parse(data);
        if (json_struct.contains("ret") &&
            json_struct.at("ret").is_number_unsigned()) {
            this->ret = json_struct.at("ret").get<uint32_t>();
        }
        if (json_struct.contains("err_msg") &&
            json_struct.at("err_msg").is_string()) {
            this->err_msg = json_struct.at("err_msg").get<std::string>();
        }
    }

 public:
    uint32_t ret = 0;
    std::string err_msg;
};

}  // namespace GPWD