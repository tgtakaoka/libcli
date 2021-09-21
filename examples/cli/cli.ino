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

using libcli::Cli;
static Cli cli;

static void handleCommand(char letter, uintptr_t extra);

/** print prompt and request letter input */
static void prompt() {
    cli.print(F("> "));
    cli.readLetter(handleCommand, 0);
}

/** handler for readDec32 */
static void handleAdd(uint32_t value, uintptr_t extra, Cli::State state) {
    static uint32_t last_num;
#define ADD_LEFT 0
#define ADD_RIGHT 1

    if (state == Cli::State::CLI_CANCEL) {
        prompt();
        return;
    }
    if (state == Cli::State::CLI_DELETE) {
        if (extra == ADD_LEFT)
            return;
        cli.backspace();
        cli.readDec32(handleAdd, ADD_LEFT, last_num);
        return;
    }
    if (extra == ADD_LEFT) {
        last_num = value;
        if (state == Cli::State::CLI_SPACE) {
            cli.readDec32(handleAdd, ADD_RIGHT);
            return;
        }
        value = 1;
    }
    if (state == Cli::State::CLI_SPACE)
        cli.println();
    cli.print(F("add integers: "));
    cli.printDec32(last_num);
    cli.print(F(" + "));
    cli.printDec32(value);
    cli.print(F(" = "));
    cli.printDec32(last_num + value);
    cli.println();
    prompt();
}

/** handler for readHex16 and readDec8 */
static void handleDump(uint32_t value, uintptr_t extra, Cli::State state) {
    static uint32_t last_addr;
#define DUMP_ADDRESS 0
#define DUMP_LENGTH 1

    if (state == Cli::State::CLI_CANCEL) {
        prompt();
        return;
    }
    if (state == Cli::State::CLI_DELETE) {
        if (extra == DUMP_ADDRESS)
            return;
        cli.backspace();
        cli.readHex16(handleDump, DUMP_ADDRESS, last_addr);
        return;
    }
    if (extra == DUMP_ADDRESS) {
        last_addr = value;
        if (state == Cli::State::CLI_SPACE) {
            cli.readDec8(handleDump, DUMP_LENGTH);
            return;
        }
        value = 16;
    }
    if (state == Cli::State::CLI_SPACE)
        cli.println();
    cli.print(F("dump memory: "));
    cli.printHex16(last_addr);
    cli.print(' ');
    cli.printDec8(value);
    cli.println();
    prompt();
}

/** handler for readHex16 and readHex8 */
static void handleMemory(uint32_t value, uintptr_t extra, Cli::State state) {
    static uint32_t last_addr;
    static uint8_t mem_buffer[4];
#define MEMORY_ADDRESS uint16_t(-1)
#define MEMORY_INDEX(index) uint16_t(index)

    if (state == Cli::State::CLI_CANCEL) {
        prompt();
        return;
    }
    uint16_t index = extra;
    if (state == Cli::State::CLI_DELETE) {
        if (extra == MEMORY_ADDRESS)
            return;
        cli.backspace();
        if (index == 0) {
            cli.readHex16(handleMemory, MEMORY_ADDRESS, last_addr);
            return;
        }
        index--;
        cli.readHex8(handleMemory, MEMORY_INDEX(index), mem_buffer[index]);
        return;
    }
    if (extra == MEMORY_ADDRESS) {
        last_addr = value;
        cli.readHex8(handleMemory, MEMORY_INDEX(0));
        return;
    }

    mem_buffer[index++] = value;
    if (state == Cli::State::CLI_SPACE) {
        if (index < sizeof(mem_buffer)) {
            cli.readHex8(handleMemory, MEMORY_INDEX(index));
            return;
        }
        cli.println();
    }
    cli.print(F("write memory: "));
    cli.printHex16(last_addr);
    for (uint8_t i = 0; i < index; i++) {
        cli.print(' ');
        cli.printHex8(mem_buffer[i]);
    }
    cli.println();
    prompt();
}

/** handler for readString */
static void handleLoad(char *string, uintptr_t extra, Cli::State state) {
    (void)extra;
    if (state != Cli::State::CLI_CANCEL) {
        cli.print(F("load file: "));
        cli.println(string);
    }
    prompt();
}

/** handler for readLetter */
static void handleCommand(char letter, uintptr_t extra) {
    if (letter == 's') {
        cli.print(F("step"));
    }
    if (letter == 'a') {
        cli.print(F("add "));
        cli.readDec32(handleAdd, ADD_LEFT);
        return;
    }
    if (letter == 'd') {
        cli.print(F("dump "));
        cli.readHex16(handleDump, DUMP_ADDRESS);
        return;
    }
    if (letter == 'l') {
        cli.print(F("load "));
        cli.readString(handleLoad, 0);
        return;
    }
    if (letter == 'm') {
        cli.print(F("memory "));
        cli.readHex16(handleMemory, MEMORY_ADDRESS);
        return;
    }
    if (letter == '?') {
        cli.print(F("libcli (version "));
        cli.print(LIBCLI_VERSION_STRING);
        cli.println(F(") example"));
        cli.println(F("  ?: help"));
        cli.println(F("  a: add"));
        cli.println(F("  s: step"));
        cli.println(F("  d: dump <address> <length>"));
        cli.println(F("  l: load <filename>"));
        cli.print(F("  m: memory <address> <byte>..."));
    }
    cli.println();
    prompt();
}

void setup() {
    Console.begin(9600);
    cli.begin(Console);
    prompt();
}

void loop() {
    cli.loop();
}

// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=4:
