#!/bin/bash

script_path=$(dirname $0)

find $script_path -path "u-bus" -a -prune -name "*.cpp" -exec clang-format -style=file -i {} \;
find $script_path -path "u-bus" -a -prune -name "*.hpp" -exec clang-format -style=file -i {} \;
