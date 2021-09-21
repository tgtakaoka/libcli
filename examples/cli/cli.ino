/*
 * Copyright 2020 Tadashi G. Takaoka
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <libcli.h>

#define Console Serial

libcli::Cli &Cli = libcli::Cli::instance();
typedef libcli::Cli::State State;

static void handleCommand(char letter, uintptr_t extra);

/** print prompt and request letter input */
static void prompt() {
    Cli.print(F("> "));
    Cli.readLetter(handleCommand, 0);
}

/** handler for readDec */
static void handleAdd(uint32_t value, uintptr_t extra, State state) {
    static uint32_t last_num;
#define ADD_LEFT 0
#define ADD_RIGHT 1
#define ADD_LIMIT 999999999UL

    if (state == State::CLI_CANCEL) {
        prompt();
        return;
    }
    if (state == State::CLI_DELETE) {
        if (extra == ADD_LEFT)
            return;
        Cli.backspace();
        Cli.readDec(handleAdd, ADD_LEFT, ADD_LIMIT, last_num);
        return;
    }
    if (extra == ADD_LEFT) {
        last_num = value;
        if (state == State::CLI_SPACE) {
            Cli.readDec(handleAdd, ADD_RIGHT, ADD_LIMIT);
            return;
        }
        value = 1;
    }
    if (state == State::CLI_SPACE)
        Cli.println();
    Cli.print(F("add integers: "));
    Cli.printDec(last_num);
    Cli.print(F(" + "));
    Cli.printDec(value);
    Cli.print(F(" = "));
    Cli.printDec(last_num + value);
    Cli.println();
    prompt();
}

/** handler for readHex and readDec */
static void handleDump(uint32_t value, uintptr_t extra, State state) {
    static uint32_t last_addr;
#define DUMP_ADDRESS 0
#define DUMP_LENGTH 1

    if (state == State::CLI_CANCEL) {
        prompt();
        return;
    }
    if (state == State::CLI_DELETE) {
        if (extra == DUMP_ADDRESS)
            return;
        Cli.backspace();
        Cli.readHex(handleDump, DUMP_ADDRESS, 6, last_addr);
        return;
    }
    if (extra == DUMP_ADDRESS) {
        last_addr = value;
        if (state == State::CLI_SPACE) {
            Cli.readDec(handleDump, DUMP_LENGTH, UINT16_MAX);
            return;
        }
        value = 16;
    }
    if (state == State::CLI_SPACE)
        Cli.println();
    Cli.print(F("dump memory: "));
    Cli.printHex(last_addr, 6);
    Cli.print(' ');
    Cli.printDec(value);
    Cli.println();
    prompt();
}

/** handler for readHex */
static void handleMemory(uint32_t value, uintptr_t extra, State state) {
    static uint32_t last_addr;
    static uint8_t mem_buffer[4];
#define MEMORY_ADDRESS uint16_t(-1)
#define MEMORY_INDEX(index) uint16_t(index)

    if (state == State::CLI_CANCEL) {
        prompt();
        return;
    }
    uint16_t index = extra;
    if (state == State::CLI_DELETE) {
        if (extra == MEMORY_ADDRESS)
            return;
        Cli.backspace();
        if (index == 0) {
            Cli.readHex(handleMemory, MEMORY_ADDRESS, 5, last_addr);
            return;
        }
        index--;
        Cli.readHex(handleMemory, MEMORY_INDEX(index), 2, mem_buffer[index]);
        return;
    }
    if (extra == MEMORY_ADDRESS) {
        last_addr = value;
        Cli.readHex(handleMemory, MEMORY_INDEX(0), 2);
        return;
    }

    mem_buffer[index++] = value;
    if (state == State::CLI_SPACE) {
        if (index < sizeof(mem_buffer)) {
            Cli.readHex(handleMemory, MEMORY_INDEX(index), 2);
            return;
        }
        Cli.println();
    }
    Cli.print(F("write memory: "));
    Cli.printHex(last_addr, 5);
    for (uint8_t i = 0; i < index; i++) {
        Cli.print(' ');
        Cli.printHex(mem_buffer[i], 2);
    }
    Cli.println();
    prompt();
}

/** handler for readString */
static void handleLoad(const char *string, uintptr_t extra, State state) {
    (void)extra;
    if (state != State::CLI_CANCEL) {
        Cli.print(F("load file: "));
        Cli.println(string);
    }
    prompt();
}

/** handler for readLetter */
static void handleCommand(char letter, uintptr_t extra) {
    if (letter == 's') {
        Cli.print(F("step"));
    }
    if (letter == 'a') {
        Cli.print(F("add "));
        Cli.readDec(handleAdd, ADD_LEFT, ADD_LIMIT);
        return;
    }
    if (letter == 'd') {
        Cli.print(F("dump "));
        Cli.readHex(handleDump, DUMP_ADDRESS, 6);
        return;
    }
    if (letter == 'l') {
        Cli.print(F("load "));
        Cli.readString(handleLoad, 0);
        return;
    }
    if (letter == 'm') {
        Cli.print(F("memory "));
        Cli.readHex(handleMemory, MEMORY_ADDRESS, 5);
        return;
    }
    if (letter == '?') {
        Cli.print(F("libcli (version "));
        Cli.print(LIBCLI_VERSION_STRING);
        Cli.println(F(") example"));
        Cli.println(F("  ?: help"));
        Cli.println(F("  a: add"));
        Cli.println(F("  s: step"));
        Cli.println(F("  d: dump <address> <length>"));
        Cli.println(F("  l: load <filename>"));
        Cli.print(F("  m: memory <address> <byte>..."));
    }
    Cli.println();
    prompt();
}

void setup() {
    Console.begin(9600);
    Cli.begin(Console);
    prompt();
}

void loop() {
    Cli.loop();
}

// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=4:
