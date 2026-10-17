#pragma once
enum Cmd : int { START = 0, STOP = 1, PAUSE = 2, CMD_NUM = 3 };
inline bool valid(int cmd) { return cmd >= 0 && cmd < CMD_NUM; }
extern "C" int calculate(int x);
extern "C" int latest_command();
extern "C" const char* command_name(int cmd);
