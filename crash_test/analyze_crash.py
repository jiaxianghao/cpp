#!/usr/bin/env python3
"""
崩溃日志分析脚本
用于分析crash_demo程序生成的崩溃日志
"""

import re
import sys
from datetime import datetime

def parse_crash_log(log_file):
    """解析崩溃日志文件"""
    try:
        with open(log_file, 'r', encoding='utf-8') as f:
            content = f.read()
    except FileNotFoundError:
        print(f"错误: 找不到日志文件 {log_file}")
        return None
    except Exception as e:
        print(f"错误: 读取日志文件失败: {e}")
        return None
    
    return content

def extract_signal_info(content):
    """提取信号信息"""
    pattern = r'捕获到信号 (\d+)\(([^)]+)\), 代码 \((\d+)\)'
    matches = re.findall(pattern, content)
    
    if matches:
        signo, signame, code = matches[0]
        return {
            'signal_number': int(signo),
            'signal_name': signame,
            'signal_code': int(code)
        }
    return None

def extract_fault_address(content):
    """提取故障地址"""
    pattern = r'故障地址: (0x[0-9a-fA-F]+)'
    matches = re.findall(pattern, content)
    
    if matches:
        return matches[0]
    return None

def extract_program_counter(content):
    """提取程序计数器"""
    pattern = r'程序计数器\([^)]+\): (0x[0-9a-fA-F]+)'
    matches = re.findall(pattern, content)
    
    if matches:
        return matches[0]
    return None

def extract_registers(content):
    """提取寄存器信息"""
    registers = {}
    
    # 查找寄存器行
    pattern = r'x(\d+)\s+:\s+(0x[0-9a-fA-F]+)'
    matches = re.findall(pattern, content)
    
    for reg_num, value in matches:
        registers[f'x{reg_num}'] = value
    
    return registers

def extract_backtrace(content):
    """提取调用栈信息"""
    backtrace_lines = []
    
    # 查找调用栈行
    lines = content.split('\n')
    in_backtrace = False
    
    for line in lines:
        if 'Backtrace:' in line:
            in_backtrace = True
            continue
        elif in_backtrace and line.strip() and '$$' in line:
            # 提取调用栈信息
            match = re.search(r'\$\$ (.+)$', line)
            if match:
                backtrace_lines.append(match.group(1))
        elif in_backtrace and not line.strip():
            break
    
    return backtrace_lines

def analyze_fault_address(fault_addr):
    """分析故障地址"""
    if not fault_addr:
        return "未找到故障地址"
    
    try:
        addr_int = int(fault_addr, 16)
        
        # 尝试将地址转换为ASCII字符
        addr_bytes = addr_int.to_bytes(8, byteorder='little')
        ascii_str = ""
        for byte in addr_bytes:
            if 32 <= byte <= 126:  # 可打印ASCII字符
                ascii_str += chr(byte)
            else:
                ascii_str += '.'
        
        analysis = f"故障地址: {fault_addr}\n"
        analysis += f"  数值: {addr_int}\n"
        analysis += f"  ASCII: {ascii_str}\n"
        
        # 判断地址范围
        if addr_int == 0:
            analysis += "  分析: 空指针访问\n"
        elif addr_int < 0x1000:
            analysis += "  分析: 访问了低地址空间（可能是空指针）\n"
        elif addr_int > 0x7fffffffffff:
            analysis += "  分析: 访问了高地址空间（可能是野指针）\n"
        else:
            analysis += "  分析: 访问了中等地址空间\n"
        
        return analysis
    except Exception as e:
        return f"故障地址分析失败: {e}"

