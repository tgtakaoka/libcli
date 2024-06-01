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

/** Get the singleton instance. */
libcli::Cli cli;
using State = libcli::Cli::State;

static void handleCommand(char letter, uintptr_t context);

/** print prompt and request letter input */
static void prompt() {
    cli.print(F("> "));
    cli.readLetter(handleCommand, 0);
}

/** callback for readDec */
static constexpr int ADD_LEFT = 0;
static constexpr int ADD_RIGHT = 1;
static constexpr uint32_t DEC_LIMIT = 999999999UL;

static void handleAddDec(uint32_t value, uintptr_t context, State state) {
    static uint32_t last_num;

    if (state == State::CLI_CANCEL) {
        prompt();
        return;
    }
    if (state == State::CLI_DELETE) {
        if (context == ADD_LEFT)
            return;
        cli.backspace();
        cli.readDec(handleAddDec, ADD_LEFT, DEC_LIMIT, last_num);
        return;
    }
    if (context == ADD_LEFT) {
        last_num = value;
        if (state == State::CLI_SPACE) {
            cli.readDec(handleAddDec, ADD_RIGHT, DEC_LIMIT);
            return;
        }
        value = 1;
    }

    cli.println();
    cli.print(F("add integers: "));
    cli.printDec(last_num);
    cli.print(F(" + "));
    cli.printDec(value);
    cli.print(F(" = "));
    cli.printlnDec(last_num + value);
    prompt();
}

/** callback for readHex */
static constexpr int HEX_WIDTH = 7;
static constexpr uint32_t HEX_LIMIT = 0x0FFFFFFFUL;

static void handleAddHex(uint32_t value, uintptr_t context, State state) {
    static uint32_t last_num;

    if (state == State::CLI_CANCEL) {
        prompt();
        return;
    }
    if (state == State::CLI_DELETE) {
        if (context == ADD_LEFT)
            return;
        cli.backspace();
        cli.readHex(handleAddHex, ADD_LEFT, HEX_LIMIT, last_num);
        return;
    }
    if (context == ADD_LEFT) {
        last_num = value;
        if (state == State::CLI_SPACE) {
            cli.readHex(handleAddHex, ADD_RIGHT, HEX_LIMIT);
            return;
        }
        value = 1;
    }

    cli.println();
    cli.print(F("add hexadecimal: "));
    cli.printHex(last_num, HEX_WIDTH);
    cli.print(F(" + "));
    cli.printHex(value, HEX_WIDTH);
    cli.print(F(" = "));
    cli.printlnHex(last_num + value, HEX_WIDTH);
    prompt();
}

/** callback for readHex and readDec */
static constexpr int DUMP_ADDRESS = 0;
static constexpr int DUMP_LENGTH = 1;
static constexpr int DUMP_ADDR_WIDTH = 6;
static constexpr uint32_t DUMP_ADDR_LIMIT = 0xFFFFFFUL;

static void handleDump(uint32_t value, uintptr_t context, State state) {
    static uint32_t last_addr;

    if (state == State::CLI_CANCEL) {
        prompt();
        return;
    }
    if (state == State::CLI_DELETE) {
        if (context == DUMP_ADDRESS)
            return;
        cli.backspace();
        cli.readHex(handleDump, DUMP_ADDRESS, DUMP_ADDR_LIMIT, last_addr);
        return;
    }
    if (context == DUMP_ADDRESS) {
        last_addr = value;
        if (state == State::CLI_SPACE) {
            cli.readDec(handleDump, DUMP_LENGTH, UINT16_MAX);
            return;
        }
        value = 16;
    }

    cli.println();
    cli.print(F("dump memory: "));
    cli.printHex(last_addr, DUMP_ADDR_WIDTH);
    cli.print(' ');
    cli.printlnDec(value);
    const auto end = last_addr + value;
    for (auto base = last_addr & ~0xF; base < end; base += 16) {
        cli.printHex(base, DUMP_ADDR_WIDTH);
        cli.print(F(": "));
        for (auto i = 0; i < 16; i++) {
            const auto a = base + i;
            if (a < last_addr || a >= end) {
                cli.printStr(F("_"), 4);
            } else {
                cli.printDec(static_cast<uint8_t>(a), 4);
            }
        }
        cli.println();
    }
    prompt();
}

/** callback for readHex */
static constexpr uint16_t MEMORY_ADDRESS = uint16_t(-1);
static uint16_t MEMORY_INDEX(int index) {
    return uint16_t(index);
}
static constexpr int MEMORY_ADDR_WIDTH = 5;
static constexpr uint32_t MEMORY_ADDR_LIMIT = 0xFFFFFUL;

