CC = gcc
CFLAGS = -Wall -Wextra -g -O0
TARGET = crash_demo
SOURCE = crash_demo.c

# 默认目标
all: $(TARGET)

# 编译目标
$(TARGET): $(SOURCE)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCE)

# 清理
clean:
	rm -f $(TARGET) crash_demo.log

# 测试段错误
test_segfault: $(TARGET)
	./$(TARGET) segfault

# 测试无效内存访问
test_invalid_mem: $(TARGET)
	./$(TARGET) invalid_mem

# 测试栈溢出
test_stack_overflow: $(TARGET)
	./$(TARGET) stack_overflow

# 查看日志
log:
	@if [ -f crash_demo.log ]; then \
		echo "=== 崩溃日志内容 ==="; \
		cat crash_demo.log; \
	else \
		echo "日志文件不存在"; \
	fi

# 分析日志
analyze: log
	@echo "=== 日志分析 ==="
	@if [ -f crash_demo.log ]; then \
		echo "1. 查找崩溃信号:"; \
		grep "捕获到信号" crash_demo.log || echo "未找到崩溃信号"; \
		echo ""; \
		echo "2. 查找故障地址:"; \
		grep "故障地址" crash_demo.log || echo "未找到故障地址"; \
		echo ""; \
		echo "3. 查找程序计数器:"; \
		grep "程序计数器" crash_demo.log || echo "未找到程序计数器"; \
		echo ""; \
		echo "4. 查找调用栈:"; \
		grep "Backtrace" crash_demo.log || echo "未找到调用栈"; \
	else \
		echo "日志文件不存在"; \
	fi

.PHONY: all clean test_segfault test_invalid_mem test_stack_overflow log analyze 