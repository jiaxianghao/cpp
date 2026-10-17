# C++：编译环境和运行环境

本程序读取可选环境变量 `GREETING` 和第一个命令行参数，打印问候语后退出。没有服务端口或数据卷，方便集中学习构建。

## 宿主机运行

在项目根目录执行，需要 g++：

```bash
g++ -std=c++17 -O2 -Wall -Wextra examples/cpp/main.cpp -o /tmp/docker-review-hello
/tmp/docker-review-hello
GREETING=你好 /tmp/docker-review-hello 同学
```

预期输出分别为 `Hello, Docker!` 和 `你好, 同学!`。

## Docker 多阶段构建

在项目根目录执行；首次下载 gcc 镜像可能比较大：

```bash
docker build -t docker-review-cpp:local examples/cpp
docker run --rm docker-review-cpp:local
docker run --rm -e GREETING=你好 docker-review-cpp:local 同学
docker image ls docker-review-cpp
```

预期输出与宿主机一致。`--rm` 在程序退出后自动删除容器。

逐行阅读 Dockerfile：

1. `FROM gcc:14-bookworm AS builder`：带编译器的构建阶段。
2. `RUN g++ ... -static ...`：构建时生成静态链接的小程序。
3. 第二条 `FROM`：开始一个新的运行阶段。
4. `COPY --from=builder`：只把可执行文件复制进最终镜像。
5. `USER 10001:10001`：该只读小程序不需要 root 权限。
6. `ENTRYPOINT` 固定运行 hello；`CMD` 提供默认参数 Docker。

本例使用静态链接以减少运行依赖干扰；实际 C++ 工程若使用动态库，需要把兼容的运行库放进最终镜像。不要直接假定在任意发行版编译的程序都能在任意基础镜像中运行。

## 验证最终镜像

```bash
docker run --rm --entrypoint sh docker-review-cpp:local -c 'id; command -v g++ || echo "运行镜像没有 g++"'
docker run --rm --entrypoint /usr/local/bin/hello docker-review-cpp:local World
```

预期：UID 为 10001，没有 g++，第二条输出 `Hello, World!`。这里的 `--entrypoint` 显式覆盖了镜像的入口程序。

练习：在 main.cpp 增加一行版本输出，重新构建并运行；观察仅更改默认参数与更改源代码所需操作的区别。

原理参考：[Docker 官方多阶段构建说明](https://docs.docker.com/build/building/multi-stage/)。
