#include "core.h"
extern "C" int calculate(int x) { return x + 2; }
extern "C" int latest_command() { return PAUSE; }
extern "C" const char* command_name(int cmd) {
    static const char* names[] = {"START", "STOP", "PAUSE"};
    return valid(cmd) ? names[cmd] : "INVALID";
}
