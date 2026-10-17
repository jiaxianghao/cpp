# 核心概念：从“程序怎么运行”开始

## 1. 镜像、容器和仓库

镜像是运行应用所需文件及配置的只读模板。容器是基于镜像创建的运行实例，有独立的可写层；一个镜像可以创建多个容器。仓库用于存储和分发镜像。[官方概览](https://docs.docker.com/get-started/docker-overview/)

```text
源代码 + Dockerfile ──build──> 镜像 ──run──> 容器
                              │            │
                             push         stop / start
                              ↓            │
                           镜像仓库        rm
                              │
                             pull
                              ↓
                           本地镜像
```

`docker-review-counter:local` 中，冒号前是镜像名称，后面是标签。标签可以被重新指向其他镜像，并不等于不可变版本。

## 2. 容器围绕主进程运行

Linux 容器的进程通常共享宿主机 Linux 内核，通过命名空间等机制获得隔离视图，通过 cgroups 管理资源。Docker Desktop 的 Linux 容器通常运行在它管理的 Linux 虚拟机中。

容器主进程退出，容器就停止。`-d` 表示后台运行，不会让已经结束的程序继续存活。CLI 程序打印一行就退出是正常现象。

- `docker run`：创建新容器并启动。
- `docker start`：启动已经存在的停止容器。
- `docker exec`：在运行中的容器内额外启动一个进程。
- `docker stop`：要求主进程停止，超时后强制终止；容器仍存在。
- `docker rm`：删除容器及其可写层。

## 3. Dockerfile 描述构建，Compose 描述运行组合

| 指令 | 本示例中的用途 | 常见误解 |
| --- | --- | --- |
| FROM | 选择包含 Python 的基础镜像 | 基础镜像不是完整虚拟机 |
| WORKDIR | 后续指令和默认运行目录为 /app | 不改变宿主机目录 |
| COPY | 将构建上下文中的文件加入镜像 | 不会自动跟踪宿主机文件变化 |
| RUN | 构建阶段执行命令，如编译程序 | 不是每次启动容器都执行 |
| ENV | 设置容器默认环境变量 | 可通过运行时配置覆盖 |
| EXPOSE | 描述应用预期使用的端口 | 不会发布宿主机端口 |
| CMD | 设置默认启动命令或参数 | 可被 docker run 后面的命令覆盖 |
| ENTRYPOINT | 设置入口程序 | 本例 C++ 程序通过 CMD 获得默认参数 |

构建命令最后的 `.` 是构建上下文。`COPY` 的源路径以它为起点；`.dockerignore` 决定哪些文件不发送进上下文。构建可复用缓存，某一步的输入变化通常会影响它以及后续步骤。

Compose 的 `services`、`ports`、`environment`、`volumes` 把运行参数写成 YAML，方便重复创建环境。[Compose 概念](https://docs.docker.com/get-started/docker-concepts/the-basics/what-is-docker-compose/)

## 4. 三种数据位置

| 数据位置 | 停止再启动 | 删除并新建容器 | 本练习用途 |
| --- | --- | --- | --- |
| 容器可写层 | 仍在 | 原可写层被删除 | 临时文件、第一组计数实验 |
| 命名卷 | 仍在 | 重新挂同一卷后仍在 | 保存访问计数 |
| bind mount 宿主机目录 | 仍在 | 再挂同一目录后仍在 | 直接查看文件、开发时挂源码 |

命名卷由 Docker 管理，生命周期独立于容器。挂载会遮住目标目录原有内容；本示例挂 `/data`，不覆盖程序目录。数据卷不是备份，应用仍可能修改或删除其中的数据。[数据卷说明](https://docs.docker.com/engine/storage/volumes/)

## 5. 端口和网络

```text
宿主机 curl 127.0.0.1:18080
                 │ 端口发布
                 ▼
             app:8080  ←── 同一 Compose 网络中的 client
```

`-p 127.0.0.1:18080:8080` 把宿主机回环地址的 18080 映射到容器的 8080。同一自定义网络中的容器可使用名称与容器端口通信，不需要绕到宿主机发布端口。

容器中的 `localhost` 指向它自己。在 client 里访问 `localhost:8080` 不会访问 app。app 在容器内监听 `0.0.0.0`，才能接收来自容器网络接口的连接。[网络说明](https://docs.docker.com/engine/network/)

## 6. 本程序刻意保持简单的地方

计数服务使用 Python 标准库、单进程顺序处理请求，将数字写入文件。它用于观察容器行为，没有数据库事务或多副本并发写入机制，不适合作为生产服务。Compose 健康检查调用 `/health`，所以不会偷偷增加计数。

C++ 示例在一个阶段编译，只把产物复制到最终镜像，减少运行镜像携带的工具。[多阶段构建说明](https://docs.docker.com/build/building/multi-stage/)
