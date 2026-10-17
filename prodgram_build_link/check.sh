#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")"
expect_output() {
    local expected="$1"
    shift
    local actual
    actual=$("$@")
    if [[ "$actual" != "$expected" ]]; then
        printf 'FAIL: %s\nexpected: %s\nactual: %s\n' "$*" "$expected" "$actual" >&2
        exit 1
    fi
}
expect_failure() {
    local name="$1" pattern="$2"
    shift 2
    if "$@" >"build/$name.log" 2>&1; then
        printf 'FAIL: expected failure: %s\n' "$name" >&2
        exit 1
    fi
    if ! grep -Eq "$pattern" "build/$name.log"; then
        cat "build/$name.log" >&2
        exit 1
    fi
    printf 'PASS expected failure: %s (build/%s.log)\n' "$name" "$name"
}
export LC_ALL=C
expect_output 'value=21' build/01/app
expect_output 'shared=7 add=14' build/02/app
expect_output 'twice=42' build/03/app
expect_output 'answer=42' build/04/app
expect_output 'answer=100' env LD_PRELOAD="$PWD/build/04/override.so" build/04/app
expect_output 'plugin=73' build/05/app ./build/05/plugin.so
build/06/app > build/memory.log
grep -q '/proc/self/maps' build/memory.log
expect_output $'construct global\nenter main\nconstruct local\nreturn from main\ndestroy local\natexit callback\ndestroy global' build/07/app
expect_output $'construct global\nenter main\nconstruct local\natexit callback\ndestroy global' build/07/app exit
expect_output $'construct global\nenter main\nconstruct local' build/07/app quick
expect_output 'c=5 cpp=9' build/08/app
expect_failure missing-symbol 'undefined reference.*(shared|add)' gcc -no-pie build/02/main.o -o build/02/missing
expect_failure duplicate-symbol 'multiple definition.*shared' gcc -no-pie build/02/main.o build/02/data.o labs/02_symbols/duplicate.c -o build/02/duplicate
expect_failure archive-order 'undefined reference.*twice' gcc -Lbuild/03 -lmathdemo build/03/main.o -o build/03/wrong-order
expect_failure missing-plugin 'cannot open shared object file' build/05/app ./build/05/no-such-plugin.so
expect_failure missing-export 'undefined symbol.*plugin_value' build/05/app ./build/04/libanswer.so
expect_failure cpp-name 'undefined reference.*c_add' g++ labs/08_abi/bad.cpp build/08/api.o -o build/08/bad
if nm build/03/app | grep -q unused_function; then
    echo 'FAIL: unused archive member was linked' >&2
    exit 1
fi
printf 'PASS: all 8 labs; expected failures verified.\n'
