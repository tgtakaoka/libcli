/*
 * Copyright 2021 Tadashi G. Takaoka
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

#include "libcli_impl.h"

#include <stdint.h>
#include <string.h>
#include "libcli.h"

using namespace libcli;
typedef Cli::State State;

namespace libcli {

namespace impl {

/** Singleton */
Impl Impl::impl;

static bool isBackspace(char c) {
    return c == '\b' || c == '\x7f';
}

static bool isCancel(char c) {
    return c == '\x03';
}

static bool isNewline(char c) {
    return c == '\n' || c == '\r';
}

/** Returns number of digits of |number| in |base|. */
static uint8_t getDigits(uint32_t number, uint8_t base) {
    uint8_t n = 0;
    do {
        n++;
        number /= base;
    } while (number);
    return n;
}

size_t Impl::printHex(uint32_t number, uint8_t width) {
    for (uint8_t n = getDigits(number, 16); n < width; n++) {
        console->print('0');
    }
    console->print(number, HEX);
    return width;
}

size_t Impl::printDec(uint32_t number, uint8_t width) {
    for (uint8_t n = getDigits(number, 10); n < width; n++) {
        console->print(' ');
    }
    console->print(number, DEC);
    return width;
}

size_t Impl::backspace(int8_t n) {
    size_t s = 0;
    while (n--)
        s += console->print(F("\b \b"));
    return s;
}

void Impl::setCallback(Cli::LetterCallback callback, uintptr_t context) {
    this->callback.letter = callback;
    setProcessor(&Impl::processLetter, context);
}

void Impl::processLetter(char c) {
    callback.letter(c, context);
}

void Impl::setCallback(
        Cli::StringCallback callback, uintptr_t context, bool word, const char *defval) {
    this->callback.string = callback;
    if (defval) {
        strncpy(str_buf, defval, sizeof(str_buf) - 1);
    } else {
        str_buf[0] = 0;
    }
    str_len = strlen(str_buf);
    str_word = word;
    setProcessor(&Impl::processString, context);
}

void Impl::processString(char c) {
    if (isNewline(c)) {
        if (str_len || !str_word) {  // can't accept empty word
            console->println();
            callback.string(str_buf, context, State::CLI_NEWLINE);
        }
    } else if (isSpace(c) && str_word) {
        if (str_len) {  // can't accept leading spaces in word
            console->print(c);
            callback.string(str_buf, context, State::CLI_SPACE);
        }
    } else if (isBackspace(c)) {
        if (str_len) {
            str_buf[--str_len] = 0;
            backspace();
        } else if (str_word) {
            callback.string(str_buf, context, State::CLI_DELETE);
        }
    } else if (isCancel(c)) {
        console->println(F(" cancel"));
        callback.string(str_buf, context, State::CLI_CANCEL);
    } else if (str_len < sizeof(str_buf) - 1) {
        str_buf[str_len++] = c;
        str_buf[str_len] = 0;
        console->print(c);
    }
}

void Impl::setCallback(
        Cli::NumberCallback callback, uint32_t context, uint32_t limit, uint8_t base) {
    this->callback.number = callback;
    num_width = getDigits(num_limit = limit, num_base = base);
    num_value = 0;
    num_len = 0;
    setProcessor(&Impl::processNumber, context);
}

void Impl::setCallback(Cli::NumberCallback callback, uint32_t context, uint32_t limit,
        uint32_t defval, uint8_t base) {
    setCallback(callback, context, limit, base);
    backspace(num_width);
    num_value = defval;
    num_len = num_width;
    if (base == 10) {
        printDec(num_value, num_len);
    } else {
        printHex(num_value, num_len);
    }
}

bool Impl::checkLimit(char c, uint8_t &n) const {
    c = toUpperCase(c);
    if (num_base == 10 && isDigit(c)) {
        n = c - '0';
        const uint32_t limit = num_limit / 10;
        return num_value < limit || (num_value == limit && n <= (num_limit % 10));
    }
    if (num_base == 16 && isHexadecimalDigit(c)) {
        n = isDigit(c) ? c - '0' : c - 'A' + 10;
        const uint32_t limit = num_limit / 16;
        return num_value < limit || (num_value == limit && n <= (num_limit % 16));
    }
    return false;
}

void Impl::processNumber(char c) {
    uint8_t n;
    if (checkLimit(c, n)) {
        num_value *= num_base;
        num_value += n;
        num_len++;
        console->print(c);
        return;
    }

    State state;
    if (isBackspace(c)) {
        if (num_len) {
            num_value /= num_base;
            num_len--;
            backspace();
            return;
        }
        state = State::CLI_DELETE;
    } else if (isSpace(c) && num_len) {
        backspace(num_len);
        num_len = num_width;
        if (num_base == 10) {
            printDec(num_value, num_len);
        } else {
            printHex(num_value, num_len);
        }
        if (isNewline(c)) {
            console->println();
            state = State::CLI_NEWLINE;
        } else {
            console->print(c);
            state = State::CLI_SPACE;
        }
    } else if (isCancel(c)) {
        console->println(F(" cancel"));
        state = State::CLI_CANCEL;
    } else {
        return;
    }
    callback.number(num_value, context, state);
}

}  // namespace impl
}  // namespace libcli

// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=4:
