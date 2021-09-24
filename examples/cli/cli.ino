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
libcli::Cli &Cli = libcli::Cli::instance();
typedef libcli::Cli::State State;

static void handleCommand(char letter, uintptr_t context);

/** print prompt and request letter input */
static void prompt() {
    Cli.print(F("> "));
    Cli.readLetter(handleCommand, 0);
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
        Cli.backspace();
        Cli.readDec(handleAddDec, ADD_LEFT, DEC_LIMIT, last_num);
        return;
    }
    if (context == ADD_LEFT) {
        last_num = value;
        if (state == State::CLI_SPACE) {
            Cli.readDec(handleAddDec, ADD_RIGHT, DEC_LIMIT);
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
        Cli.backspace();
        Cli.readHex(handleAddHex, ADD_LEFT, HEX_LIMIT, last_num);
        return;
    }
    if (context == ADD_LEFT) {
        last_num = value;
        if (state == State::CLI_SPACE) {
            Cli.readHex(handleAddHex, ADD_RIGHT, HEX_LIMIT);
            return;
        }
        value = 1;
    }
    if (state == State::CLI_SPACE)
        Cli.println();
    Cli.print(F("add hexadecimal: "));
    Cli.printHex(last_num, HEX_WIDTH);
    Cli.print(F(" + "));
    Cli.printHex(value, HEX_WIDTH);
    Cli.print(F(" = "));
    Cli.printHex(last_num + value, HEX_WIDTH);
    Cli.println();
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
        Cli.backspace();
        Cli.readHex(handleDump, DUMP_ADDRESS, DUMP_ADDR_LIMIT, last_addr);
        return;
    }
    if (context == DUMP_ADDRESS) {
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
    Cli.printHex(last_addr, DUMP_ADDR_WIDTH);
    Cli.print(' ');
    Cli.printDec(value);
    Cli.println();
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
        Cli.backspace();
        if (index == 0) {
            Cli.readHex(handleMemory, MEMORY_ADDRESS, MEMORY_ADDR_LIMIT, last_addr);
            return;
        }
        index--;
        Cli.readHex(handleMemory, MEMORY_INDEX(index), UINT8_MAX, mem_buffer[index]);
        return;
    }
    if (context == MEMORY_ADDRESS) {
        last_addr = value;
        Cli.readHex(handleMemory, MEMORY_INDEX(0), UINT8_MAX);
        return;
    }

    mem_buffer[index++] = value;
    if (state == State::CLI_SPACE) {
        if (index < sizeof(mem_buffer)) {
            Cli.readHex(handleMemory, MEMORY_INDEX(index), UINT8_MAX);
            return;
        }
        Cli.println();
    }
    Cli.print(F("write memory: "));
    Cli.printHex(last_addr, MEMORY_ADDR_WIDTH);
    for (uint8_t i = 0; i < index; i++) {
        Cli.print(' ');
        Cli.printHex(mem_buffer[i], 2);
    }
    Cli.println();
    prompt();
}

/** callback for readLine */
static void handleLoad(const char *line, uintptr_t context, State state) {
    (void)context;
    if (state != State::CLI_CANCEL) {
        Cli.print(F("load file: '"));
        Cli.print(line);
        Cli.println('\'');
    }
    prompt();
}

/** callback for readWord */
static constexpr size_t WORD_LEN = 10;

static void handleWord(const char *word, uintptr_t context, State state) {
    static char words[4][WORD_LEN + 1];
    static constexpr size_t WORD_MAX = sizeof(words) / sizeof(words[0]);

    if (state == State::CLI_CANCEL) {
        prompt();
        return;
    }
    size_t index = context;
    if (state == State::CLI_DELETE) {
        if (index != 0) {
            Cli.backspace();
            index--;
            Cli.readWord(handleWord, index, words[index]);
        }
        return;
    }

    strncpy(words[index++], word, WORD_LEN);
    if (state == State::CLI_SPACE) {
        if (index < WORD_MAX) {
            Cli.readWord(handleWord, index);
            return;
        }
        Cli.println();
    }

    for (size_t i = 0; i < index; i++) {
        Cli.print(F("  word "));
        Cli.printDec(i + 1);
        Cli.print(F(": '"));
        Cli.print(words[i]);
        Cli.println('\'');
    }
    prompt();
}

/** callback for readLetter */
static void handleCommand(char letter, uintptr_t context) {
    if (letter == 's') {
        Cli.print(F("step"));
    }
    if (letter == 'a') {
        Cli.print(F("add decimal "));
        Cli.readDec(handleAddDec, ADD_LEFT, DEC_LIMIT);
        return;
    }
    if (letter == 'h') {
        Cli.print(F("add hexadecimal "));
        Cli.readHex(handleAddHex, ADD_LEFT, HEX_LIMIT);
        return;
    }
    if (letter == 'l') {
        Cli.print(F("load "));
        Cli.readLine(handleLoad, 0);
        return;
    }
    if (letter == 'w') {
        Cli.print(F("word "));
        Cli.readWord(handleWord, 0);
        return;
    }
    if (letter == 'd') {
        Cli.print(F("dump "));
        Cli.readHex(handleDump, DUMP_ADDRESS, DUMP_ADDR_LIMIT);
        return;
    }
    if (letter == 'm') {
        Cli.print(F("memory "));
        Cli.readHex(handleMemory, MEMORY_ADDRESS, MEMORY_ADDR_LIMIT);
        return;
    }
    if (letter == '?') {
        Cli.print(F("libcli (version "));
        Cli.print(LIBCLI_VERSION_STRING);
        Cli.println(F(") example"));
        Cli.println(F("  ?: help"));
        Cli.println(F("  s: step"));
        Cli.println(F("  a: add decimal"));
        Cli.println(F("  h: add hexadecimal"));
        Cli.println(F("  l: load <filename>"));
        Cli.println(F("  w: word <word>..."));
        Cli.println(F("  d: dump <address> <length>"));
        Cli.print(F("  m: memory <address> <byte>..."));
    }
    Cli.println();
    prompt();
}

void setup() {
    Serial.begin(9600);
    Cli.begin(Serial);
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
