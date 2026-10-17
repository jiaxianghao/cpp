int shared = 7;            /* initialized writable data */
int zero_area[1024];        /* zero initialized, normally .bss */
static int local_bias = 2;  /* local symbol, internal linkage */
const char label[] = "ELF";
int add(int x) { return x + shared + local_bias; }