static void handleMemory(uint32_t value, uintptr_t context, State state) {
    static uint32_t last_addr;
    static uint8_t mem_buffer[4];

    if (state == State::CLI_CANCEL) {
        prompt();
        return;
    }
    uint16_t index = context;
    if (state == State::CLI_DELETE) {
        if (context == MEMORY_ADDRESS)
            return;
        cli.backspace();
        if (index == 0) {
            cli.readHex(handleMemory, MEMORY_ADDRESS, MEMORY_ADDR_LIMIT, last_addr);
            return;
        }
        index--;
        cli.readHex(handleMemory, MEMORY_INDEX(index), UINT8_MAX, mem_buffer[index]);
        return;
    }
    if (context == MEMORY_ADDRESS) {
        last_addr = value;
        cli.readHex(handleMemory, MEMORY_INDEX(0), UINT8_MAX);
        return;
    }

    mem_buffer[index++] = value;
    if (state == State::CLI_SPACE) {
        if (index < sizeof(mem_buffer)) {
            cli.readHex(handleMemory, MEMORY_INDEX(index), UINT8_MAX);
            return;
        }
    }

    cli.println();
    cli.print(F("write memory: "));
    cli.printHex(last_addr, MEMORY_ADDR_WIDTH);
    for (uint8_t i = 0; i < index; i++) {
        cli.print(' ');
        cli.printHex(mem_buffer[i], 2);
    }
    cli.println();
    prompt();
}

static char str_buffer[40];

/** callback for readLine */
static void handleLoad(char *line, uintptr_t context, State state) {
    (void)context;
    if (state != State::CLI_CANCEL) {
        cli.println();
        cli.print(F("load file: '"));
        cli.print(line);
        cli.println('\'');
    }
    prompt();
}

/** callback for readWord */
static constexpr size_t WORD_LEN = 10 + 1;

static void handleWord(char *word, uintptr_t context, State state) {
    static char words[4][WORD_LEN];
    static constexpr size_t WORD_MAX = sizeof(words) / sizeof(words[0]);

    if (state == State::CLI_CANCEL) {
        prompt();
        return;
    }
    size_t index = context;
    if (state == State::CLI_DELETE) {
        if (index != 0) {
            cli.backspace();
            index--;
            strcpy(str_buffer, words[index]);
            cli.readWord(handleWord, index, str_buffer, WORD_LEN, true);
        }
        return;
    }

    strncpy(words[index], word, WORD_LEN);
    words[index][0] = toUpperCase(words[index][0]);
    cli.backspace(strlen(words[index]) + 1);
    cli.print(words[index]);
    index++;
    if (state == State::CLI_SPACE) {
        if (index < WORD_MAX) {
            cli.print(' ');
            cli.readWord(handleWord, index, str_buffer, WORD_LEN);
            return;
        }
    }

    cli.println();
    for (size_t i = 0; i < index; i++) {
        cli.print(F("  word "));
        cli.printDec(i + 1);
        cli.print(F(": "));
        cli.printStr(words[i], -10);
        cli.println(F("!"));
    }
    prompt();
}

/** callback for readLetter */
static void handleCommand(char letter, uintptr_t context) {
    if (letter == 's') {
        cli.print(F("step"));
    }
    if (letter == 'a') {
        cli.print(F("add decimal "));
        cli.readDec(handleAddDec, ADD_LEFT, DEC_LIMIT);
        return;
    }
    if (letter == 'h') {
        cli.print(F("add hexadecimal "));
        cli.readHex(handleAddHex, ADD_LEFT, HEX_LIMIT);
        return;
    }
    if (letter == 'l') {
        cli.print(F("load "));
        cli.readLine(handleLoad, 0, str_buffer, sizeof(str_buffer));
        return;
    }
    if (letter == 'w') {
        cli.print(F("word "));
        cli.readWord(handleWord, 0, str_buffer, WORD_LEN);
        return;
    }
    if (letter == 'd') {
        cli.print(F("dump "));
        cli.readHex(handleDump, DUMP_ADDRESS, DUMP_ADDR_LIMIT);
        return;
    }
    if (letter == 'm') {
        cli.print(F("memory "));
        cli.readHex(handleMemory, MEMORY_ADDRESS, MEMORY_ADDR_LIMIT);
        return;
    }
    if (letter == '?') {
        cli.print(F("libcli (version "));
        cli.print(LIBCLI_VERSION_STRING);
        cli.println(F(") example"));
        cli.println(F("  ?: help"));
        cli.println(F("  s: step"));
        cli.println(F("  a: add decimal"));
        cli.println(F("  h: add hexadecimal"));
        cli.println(F("  l: load <filename>"));
        cli.println(F("  w: word <word>..."));
        cli.println(F("  d: dump <address> <length>"));
        cli.print(F("  m: memory <address> <byte>..."));
    }
    cli.println();
    prompt();
}

void setup() {
    Serial.begin(9600);
    cli.begin(Serial);
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
