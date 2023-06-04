#!/bin/bash

script_path=$(dirname $0)

find $script_path -path "${script_path}/u-bus" -a -prune -o -name "*.cpp" -exec clang-format -style=file -i {} \;
find $script_path -path "${script_path}/u-bus" -a -prune -o -name "*.hpp" -exec clang-format -style=file -i {} \;
