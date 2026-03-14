#!/bin/bash
# 简化版 ARM8155 编译脚本

if [[ $# -eq 0 ]]; then
    echo "用法: $0 <工程目录名>"
    echo "示例: $0 2.x"
    exit 1
fi

PROJECT_NAME="$1"
WORK_DIR="/data/work"
BUILD_DIR="$WORK_DIR/$PROJECT_NAME/src"

echo "工程目录: $PROJECT_NAME"
echo "构建目录: $BUILD_DIR"

# 检查目录
if [[ ! -d "$BUILD_DIR" ]]; then
    echo "错误: 构建目录不存在: $BUILD_DIR" >&2
    exit 1
fi

# 执行编译
docker run --rm \
    -v "$WORK_DIR:/workspace" \
    -w "/workspace/$PROJECT_NAME" \
    -u $(id -u):$(id -g) \
    reg.uisee.ai/devops/8155_uos:1.9 \
    bash -c "
        git config --global --add safe.directory /workspace
        git config --global --add safe.directory /workspace/$PROJECT_NAME
        export PATH=\$PATH:/opt/gcc-arm-none-eabi-7-2017-q4-major/bin
        export ARM8155_SYSROOT=/yocto-sysroot
        catkin_make -DARM8155=1 -DARM8155_SYSROOT=\${ARM8155_SYSROOT} -j8 install
    "

echo "编译完成！"