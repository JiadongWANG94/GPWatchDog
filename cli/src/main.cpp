/**
 * Wang Jiadong <jiadong.wang.94@outlook.com>
 */

#include "CLI11.hpp"

#include "gpwd_message.hpp"
#include "ubus_runtime.hpp"
#include "log.hpp"

using namespace GPWD;

int main(int argc, char **argv) {
    g_log_manager.SetLogLevel(4);
    CLI::App app{"gpwd_cli: command line interface for managing gpwd"};
    std::string master_ip = "127.0.0.1";
    app.add_option("--master_ip", master_ip, "ip of ubus master, default: 127.0.0.1");
    uint32_t master_port = 8101;
    app.add_option("--master_port", master_port, "port of ubus master, default: 8101");
    app.require_subcommand();
    CLI::App *subcom_register = app.add_subcommand("register", "register new process");
    std::string arg_name = "";
    subcom_register->add_option("--name", arg_name, "name of new process")->required();
    std::string arg_launch_cmd = "";
    subcom_register->add_option("--launch_cmd", arg_launch_cmd, "launch cmd of new process")->required();
    uint32_t arg_restart_threshold = 0;
    subcom_register->add_option("--restart_threshold", arg_restart_threshold, "restart threshold of new process")->required();
    uint32_t arg_start_restrain = 0;
    subcom_register->add_option("--start_restrain", arg_start_restrain, "start_restrain of new process")->required();

    CLI::App *subcom_unregister = app.add_subcommand("unregister", "unregister existing process");
    subcom_unregister->add_option("--name", arg_name, "name of process to unregister")->required();
    try {
        app.parse(argc, argv);
    } catch (const CLI::ParseError &e) {
        return app.exit(e);
    }

    if (subcom_register->parsed()) {
        RegistrationMsg msg;
        msg.name = arg_name;
        msg.cmd = arg_launch_cmd;
        msg.restart_threshold = arg_restart_threshold;
        msg.start_restrain = arg_start_restrain;
        UBusRuntime runtime;
        runtime.init("gpwd_cli", master_ip, master_port);
        ResponseMsg response;
        runtime.call_method<RegistrationMsg, ResponseMsg>("watchdog/registration", msg, &response);
        std::cout << response.err_msg << std::endl;
    }

    if (subcom_unregister->parsed()) {
        StringMsg msg;
        msg.data = arg_name;
        UBusRuntime runtime;
        runtime.init("gpwd_cli", master_ip, master_port);
        ResponseMsg response;
        runtime.call_method<StringMsg, ResponseMsg>("watchdog/unregistration", msg, &response);
        std::cout << response.err_msg << std::endl;
    }

    return 0;
}