def analyze_registers(registers):
    """分析寄存器信息"""
    if not registers:
        return "未找到寄存器信息"
    
    analysis = "寄存器分析:\n"
    
    # 分析x0寄存器（通常包含函数参数）
    if 'x0' in registers:
        x0_value = registers['x0']
        try:
            x0_int = int(x0_value, 16)
            x0_bytes = x0_int.to_bytes(8, byteorder='little')
            x0_ascii = ""
            for byte in x0_bytes:
                if 32 <= byte <= 126:
                    x0_ascii += chr(byte)
                else:
                    x0_ascii += '.'
            
            analysis += f"  x0 (参数1): {x0_value} -> ASCII: {x0_ascii}\n"
        except:
            analysis += f"  x0 (参数1): {x0_value}\n"
    
    # 分析其他重要寄存器
    important_regs = ['x1', 'x2', 'x30']  # x30是链接寄存器
    for reg in important_regs:
        if reg in registers:
            analysis += f"  {reg}: {registers[reg]}\n"
    
    return analysis

def analyze_backtrace(backtrace):
    """分析调用栈"""
    if not backtrace:
        return "未找到调用栈信息"
    
    analysis = "调用栈分析:\n"
    
    for i, frame in enumerate(backtrace):
        analysis += f"  {i}: {frame}\n"
        
        # 尝试提取函数名和地址
        if '(' in frame and ')' in frame:
            func_match = re.search(r'([^(]+)\([^)]*\)', frame)
            if func_match:
                func_name = func_match.group(1).strip()
                analysis += f"      函数: {func_name}\n"
    
    return analysis

def main():
    if len(sys.argv) != 2:
        print("用法: python3 analyze_crash.py <log_file>")
        print("示例: python3 analyze_crash.py crash_demo.log")
        sys.exit(1)
    
    log_file = sys.argv[1]
    content = parse_crash_log(log_file)
    
    if not content:
        sys.exit(1)
    
    print("=" * 60)
    print("崩溃日志分析报告")
    print("=" * 60)
    
    # 1. 分析信号信息
    signal_info = extract_signal_info(content)
    if signal_info:
        print(f"\n1. 崩溃信号:")
        print(f"   信号编号: {signal_info['signal_number']}")
        print(f"   信号名称: {signal_info['signal_name']}")
        print(f"   信号代码: {signal_info['signal_code']}")
        
        # 根据信号类型给出建议
        if signal_info['signal_number'] == 11:  # SIGSEGV
            print("   分析: 段错误 - 访问了无效的内存地址")
        elif signal_info['signal_number'] == 7:   # SIGBUS
            print("   分析: 总线错误 - 内存访问对齐问题")
        elif signal_info['signal_number'] == 4:   # SIGILL
            print("   分析: 非法指令 - 执行了无效的机器指令")
    else:
        print("\n1. 崩溃信号: 未找到")
    
    # 2. 分析故障地址
    fault_addr = extract_fault_address(content)
    print(f"\n2. 故障地址分析:")
    print(analyze_fault_address(fault_addr))
    
    # 3. 分析程序计数器
    pc = extract_program_counter(content)
    if pc:
        print(f"\n3. 程序计数器:")
        print(f"   PC: {pc}")
    else:
        print(f"\n3. 程序计数器: 未找到")
    
    # 4. 分析寄存器
    registers = extract_registers(content)
    print(f"\n4. 寄存器分析:")
    print(analyze_registers(registers))
    
    # 5. 分析调用栈
    backtrace = extract_backtrace(content)
    print(f"\n5. 调用栈分析:")
    print(analyze_backtrace(backtrace))
    
    # 6. 总结和建议
    print(f"\n6. 总结和建议:")
    if signal_info and signal_info['signal_number'] == 11:
        print("   - 这是一个段错误(SIGSEGV)")
        print("   - 可能的原因:")
        print("     * 空指针解引用")
        print("     * 访问已释放的内存")
        print("     * 数组越界访问")
        print("     * 栈溢出")
        print("   - 建议:")
        print("     * 检查指针的有效性")
        print("     * 使用调试器设置断点")
        print("     * 检查内存分配和释放")
    
    print("\n" + "=" * 60)

if __name__ == "__main__":
    main() 