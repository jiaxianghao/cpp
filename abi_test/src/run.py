#!/usr/bin/env python3
"""Linux/GCC 实验；每一步验证输出，不改系统库搜索路径。"""
from pathlib import Path
import shutil
import subprocess

HERE = Path(__file__).resolve().parent
BUILD = HERE / "build"
BUILD.mkdir(exist_ok=True)

def run(*args):
    return subprocess.check_output([str(a) for a in args], text=True)

def check(label, executable, expected):
    output = run(executable).strip()
    print(f"{label}\n  {output}")
    assert output == expected, (label, output, expected)

for version in ("v1", "v2"):
    out = BUILD / version
    out.mkdir(exist_ok=True)
    run("g++", "-std=c++11", "-Wall", "-Wextra", "-Werror", "-fPIC", "-c",
        HERE / version / "core.cpp", "-o", out / "core.o")
    run("g++", "-shared", out / "core.o", "-Wl,-soname,libcore.so", "-o", out / "libcore.so")
    run("ar", "rcs", out / "libcore.a", out / "core.o")
    run("g++", "-std=c++11", "-O2", "-I", HERE / version,
        "-c", HERE / "client.cpp", "-o", out / "client.o")

active = BUILD / "active"
active.mkdir(exist_ok=True)
shutil.copy2(BUILD / "v1/libcore.so", active / "libcore.so")
run("g++", BUILD / "v1/client.o", "-L", active, "-lcore",
    "-Wl,-rpath,$ORIGIN/active", "-o", BUILD / "dynamic_old")
old = "calculate(10)=11 CMD_NUM=2 cmd=1 name=STOP accepted=1"
mixed = "calculate(10)=12 CMD_NUM=2 cmd=2 name=PAUSE accepted=0"
new = "calculate(10)=12 CMD_NUM=3 cmd=2 name=PAUSE accepted=1"
check("1. 旧程序 + 旧动态库", BUILD / "dynamic_old", old)
# 前一次进程已经结束，现在替换文件，再启动同一个程序。
shutil.copy2(BUILD / "v2/libcore.so", active / "libcore.so")
check("2. 只换动态库：算法更新，但旧枚举检查仍拒绝新命令", BUILD / "dynamic_old", mixed)
run("g++", BUILD / "v2/client.o", "-L", active, "-lcore",
    "-Wl,-rpath,$ORIGIN/active", "-o", BUILD / "dynamic_new")
check("3. 使用新头文件重编调用方", BUILD / "dynamic_new", new)
shutil.copy2(BUILD / "v1/libcore.a", active / "libcore.a")
run("g++", BUILD / "v1/client.o", active / "libcore.a", "-o", BUILD / "static_old")
check("4. 静态链接旧 core", BUILD / "static_old", old)
shutil.copy2(BUILD / "v2/libcore.a", active / "libcore.a")
check("5. 只替换静态库文件：旧程序完全不变", BUILD / "static_old", old)
run("g++", BUILD / "v1/client.o", active / "libcore.a", "-o", BUILD / "static_relinked")
check("6. 仅重链接：算法更新，调用方的枚举检查仍旧", BUILD / "static_relinked", mixed)
run("g++", BUILD / "v2/client.o", active / "libcore.a", "-o", BUILD / "static_new")
check("7. 重编调用方并链接新静态库", BUILD / "static_new", new)
run("g++", "-std=c++11", HERE / "layout.cpp", "-o", BUILD / "layout")
print("8. 结构体布局（实际值取决于平台）\n" + run(BUILD / "layout"))
assert "libcore.so" in run("readelf", "-d", BUILD / "dynamic_old")
assert "libcore.so" not in run("readelf", "-d", BUILD / "static_old")
print("全部验证通过。静态示例仅静态链接 core，系统运行库仍可能动态链接。")
