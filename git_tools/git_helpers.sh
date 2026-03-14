#!/bin/bash

# Git 多仓库管理助手函数

# 定义仓库列表
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

# 批量切换分支函数
gswitch() {
    if [ $# -eq 0 ]; then
        echo "用法: gswitch <分支名>"
        echo "示例: gswitch release-wuliu-mz-2.6.4hotfix6-RC4-CAG"
        return 1
    fi
    
    local BRANCH_NAME="$1"
    echo "开始切换到分支: $BRANCH_NAME"
    echo "=================================="
    
    for repo in "${REPOS[@]}"; do
        if [ -d "$repo" ]; then
            echo "处理仓库: $repo"
            cd "$repo"
            
            # 检查远程分支是否存在
            if git ls-remote --heads origin "$BRANCH_NAME" | grep -q "$BRANCH_NAME"; then
                # 检查是否已经有本地分支
                if git show-ref --verify --quiet refs/heads/"$BRANCH_NAME"; then
                    echo "  本地分支已存在，切换到: $BRANCH_NAME"
                    git checkout "$BRANCH_NAME"
                    git pull origin "$BRANCH_NAME"
                else
                    echo "  创建本地分支并切换到: $BRANCH_NAME"
                    git checkout -b "$BRANCH_NAME" "origin/$BRANCH_NAME"
                fi
                echo "  ✅ $repo 切换成功"
            else
                echo "  ❌ $repo 中不存在分支 $BRANCH_NAME"
            fi
            
            cd ..
            echo ""
        else
            echo "❌ 仓库 $repo 不存在"
        fi
    done
    
    echo "=================================="
    echo "批量切换完成！"
}

# 检查所有仓库状态函数
gstatus() {
    echo "检查所有仓库状态"
    echo "=================================="
    
    for repo in "${REPOS[@]}"; do
        if [ -d "$repo" ]; then
            echo "仓库: $repo"
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
            echo "❌ 仓库 $repo 不存在"
        fi
    done
    
    echo "=================================="
}

# 批量拉取最新代码函数
gpull() {
    echo "批量拉取最新代码"
    echo "=================================="
    
    for repo in "${REPOS[@]}"; do
        if [ -d "$repo" ]; then
            echo "处理仓库: $repo"
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
            echo "❌ 仓库 $repo 不存在"
        fi
    done
    
    echo "=================================="
    echo "批量拉取完成！"
}

# 显示帮助信息
ghelp() {
    echo "Git 多仓库管理助手"
    echo "=================="
    echo "gswitch <分支名>  - 批量切换到指定分支"
    echo "gstatus          - 检查所有仓库状态"
    echo "gpull            - 批量拉取最新代码"
    echo "ghelp            - 显示此帮助信息"
    echo ""
    echo "使用方法："
    echo "  source git_helpers.sh  # 加载函数"
    echo "  gswitch release-wuliu-mz-2.6.4hotfix6-RC4-CAG"
}
