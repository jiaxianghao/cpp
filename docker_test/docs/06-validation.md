# 验证记录

验证日期：2026-10-16。

| 项目 | 结果 |
| --- | --- |
| Docker CLI / daemon | 已检查，27.3.1 |
| Compose | 已检查，v2.29.7 |
| Compose 配置 | `docker compose config --quiet` 通过 |
| HTTP 示例 | 宿主机 Python 3.8.10 实测通过：问候语配置、计数增加、健康检查不增加计数、404、重启后继续计数 |
| C++ 示例 | 宿主机 g++ 使用 C++17、警告选项和静态链接编译成功，默认参数与中文环境变量输出符合预期 |
| Markdown 本地链接 | 已检查，目标文件存在 |
| Docker 镜像构建及容器实验 | 尚未完成：拉取 `python:3.12-slim-bookworm` 时 Docker Hub 返回 connection reset by peer |
| C++ 多阶段镜像 | 尚未完成容器实测；gcc 与 debian 基础镜像未下载 |

本机已有其他项目的镜像，但未用它们替代教学基础镜像，也未修改 Docker daemon 配置。未创建持续运行的教学容器。

## 不依赖 Docker 的程序自检

在项目根目录执行：

```bash
python3 examples/counter/smoke_test.py
g++ -std=c++17 -O2 -Wall -Wextra -static examples/cpp/main.cpp -o /tmp/docker-review-hello
/tmp/docker-review-hello
GREETING=你好 /tmp/docker-review-hello 同学
```

HTTP 自检使用临时目录与随机端口，结束后关闭服务并删除临时计数文件，不改变练习数据。

## 镜像仓库连接恢复后的验证

```bash
docker compose up -d --build --wait
curl -fsS http://127.0.0.1:18080/
docker compose run --rm client
docker compose down
docker compose up -d --wait
curl -fsS http://127.0.0.1:18080/
docker compose down
docker build -t docker-review-cpp:local examples/cpp
docker run --rm docker-review-cpp:local
docker run --rm -e GREETING=你好 docker-review-cpp:local 同学
```

预期：三次计数响应连续递增，down/up 后保持数据；C++ 输出分别为 `Hello, Docker!` 与 `你好, 同学!`。Compose 健康检查访问 `/health`，不增加计数。
