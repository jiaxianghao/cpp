#!/bin/bash

# Git 多仓库管理工具集
# 主入口脚本

# 获取脚本所在目录
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# 定义仓库列表（可以根据需要修改）
REPOS=(
    "uos_base"
    "uos_core" 
    "uos_io"
    "uos_nome"
    "uos_operation"
    "uos_utility"
    "uos_emulation"
    "uos_rcslib"
    "uos_release"
    "uos_operation_extension"
)

# 工作目录（使用当前运行命令的路径）
WORK_DIR="$(pwd)"

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# 打印带颜色的消息
print_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# 批量切换分支函数
switch_branch() {
    if [ $# -eq 0 ]; then
        print_error "用法: $0 switch <分支名>"
        print_info "示例: $0 switch release-wuliu-mz-2.6.4hotfix6-RC4-CAG"
        return 1
    fi
    
    local BRANCH_NAME="$1"
    print_info "开始切换到分支: $BRANCH_NAME"
    echo "=================================="
    
    cd "$WORK_DIR" || { print_error "无法进入工作目录: $WORK_DIR"; return 1; }
    
    local success_count=0
    local total_count=0
    
    for repo in "${REPOS[@]}"; do
        if [ -d "$repo" ]; then
            total_count=$((total_count + 1))
            print_info "处理仓库: $repo"
            cd "$repo"
            
            # 检查远程分支是否存在
            if git ls-remote --heads origin "$BRANCH_NAME" | grep -q "$BRANCH_NAME"; then
                # 检查是否已经有本地分支
                if git show-ref --verify --quiet refs/heads/"$BRANCH_NAME"; then
                    print_info "  本地分支已存在，切换到: $BRANCH_NAME"
                    git checkout "$BRANCH_NAME"
                    git pull origin "$BRANCH_NAME"
                else
                    print_info "  创建本地分支并切换到: $BRANCH_NAME"
                    git checkout -b "$BRANCH_NAME" "origin/$BRANCH_NAME"
                fi
                print_success "$repo 切换成功"
                success_count=$((success_count + 1))
            else
                print_warning "$repo 中不存在分支 $BRANCH_NAME"
            fi
            
            cd ..
            echo ""
        else
            print_warning "仓库 $repo 不存在"
        fi
    done
    
    echo "=================================="
    print_success "批量切换完成！成功: $success_count/$total_count"
}

# 检查所有仓库状态函数
check_status() {
    print_info "检查所有仓库状态"
    echo "=================================="
    
    cd "$WORK_DIR" || { print_error "无法进入工作目录: $WORK_DIR"; return 1; }
    
    for repo in "${REPOS[@]}"; do
        if [ -d "$repo" ]; then
            print_info "仓库: $repo"
            cd "$repo"
            
            # 获取当前分支
            local CURRENT_BRANCH=$(git branch --show-current)
            if [ -z "$CURRENT_BRANCH" ]; then
                CURRENT_BRANCH="分离头指针: $(git rev-parse --short HEAD)"
            fi
            
            # 检查是否有未提交的更改
            if git diff --quiet && git diff --cached --quiet; then
                local STATUS="✅ 干净"
            else
                local STATUS="⚠️  有未提交更改"
            fi
            
            echo "  分支: $CURRENT_BRANCH"
            echo "  状态: $STATUS"
            echo ""
            
            cd ..
        else
            print_warning "仓库 $repo 不存在"
        fi
    done
    
    echo "=================================="
}

# 批量拉取最新代码函数
pull_all() {
    print_info "批量拉取最新代码"
    echo "=================================="
    
    cd "$WORK_DIR" || { print_error "无法进入工作目录: $WORK_DIR"; return 1; }
    
    local success_count=0
    local total_count=0
    
    for repo in "${REPOS[@]}"; do
        if [ -d "$repo" ]; then
            total_count=$((total_count + 1))
            print_info "处理仓库: $repo"
            cd "$repo"
            
            local CURRENT_BRANCH=$(git branch --show-current)
            if [ -n "$CURRENT_BRANCH" ]; then
                print_info "  拉取所有分支"
                # 拉取所有远程分支
                git fetch --all
                # 如果当前分支有对应的远程分支，则更新当前分支
                if git show-ref --verify --quiet refs/remotes/origin/"$CURRENT_BRANCH"; then
                    print_info "  更新当前分支: $CURRENT_BRANCH"
                    git pull origin "$CURRENT_BRANCH"
                else
                    print_info "  当前分支 $CURRENT_BRANCH 没有对应的远程分支"
                fi
                print_success "$repo 所有分支拉取完成"
                success_count=$((success_count + 1))
            else
                print_warning "$repo 处于分离头指针状态，跳过"
            fi
            
            cd ..
            echo ""
        else
            print_warning "仓库 $repo 不存在"
        fi
    done
    
    echo "=================================="
    print_success "批量拉取完成！成功: $success_count/$total_count"
}

# 显示帮助信息
show_help() {
    echo "Git 多仓库管理工具"
    echo "=================="
    echo ""
    echo "用法: $0 <命令> [参数]"
    echo ""
    echo "命令:"
    echo "  switch <分支名>     - 批量切换到指定分支"
    echo "  status             - 检查所有仓库状态"
    echo "  pull               - 批量拉取最新代码"
    echo "  help               - 显示此帮助信息"
    echo ""
    echo "示例:"
    echo "  $0 switch release-wuliu-mz-2.6.4hotfix6-RC4-CAG"
    echo "  $0 status"
    echo "  $0 pull"
    echo ""
    echo "配置:"
    echo "  工作目录: $WORK_DIR"
    echo "  仓库列表: ${REPOS[*]}"
}

# 主函数
main() {
    case "$1" in
        "switch")
            shift
            switch_branch "$@"
            ;;
        "status")
            check_status
            ;;
        "pull")
            pull_all
            ;;
        "help"|"--help"|"-h"|"")
            show_help
            ;;
        *)
            print_error "未知命令: $1"
            show_help
            exit 1
            ;;
    esac
}

# 执行主函数
main "$@"



