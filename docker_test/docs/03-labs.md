# 动手实验

除特别说明外，所有命令在 `/home/uisee/cpp/docker_test` 执行，按顺序练习。命令中的容器、卷、网络名称都带 `review`，避免和其他项目混淆。若重复练习遇到同名对象，先用 `docker ps -a` 检查是不是自己之前的练习对象。

## 实验 0：确认环境与程序

```bash
docker version
docker compose version
docker info
```

预期：`docker version` 同时显示 Client 和 Server。只有 Client 或连接失败，先看排障文档。

可选：先在宿主机运行程序，理解 Docker 装的是什么。需要 Python 3.8+，在第一个终端执行：

```bash
DATA_DIR=./examples/counter/local-data PORT=18080 python3 examples/counter/app.py
```

第二个终端访问 `curl -fsS http://127.0.0.1:18080/` 两次，观察 `visits` 累加，然后在第一个终端按 Ctrl+C 停止。这个目录与后续 Docker 卷互不影响。

## 实验 1：构建并启动

```bash
docker build -t docker-review-counter:local examples/counter
docker image ls docker-review-counter
docker run -d --name review-app -p 127.0.0.1:18080:8080 docker-review-counter:local
docker ps
curl -fsS http://127.0.0.1:18080/
curl -fsS http://127.0.0.1:18080/
```

预期：首次两次计数分别是 1、2。若容器刚启动时请求失败，查看日志确认监听后重试。`-t` 设置镜像标签；`-d` 后台运行；`--name` 设置容器名称；`-p` 发布端口。

思考：执行两次 `docker run` 是重启同一个容器吗？同一个名称为什么不能重复使用？

## 实验 2：查看、停止、重启与删除

```bash
docker logs --tail=20 review-app
docker exec review-app sh -c 'pwd; cat /data/visits.txt'
docker inspect review-app --format '{{.State.Status}}'
docker stop review-app
docker ps -a --filter name=review-app
docker start review-app
```

待启动后请求：

```bash
curl -fsS http://127.0.0.1:18080/
docker stop review-app
docker rm review-app
docker run -d --name review-app -p 127.0.0.1:18080:8080 docker-review-counter:local
```

再次请求应回到 1。前一次 `stop/start` 保留可写层；`rm/run` 换了新容器。完成后：

```bash
docker stop review-app
docker rm review-app
```

## 实验 3：构建、配置和源码修改

```bash
docker run -d --name review-app -p 127.0.0.1:18080:8080 -e APP_MESSAGE='I remember Docker' docker-review-counter:local
curl -fsS http://127.0.0.1:18080/
docker history docker-review-counter:local
```

预期问候语改变。这是运行时环境变量覆盖，不需要改镜像。

然后按顺序操作：

1. 修改 `examples/counter/app.py`，在 `/` 的响应字典里加一个字段 `"version": "v2"`。
2. 再次 curl，旧容器仍没有这个字段，因为代码在构建时已复制进镜像。
3. 执行下面的构建命令，观察缓存输出；此时旧容器仍使用旧镜像。

```bash
docker build -t docker-review-counter:local examples/counter
docker stop review-app
docker rm review-app
docker run -d --name review-app -p 127.0.0.1:18080:8080 docker-review-counter:local
```

现在 curl 应能看到 `version`。完成后删除容器，并删除刚加的字段、重新构建，恢复初始示例：

```bash
docker stop review-app
docker rm review-app
docker build -t docker-review-counter:local examples/counter
```

## 实验 4：数据卷与宿主机目录

### 命名卷

```bash
docker volume create review-counter-data
docker run -d --name review-app -p 127.0.0.1:18080:8080 --mount type=volume,src=review-counter-data,dst=/data docker-review-counter:local
curl -fsS http://127.0.0.1:18080/
curl -fsS http://127.0.0.1:18080/
docker stop review-app
docker rm review-app
docker run -d --name review-app -p 127.0.0.1:18080:8080 --mount type=volume,src=review-counter-data,dst=/data docker-review-counter:local
```

再次请求，预期计数继续增加。若是新卷，前两次为 1、2，新容器第一次为 3。

```bash
curl -fsS http://127.0.0.1:18080/
docker inspect review-app --format '{{json .Mounts}}'
docker volume inspect review-counter-data
docker stop review-app
docker rm review-app
```

### bind mount

```bash
mkdir -p /tmp/docker-review-bind
docker run -d --name review-app -p 127.0.0.1:18080:8080 --mount type=bind,src=/tmp/docker-review-bind,dst=/data docker-review-counter:local
curl -fsS http://127.0.0.1:18080/
cat /tmp/docker-review-bind/visits.txt
docker stop review-app
docker rm review-app
```

预期宿主机文件与响应中的数字一致。这里假定使用本机 Docker daemon；远程 Docker 的 bind mount 路径位于远程主机。示例容器默认 root，生成文件可能属于 root。

清理说明：卷保留供检查。确定不再需要练习计数时，执行 `docker volume rm review-counter-data`，这会删除该卷中的数据。bind mount 文件由你在宿主机管理。

## 实验 5：两个容器互相访问

```bash
docker network create review-net
docker run -d --name review-app --network review-net docker-review-counter:local
docker run --rm --network review-net docker-review-counter:local python -c "import urllib.request; print(urllib.request.urlopen('http://review-app:8080/').read().decode())"
docker network inspect review-net
```

预期客户端输出 JSON 后退出并自动删除；app 持续运行。本次没有 `-p`，但同一网络中的客户端仍可访问 `review-app:8080`。

故障练习：把客户端 URL 改为 `http://localhost:8080/`，预期连接失败。解释它为什么访问的是客户端自身。

```bash
docker stop review-app
docker rm review-app
docker network rm review-net
```

## 实验 6：使用 Compose 重复创建环境

先确认之前占用 18080 的练习容器已删除。

```bash
docker compose config
docker compose up -d --build --wait
docker compose ps
curl -fsS http://127.0.0.1:18080/
docker compose run --rm client
docker compose logs --tail=20 app
docker compose exec app cat /data/visits.txt
```

`client` 放在 `tools` profile 中，普通 `up` 只启动 app；显式 `run client` 会运行一次客户端。客户端通过 `app:8080` 访问服务。普通依赖顺序不代表服务就绪，这里用健康检查和 `service_healthy` 表达就绪依赖。

记录计数后执行：

```bash
docker compose down
docker compose up -d --wait
curl -fsS http://127.0.0.1:18080/
```

预期计数在原值上加 1。Compose 卷名默认包含项目名前缀，本项目为 `docker-review_counter-data`，和实验 4 的卷是两个独立对象。

再将 `compose.yaml` 的 `APP_MESSAGE` 改为 `Compose is convenient`，执行 `docker compose up -d --wait`，请求后应看到新问候语。仅执行 `restart` 不会应用修改后的环境变量配置。

完成后恢复问候语，并执行 `docker compose down`。若希望彻底重置本项目计数，可主动执行 `docker compose down -v`；`-v` 会删除本项目声明的非 external 命名卷及相关匿名卷。

## 实验 7：C++ 与独立验收

完成 [C++ 示例说明](../examples/cpp/README.md)，再做 [第 7 天综合任务](01-plan.md)。遇到故障先查状态、日志和配置，最后看 [自测答案](05-self-check.md)。
