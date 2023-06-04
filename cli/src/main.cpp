/**
 * Wang Jiadong <jiadong.wang.94@outlook.com>
 */

#include "CLI11.hpp"
#include "nlohmann/json.hpp"

#include "gpwd_message.hpp"
#include "ubus_runtime.hpp"
#include "log.hpp"

using namespace GPWD;

int main(int argc, char **argv) {
    g_log_manager.SetLogLevel(4);
    CLI::App app{"gpwd_cli: command line interface for managing gpwd"};
    std::string master_ip = "127.0.0.1";
    app.add_option("--master_ip", master_ip,
                   "ip of ubus master, default: 127.0.0.1");
    uint32_t master_port = 8101;
    app.add_option("--master_port", master_port,
                   "port of ubus master, default: 8101");
    app.require_subcommand();
    CLI::App *subcom_register =
        app.add_subcommand("register", "register new process");
    std::string arg_name = "";
    subcom_register->add_option("--name", arg_name, "name of new process")
        ->required();
    std::string arg_executable = "";
    subcom_register
        ->add_option("--executable", arg_executable,
                     "executable of new process")
        ->required();
    std::string arg_args;
    subcom_register->add_option(
        "--args", arg_args,
        "args of new process, example '[\\'-v\\', \\'ABC\\']'");
    std::string arg_envs;
    subcom_register->add_option("--envs", arg_envs,
                                "envs of new process example "
                                "'[\\'CONFIG=config.json\\', \\'DEBUG=1\\']'");
    uint32_t arg_restart_threshold = 0;
    subcom_register
        ->add_option("--restart_threshold", arg_restart_threshold,
                     "restart threshold of new process")
        ->required();
    uint32_t arg_start_restrain = 0;
    subcom_register
        ->add_option("--start_restrain", arg_start_restrain,
                     "start_restrain of new process")
        ->required();

    CLI::App *subcom_deregister =
        app.add_subcommand("deregister", "deregister existing process");
    subcom_deregister
        ->add_option("--name", arg_name, "name of process to deregister")
        ->required();
    try {
        app.parse(argc, argv);
    } catch (const CLI::ParseError &e) {
        return app.exit(e);
    }

    if (subcom_register->parsed()) {
        RegistrationMsg msg;
        msg.name = arg_name;
        msg.executable = arg_executable;
        msg.restart_threshold = arg_restart_threshold;
        msg.start_restrain = arg_start_restrain;
        if (arg_args != "") {
            if (!nlohmann::json::accept(arg_args)) {
                std::cout << "Invalid args" << std::endl;
                return 1;
            }
            nlohmann::json json_struct = nlohmann::json::parse(arg_args);
            if (!json_struct.is_array()) {
                std::cout << "Invalid args" << std::endl;
                return 1;
            }
            for (auto &arg : json_struct) {
                if (!arg.is_string()) {
                    std::cout << "Invalid args" << std::endl;
                    return 1;
                }
                msg.args.push_back(arg.get<std::string>());
            }
        }
        if (arg_envs != "") {
            if (!nlohmann::json::accept(arg_envs)) {
                std::cout << "Invalid envs" << std::endl;
                return 1;
            }
            nlohmann::json json_struct = nlohmann::json::parse(arg_envs);
            if (!json_struct.is_array()) {
                std::cout << "Invalid envs" << std::endl;
                return 1;
            }
            for (auto &env : json_struct) {
                if (!env.is_string()) {
                    std::cout << "Invalid envs" << std::endl;
                    return 1;
                }
                msg.envs.push_back(env.get<std::string>());
            }
        }
        UBusRuntime runtime;
        runtime.init("gpwd_cli", master_ip, master_port);
        ResponseMsg response;
        runtime.call_method<RegistrationMsg, ResponseMsg>(
            "watchdog/registration", msg, &response);
        std::cout << response.err_msg << std::endl;
    }

    if (subcom_deregister->parsed()) {
        StringMsg msg;
        msg.data = arg_name;
        UBusRuntime runtime;
        runtime.init("gpwd_cli", master_ip, master_port);
        ResponseMsg response;
        runtime.call_method<StringMsg, ResponseMsg>("watchdog/deregistration",
                                                    msg, &response);
        std::cout << response.err_msg << std::endl;
    }

    return 0;
}