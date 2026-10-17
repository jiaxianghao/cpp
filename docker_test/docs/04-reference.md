# 命令速查与排障

命令里的 `review-app` 是单容器实验名称。Compose 实验用 `docker compose ... app` 操作。

## 常用命令

| 目的 | 命令 |
| --- | --- |
| 环境信息 | `docker version`、`docker info`、`docker compose version` |
| 查看镜像 | `docker image ls` |
| 构建镜像 | `docker build -t docker-review-counter:local examples/counter` |
| 查看运行 / 所有容器 | `docker ps` / `docker ps -a` |
| 跟踪日志，Ctrl+C 退出查看 | `docker logs -f --tail=50 review-app` |
| 进入运行中的容器 | `docker exec -it review-app sh` |
| 状态与退出码 | `docker inspect review-app --format '{{.State.Status}} {{.State.ExitCode}} {{.State.Error}}'` |
| 挂载信息 | `docker inspect review-app --format '{{json .Mounts}}'` |
| 端口映射 | `docker port review-app` |
| 资源占用快照 | `docker stats --no-stream` |
| 停止 / 再启动 | `docker stop review-app` / `docker start review-app` |
| 删除已停止容器 | `docker rm review-app` |
| 查看卷和网络 | `docker volume ls`、`docker network ls` |
| 展开并检查 Compose | `docker compose config` |
| 构建并启动，等待健康 | `docker compose up -d --build --wait` |
| 查看 Compose 状态与日志 | `docker compose ps -a`、`docker compose logs --tail=50 app` |
| 重建 app 容器 | `docker compose up -d --force-recreate --wait app` |
| 删除项目容器和网络，保留命名卷 | `docker compose down` |

## 排障顺序

先确认命令在哪个目录执行，然后依次检查：容器是否存在 → 是否运行 → 日志 → 端口/网络 → 挂载/环境变量。

| 现象 | 检查和处理 |
| --- | --- |
| Cannot connect to the Docker daemon | 用 `docker context show` 确认目标，用 `docker info` 检查 daemon；本机 Linux 可查 `systemctl status docker` |
| docker.sock permission denied | 检查执行用户的 Docker 权限与当前会话；不要直接把 socket 改成全员可写 |
| 拉取镜像超时 | 检查镜像仓库连接、DNS、代理；区分网络失败与 Dockerfile 语法错误 |
| name is already in use | 用 `docker ps -a --filter name=review-app` 查看旧容器，启动旧容器或停止并删除自己的练习容器 |
| port is already allocated | 用 `docker ps` 和 `ss -ltn` 查占用，或将宿主机端口换为 18081，同时修改 curl URL |
| 容器刚创建就退出 | 查看 `docker ps -a` 和日志；程序执行完成也会正常退出 |
| curl connection refused | 检查容器状态、日志是否开始监听、发布端口是否正确；刚启动时稍后重试 |
| 从其他容器访问 localhost 失败 | localhost 指当前容器；使用同一自定义网络中的容器名 / Compose 服务名和容器端口 |
| exec 提示容器未运行 | 先查退出原因；exec 只适用于运行中的容器 |
| 容器里找不到 bash/curl | 精简镜像不一定包含这些工具；本例用 sh 和 Python urllib |
| 修改代码却没有变化 | 代码 COPY 进镜像后需 build，再创建新容器；restart 仍是原容器 |
| 修改 Compose 环境变量无效 | 运行 `docker compose up -d` 应用变更，不能只 restart |
| 重建后计数归零 | inspect 挂载，确认仍挂在 /data 且卷名相同；检查是否执行了 down -v |

## 清理范围

优先删除明确命名的练习对象。`docker compose down` 适合结束本项目练习。`docker compose down -v` 会删除练习数据，只有主动重置时使用。

不需要为本课程执行全局 `docker system prune`；它可能影响其他项目的未使用资源。删除容器不会删除对应镜像，删除镜像也不是清理命名卷的方法。
