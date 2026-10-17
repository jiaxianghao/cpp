#include "common.h"
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    /* exec 成功后重新从 main 开始，进入此分支。 */
    if (argc == 2 && strcmp(argv[1], "--exec-child") == 0) {
        printf("exec image: pid=%ld\n", (long)getpid());
        return 7;
    }
    setvbuf(stdout, NULL, _IONBF, 0);
    int value = 10;
    printf("before fork: pid=%ld value=%d address=%p\n",
           (long)getpid(), value, (void *)&value);
    pid_t child = fork();
    if (child == -1) die("fork");
    if (child == 0) {
        value = 99;
        printf("child: pid=%ld value=%d address=%p\n",
               (long)getpid(), value, (void *)&value);
        execlp(argv[0], argv[0], "--exec-child", (char *)NULL);
        perror("exec");
        _exit(127);
    }
    int status;
    while (waitpid(child, &status, 0) == -1) {
        if (errno != EINTR) die("waitpid");
    }
    if (!WIFEXITED(status)) {
        fprintf(stderr, "child did not exit normally\n");
        return 1;
    }
    printf("parent: value=%d address=%p child_exit=%d\n",
           value, (void *)&value, WEXITSTATUS(status));
    return value == 10 && WEXITSTATUS(status) == 7 ? 0 : 1;
}
