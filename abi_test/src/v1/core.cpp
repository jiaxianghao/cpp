#include "core.h"
extern "C" int calculate(int x) { return x + 1; }
extern "C" int latest_command() { return STOP; }
extern "C" const char* command_name(int cmd) {
    static const char* names[] = {"START", "STOP"};
    return valid(cmd) ? names[cmd] : "INVALID";
}
