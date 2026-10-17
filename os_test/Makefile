CC = cc
CFLAGS = -std=c11 -O2 -Wall -Wextra -Wpedantic -Werror -D_POSIX_C_SOURCE=200809L
PROGRAMS = process pipe counter queue
BINARIES = $(addprefix build/,$(PROGRAMS))

.PHONY: all check clean
all: $(BINARIES)

build/%: labs/%.c labs/common.h | build
	$(CC) $(CFLAGS) $< -o $@ -pthread

build:
	mkdir -p build

check: all
	python3 -m unittest discover -s tests -v

clean:
	rm -f $(BINARIES)
