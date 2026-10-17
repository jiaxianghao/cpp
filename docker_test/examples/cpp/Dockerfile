FROM gcc:14-bookworm AS builder
WORKDIR /src
COPY main.cpp .
RUN g++ -std=c++17 -O2 -Wall -Wextra -static main.cpp -o hello

FROM debian:bookworm-slim
COPY --from=builder /src/hello /usr/local/bin/hello
USER 10001:10001
ENTRYPOINT ["/usr/local/bin/hello"]
CMD ["Docker"]
