CC := gcc
CXX := g++
CFLAGS := -std=c11 -O0 -g -Wall -Wextra -fno-common
CXXFLAGS := -std=c++11 -O0 -g -Wall -Wextra
B := build

.PHONY: all check clean
all: $(B)/01/app $(B)/02/app $(B)/03/app $(B)/04/app $(B)/04/override.so $(B)/05/app $(B)/05/plugin.so $(B)/06/app $(B)/07/app $(B)/08/app

$(B)/01 $(B)/02 $(B)/03 $(B)/04 $(B)/05 $(B)/06 $(B)/07 $(B)/08:
	mkdir -p $@

$(B)/01/main.i: labs/01_pipeline/main.c | $(B)/01
	$(CC) -E $< -o $@
$(B)/01/main.s: $(B)/01/main.i
	$(CC) $(CFLAGS) -S $< -o $@
$(B)/01/main.o: $(B)/01/main.s
	$(CC) -c $< -o $@
$(B)/01/app: $(B)/01/main.o
	$(CC) $< -o $@

$(B)/02/%.o: labs/02_symbols/%.c | $(B)/02
	$(CC) $(CFLAGS) -fno-pie -c $< -o $@
$(B)/02/app: $(B)/02/main.o $(B)/02/data.o
	$(CC) -no-pie $^ -Wl,-Map,$(B)/02/link.map -o $@

$(B)/03/%.o: labs/03_archive/%.c | $(B)/03
	$(CC) $(CFLAGS) -c $< -o $@
$(B)/03/libmathdemo.a: $(B)/03/twice.o $(B)/03/unused.o
	ar rcs $@ $^
$(B)/03/app: $(B)/03/main.o $(B)/03/libmathdemo.a
	$(CC) $(B)/03/main.o -L$(B)/03 -lmathdemo -o $@

$(B)/04/libanswer.so: labs/04_shared/answer.c | $(B)/04
	$(CC) $(CFLAGS) -fPIC -shared $< -Wl,-soname,libanswer.so -o $@
$(B)/04/override.so: labs/04_shared/override.c | $(B)/04
	$(CC) $(CFLAGS) -fPIC -shared $< -o $@
$(B)/04/app: labs/04_shared/main.c $(B)/04/libanswer.so
	$(CC) $(CFLAGS) -fplt $< -L$(B)/04 -lanswer -Wl,-z,lazy,--enable-new-dtags,-rpath,'$$ORIGIN' -o $@

$(B)/05/app: labs/05_dlopen/main.c | $(B)/05
	$(CC) $(CFLAGS) $< -ldl -o $@
$(B)/05/plugin.so: labs/05_dlopen/plugin.c | $(B)/05
	$(CC) $(CFLAGS) -fPIC -shared $< -o $@
$(B)/06/app: labs/06_memory/main.c | $(B)/06
	$(CC) $(CFLAGS) -fPIE -pie $< -o $@
$(B)/07/app: labs/07_runtime/main.cpp | $(B)/07
	$(CXX) $(CXXFLAGS) $< -o $@
$(B)/08/api.o: labs/08_abi/api.c labs/08_abi/api.h | $(B)/08
	$(CC) $(CFLAGS) -c $< -o $@
$(B)/08/main.o: labs/08_abi/main.cpp labs/08_abi/api.h | $(B)/08
	$(CXX) $(CXXFLAGS) -c $< -o $@
$(B)/08/app: $(B)/08/main.o $(B)/08/api.o
	$(CXX) $^ -o $@

check: all
	bash check.sh
clean:
	rm -rf $(B)
