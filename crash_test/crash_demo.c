/*
 * 简化的崩溃演示程序
 * 演示如何记录崩溃信息并分析崩溃原因
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <execinfo.h>
#include <sys/ucontext.h>
#include <fcntl.h>
#include <time.h>
#include <stdarg.h>
#include <sys/ucontext.h>
#include <sys/reg.h>

#define MAX_BACKTRACE_DEPTH 64
#define LOG_BUFFER_SIZE 4096

// 全局日志文件描述符
static int g_log_fd = -1;

// 日志函数
static void write_log(const char *format, ...)
{
    if (g_log_fd < 0) return;
    
    char buffer[LOG_BUFFER_SIZE];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    write(g_log_fd, buffer, strlen(buffer));
}

// 获取当前时间字符串
static void get_time_str(char *buffer, size_t size)
{
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    strftime(buffer, size, "%Y-%m-%d %H:%M:%S", tm_info);
}

// 记录寄存器信息
static void dump_registers(ucontext_t *ucp)
{
    if (!ucp) return;
    
    char time_str[64];
    get_time_str(time_str, sizeof(time_str));
    
    write_log("%s INFO  <crash_demo.c:dump_registers> $$ ==========================================================\n", time_str);
    write_log("%s INFO  <crash_demo.c:dump_registers> $$ 程序崩溃，记录寄存器信息\n", time_str);
    write_log("%s INFO  <crash_demo.c:dump_registers> $$ ==========================================================\n", time_str);
    
#ifdef __aarch64__
    // ARM64架构寄存器
    write_log("%s INFO  <crash_demo.c:dump_registers> $$ 故障地址: 0x%lx\n", time_str, ucp->uc_mcontext.fault_address);
    write_log("%s INFO  <crash_demo.c:dump_registers> $$ 程序计数器(PC): 0x%lx\n", time_str, ucp->uc_mcontext.pc);
    write_log("%s INFO  <crash_demo.c:dump_registers> $$ 栈指针(SP): 0x%lx\n", time_str, ucp->uc_mcontext.sp);
    write_log("%s INFO  <crash_demo.c:dump_registers> $$ 程序状态: 0x%llx\n", time_str, ucp->uc_mcontext.pstate);
    
    write_log("%s INFO  <crash_demo.c:dump_registers> $$ 通用寄存器:\n", time_str);
    for (int i = 0; i < 31; i++)
    {
        write_log("%s INFO  <crash_demo.c:dump_registers> $$  x%-2d : %016lx\n", time_str, i, ucp->uc_mcontext.regs[i]);
    }
#elif defined(__x86_64__)
    // x86_64架构寄存器
    // write_log("%s INFO  <crash_demo.c:dump_registers> $$ 程序计数器(RIP): 0x%lx\n", time_str, ucp->uc_mcontext.gregs[REG_RIP]);
    // write_log("%s INFO  <crash_demo.c:dump_registers> $$ 栈指针(RSP): 0x%lx\n", time_str, ucp->uc_mcontext.gregs[REG_RSP]);
    // write_log("%s INFO  <crash_demo.c:dump_registers> $$ 基址指针(RBP): 0x%lx\n", time_str, ucp->uc_mcontext.gregs[REG_RBP]);
    // write_log("%s INFO  <crash_demo.c:dump_registers> $$ 通用寄存器:\n", time_str);
    // write_log("%s INFO  <crash_demo.c:dump_registers> $$  RAX : %016lx\n", time_str, ucp->uc_mcontext.gregs[REG_RAX]);
    // write_log("%s INFO  <crash_demo.c:dump_registers> $$  RBX : %016lx\n", time_str, ucp->uc_mcontext.gregs[REG_RBX]);
    // write_log("%s INFO  <crash_demo.c:dump_registers> $$  RCX : %016lx\n", time_str, ucp->uc_mcontext.gregs[REG_RCX]);
    // write_log("%s INFO  <crash_demo.c:dump_registers> $$  RDX : %016lx\n", time_str, ucp->uc_mcontext.gregs[REG_RDX]);
    // write_log("%s INFO  <crash_demo.c:dump_registers> $$  RSI : %016lx\n", time_str, ucp->uc_mcontext.gregs[REG_RSI]);
    // write_log("%s INFO  <crash_demo.c:dump_registers> $$  RDI : %016lx\n", time_str, ucp->uc_mcontext.gregs[REG_RDI]);
    // write_log("%s INFO  <crash_demo.c:dump_registers> $$  R8  : %016lx\n", time_str, ucp->uc_mcontext.gregs[REG_R8]);
    // write_log("%s INFO  <crash_demo.c:dump_registers> $$  R9  : %016lx\n", time_str, ucp->uc_mcontext.gregs[REG_R9]);
    // write_log("%s INFO  <crash_demo.c:dump_registers> $$  R10 : %016lx\n", time_str, ucp->uc_mcontext.gregs[REG_R10]);
    // write_log("%s INFO  <crash_demo.c:dump_registers> $$  R11 : %016lx\n", time_str, ucp->uc_mcontext.gregs[REG_R11]);
    // write_log("%s INFO  <crash_demo.c:dump_registers> $$  R12 : %016lx\n", time_str, ucp->uc_mcontext.gregs[REG_R12]);
    // write_log("%s INFO  <crash_demo.c:dump_registers> $$  R13 : %016lx\n", time_str, ucp->uc_mcontext.gregs[REG_R13]);
    // write_log("%s INFO  <crash_demo.c:dump_registers> $$  R14 : %016lx\n", time_str, ucp->uc_mcontext.gregs[REG_R14]);
    // write_log("%s INFO  <crash_demo.c:dump_registers> $$  R15 : %016lx\n", time_str, ucp->uc_mcontext.gregs[REG_R15]);
#else
    // 其他架构，只记录基本信息
    write_log("%s INFO  <crash_demo.c:dump_registers> $$ 当前架构不支持详细的寄存器记录\n", time_str);
#endif
}

// 记录调用栈
static void dump_backtrace()
{
    void *stacktrace[MAX_BACKTRACE_DEPTH];
    int size = backtrace(stacktrace, MAX_BACKTRACE_DEPTH);
    char **symbols = backtrace_symbols(stacktrace, size);
    
    char time_str[64];
    get_time_str(time_str, sizeof(time_str));
    
    write_log("%s INFO  <crash_demo.c:dump_backtrace> $$ 调用栈信息:\n", time_str);
    write_log("%s INFO  <crash_demo.c:dump_backtrace> $$ Backtrace: %d stack frames.\n", time_str, size);
    
    for (int i = 0; i < size; i++)
    {
        write_log("%s INFO  <crash_demo.c:dump_backtrace> $$ %s\n", time_str, symbols[i]);
    }
    
    free(symbols);
}

// 记录进程内存映射
static void dump_process_maps()
{
    char time_str[64];
    get_time_str(time_str, sizeof(time_str));
    
    write_log("%s INFO  <crash_demo.c:dump_process_maps> $$ 进程内存映射:\n", time_str);
    
    FILE *fp = fopen("/proc/self/maps", "r");
    if (!fp) {
        write_log("%s INFO  <crash_demo.c:dump_process_maps> $$ 无法打开 /proc/self/maps\n", time_str);
        return;
    }
    
    char line[512];
    while (fgets(line, sizeof(line), fp)) {
        write_log("%s INFO  <crash_demo.c:dump_process_maps> $$ %s", time_str, line);
    }
    
    fclose(fp);
}

// 信号处理函数
static void signal_handler(int signo, siginfo_t *si, void *ucp)
{
    char time_str[64];
    get_time_str(time_str, sizeof(time_str));
    
    // 记录崩溃信号信息
    write_log("%s INFO  <crash_demo.c:signal_handler> $$ \n", time_str);
    write_log("%s INFO  <crash_demo.c:signal_handler> $$ ==========================================================\n", time_str);
    write_log("%s INFO  <crash_demo.c:signal_handler> $$ 捕获到信号 %d(%s), 代码 (%d)\n", 
              time_str, si->si_signo, strsignal(si->si_signo), si->si_code);
    write_log("%s INFO  <crash_demo.c:signal_handler> $$ 访问地址: %p\n", time_str, si->si_addr);
    write_log("%s INFO  <crash_demo.c:signal_handler> $$ ==========================================================\n", time_str);
    
    // 记录寄存器信息
    dump_registers((ucontext_t *)ucp);
    
    // 记录调用栈
    dump_backtrace();
    
    // 记录进程内存映射
    dump_process_maps();
    
    // 关闭日志文件
    if (g_log_fd >= 0) {
        close(g_log_fd);
        g_log_fd = -1;
    }
    
    // 恢复默认信号处理并重新发送信号
    signal(signo, SIG_DFL);
    raise(signo);
}

// 设置信号处理
static void setup_signal_handlers()
{
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_sigaction = signal_handler;
    sa.sa_flags = SA_SIGINFO;
    
    // 设置致命信号的处理器
    sigaction(SIGSEGV, &sa, NULL);  // 段错误
    sigaction(SIGBUS, &sa, NULL);   // 总线错误
    sigaction(SIGILL, &sa, NULL);   // 非法指令
    sigaction(SIGFPE, &sa, NULL);   // 浮点异常
    sigaction(SIGABRT, &sa, NULL);  // 中止信号
}

// 故意制造崩溃的函数
static void cause_segmentation_fault()
{
    char time_str[64];
    get_time_str(time_str, sizeof(time_str));
    write_log("%s INFO  <crash_demo.c:cause_segmentation_fault> $$ 即将制造段错误...\n", time_str);
    
    // 访问空指针
    int *ptr = NULL;
    *ptr = 42;  // 这里会触发段错误
}

static void cause_invalid_memory_access()
{
    char time_str[64];
    get_time_str(time_str, sizeof(time_str));
    write_log("%s INFO  <crash_demo.c:cause_invalid_memory_access> $$ 即将访问无效内存...\n", time_str);
    
    // 访问无效地址
    int *ptr = (int *)0x12345678;
    *ptr = 42;  // 这里会触发段错误
}

static void cause_stack_overflow()
{
    char time_str[64];
    get_time_str(time_str, sizeof(time_str));
    write_log("%s INFO  <crash_demo.c:cause_stack_overflow> $$ 即将制造栈溢出...\n", time_str);
    
    // 递归调用导致栈溢出
    char large_array[1024 * 1024];  // 1MB的栈分配
    memset(large_array, 0, sizeof(large_array));
    cause_stack_overflow();  // 递归调用
}

int main(int argc, char *argv[])
{
    // 打开日志文件
    g_log_fd = open("crash_demo.log", O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (g_log_fd < 0) {
        fprintf(stderr, "无法打开日志文件\n");
        return 1;
    }
    
    char time_str[64];
    get_time_str(time_str, sizeof(time_str));
    write_log("%s INFO  <crash_demo.c:main> $$ 程序启动\n", time_str);
    
    // 设置信号处理器
    setup_signal_handlers();
    
    if (argc < 2) {
        write_log("%s INFO  <crash_demo.c:main> $$ 用法: %s <crash_type>\n", time_str, argv[0]);
        write_log("%s INFO  <crash_demo.c:main> $$ crash_type: segfault, invalid_mem, stack_overflow\n", time_str);
        return 1;
    }
    
    if (strcmp(argv[1], "segfault") == 0) {
        write_log("%s INFO  <crash_demo.c:main> $$ 选择段错误测试\n", time_str);
        cause_segmentation_fault();
    } else if (strcmp(argv[1], "invalid_mem") == 0) {
        write_log("%s INFO  <crash_demo.c:main> $$ 选择无效内存访问测试\n", time_str);
        cause_invalid_memory_access();
    } else if (strcmp(argv[1], "stack_overflow") == 0) {
        write_log("%s INFO  <crash_demo.c:main> $$ 选择栈溢出测试\n", time_str);
        cause_stack_overflow();
    } else {
        write_log("%s INFO  <crash_demo.c:main> $$ 未知的崩溃类型: %s\n", time_str, argv[1]);
        return 1;
    }
    
    // 正常情况下不会执行到这里
    if (g_log_fd >= 0) {
        close(g_log_fd);
    }
    
    return 0;
}