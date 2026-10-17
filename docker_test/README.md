# Docker 动手复习包

适合学过《深入浅出 Docker》、现在希望重新上手的读者。按每天 30～45 分钟设计，全部材料在本目录，不需要重新通读整本书。

没有假定你使用的书籍版次，因此按知识主题组织，不对应具体章号。内容是独立编写的复习讲义，命令按 Docker CLI 与 Compose v2 核对。

## 从这里开始

1. 看 [7 天复习计划](docs/01-plan.md)，每天完成一个小目标。
2. 看 [核心概念](docs/02-concepts.md)，先理解镜像、容器、进程之间的关系。
3. 按 [动手实验](docs/03-labs.md) 顺序操作；每段代码都标出了所在目录和预期现象。
4. 遇到问题查 [命令速查与排障](docs/04-reference.md)。
5. 最后做 [自测与答案](docs/05-self-check.md)，以及 [C++ 多阶段构建](examples/cpp/README.md)。

## 先运行一次（约 5 分钟）

需要 Docker Engine、Compose v2、curl；首次构建需要访问镜像仓库。示例本身不需要 pip 依赖。以下命令从项目根目录执行：

```bash
cd /home/uisee/cpp/docker_test
docker compose up -d --build --wait
curl -fsS http://127.0.0.1:18080/
curl -fsS http://127.0.0.1:18080/
docker compose logs --tail=20 app
docker compose down
```

两次响应中的 `visits` 应依次增加，`message` 为 `Hello from Compose`。`down` 删除本项目容器和网络，保留计数数据卷。第一次无历史数据时从 1 开始；后续重跑会继续累加。

## 目录

```text
docs/                 中文计划、讲义、实验、速查、自测、验证记录
examples/counter/     Python 标准库 HTTP 服务及 Dockerfile
examples/cpp/         C++ 命令行程序及多阶段 Dockerfile
compose.yaml          计数服务 + 按需运行的客户端
```

Python 示例用来减少业务代码干扰：GET `/` 加一次计数，GET `/health` 只探活。C++ 示例演示把编译工具留在构建阶段。两者都可以在宿主机运行，再放进 Docker，对比环境差异。

## 资料依据

官方文档核对日期：2026-10-16。可变镜像标签方便学习，但不保证不同日期拉取到完全相同的镜像；严格复现应记录并固定镜像 digest。

- [Docker 概览](https://docs.docker.com/get-started/docker-overview/)
- [Compose 概念](https://docs.docker.com/get-started/docker-concepts/the-basics/what-is-docker-compose/)
- [数据卷](https://docs.docker.com/engine/storage/volumes/)
- [容器网络](https://docs.docker.com/engine/network/)
- [多阶段构建](https://docs.docker.com/build/building/multi-stage/)

本机实际验证情况见 [验证记录](docs/06-validation.md)。
