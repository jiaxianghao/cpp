#!/bin/bash

# Git 多仓库管理工具别名自动加载脚本
# 这个脚本会自动加载别名到当前shell中

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
GIT_MULTI="$SCRIPT_DIR/git_multi.sh"

# 定义别名
alias gswitch="$GIT_MULTI switch"
alias gstatus="$GIT_MULTI status"
alias gpull="$GIT_MULTI pull"
alias ghelp="$GIT_MULTI help"
alias gmulti="$GIT_MULTI"