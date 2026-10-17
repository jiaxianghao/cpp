#include "common.h"
#include <sys/wait.h>
#include <unistd.h>

int main(void)
{
    int ends[2];
    if (pipe(ends) == -1) die("pipe");
    pid_t child = fork();
    if (child == -1) die("fork");
    if (child == 0) {
        close(ends[1]); /* 子进程只读，不能保留自己的写端。 */
        char buffer[4]; /* 故意分块读取：管道没有消息边界。 */
        size_t total = 0;
        for (;;) {
            ssize_t n = read(ends[0], buffer, sizeof(buffer));
            if (n == 0) break;
            if (n < 0) {
                if (errno == EINTR) continue;
                die("read");
            }
            total += (size_t)n;
        }
        close(ends[0]);
        printf("child: bytes=%zu EOF reached\n", total);
        return total == 11 ? 0 : 1;
    }
    close(ends[0]);
    const char message[] = "hello pipe\n";
    size_t sent = 0;
    while (sent < sizeof(message) - 1) {
        ssize_t n = write(ends[1], message + sent, sizeof(message) - 1 - sent);
        if (n < 0) {
            if (errno == EINTR) continue;
            die("write");
        }
        sent += (size_t)n;
    }
    close(ends[1]); /* 所有写端关闭且缓冲读完后，read 才返回 EOF。 */
    int status;
    while (waitpid(child, &status, 0) == -1) {
        if (errno != EINTR) die("waitpid");
    }
    return WIFEXITED(status) && WEXITSTATUS(status) == 0 ? 0 : 1;
}
