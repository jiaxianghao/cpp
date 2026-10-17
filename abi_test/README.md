# C++ 库变更与 ABI 实验

- [知识总结](docs/library_changes.md)：头文件、编译链接、静态/动态库、API/ABI、更新判断。
- [实验说明](docs/experiments.md)：操作步骤、预期结果和解释。
- `src/`：完整示例源码及自动验证脚本。

在 Linux 上准备 `g++`、`ar`、`readelf` 和 Python 3，然后执行：

```bash
python3 src/run.py
```

构建产物保存在 `src/build/`。脚本可以重复运行，不需要安装到系统目录。
