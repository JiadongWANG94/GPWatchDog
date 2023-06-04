# GPWatchDog
General Purpose Watch Dog

## Components
* GPWDServer  
Server that runs in background and monitor



* GPWDClient  



* gpwd_cli  
Command-line interface for managing compnents that GPWD should monitor.

```shell
gpwd_cli register --name "my_app1" --launch_cmd "/usr/bin/my_app1" --restart_threshold 15 --start_restrain 10

gpwd_cli list

gpwd_clit deregister --name "my_app1"
```