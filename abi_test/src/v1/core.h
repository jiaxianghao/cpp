#pragma once
// 固定底层类型，隔离“枚举大小改变”这一因素。
enum Cmd : int { START = 0, STOP = 1, CMD_NUM = 2 };
inline bool valid(int cmd) { return cmd >= 0 && cmd < CMD_NUM; }
extern "C" int calculate(int x);
extern "C" int latest_command();
extern "C" const char* command_name(int cmd);
