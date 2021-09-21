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
Impl Impl::_impl;

static bool isBackspace(char c) {
    return c == '\b' || c == '\x7f';
}

static bool isCancel(char c) {
    return c == '\x03';
}

static bool isNewline(char c) {
    return c == '\n' || c == '\r';
}

/** Returns number of digits of |value| in |base|. */
static uint8_t getDigits(uint32_t value, uint8_t base) {
    uint8_t n = 0;
    do {
        n++;
        value /= base;
    } while (value);
    return n;
}

size_t Impl::printHex(uint32_t value, uint8_t width) {
    for (uint8_t n = getDigits(value, 16); n < width; n++) {
        console->print('0');
    }
    console->print(value, HEX);
    return width;
}

size_t Impl::printDec(uint32_t value, uint8_t width) {
    for (uint8_t n = getDigits(value, 10); n < width; n++) {
        console->print(' ');
    }
    console->print(value, DEC);
    return width;
}

size_t Impl::backspace(int8_t n) {
    size_t s = 0;
    while (n--)
        s += console->print(F("\b \b"));
    return s;
}

void Impl::setHandler(Cli::LetterHandler handler, uintptr_t extra) {
    this->handler.letter = handler;
    setProcessor(&Impl::processLetter, extra);
}

void Impl::processLetter(char c) {
    handler.letter(c, extra);
}

void Impl::setHandler(Cli::StringHandler handler, uintptr_t extra, bool word, const char *defval) {
    this->handler.string = handler;
    if (defval) {
        strncpy(str_buf, defval, sizeof(str_buf) - 1);
    } else {
        str_buf[0] = 0;
    }
    str_len = strlen(str_buf);
    str_word = word;
    setProcessor(&Impl::processString, extra);
}

void Impl::processString(char c) {
    if (isNewline(c)) {
        if (str_len || !str_word) { // can't accept empty word
            console->println();
            handler.string(str_buf, extra, State::CLI_NEWLINE);
        }
    } else if (isSpace(c) && str_word) {
        if (str_len) {          // can't accept leading spaces in word
            console->print(c);
            handler.string(str_buf, extra, State::CLI_SPACE);
        }
    } else if (isBackspace(c)) {
        if (str_len) {
            str_buf[--str_len] = 0;
            backspace();
        } else if (str_word) {
            handler.string(str_buf, extra, State::CLI_DELETE);
        }
    } else if (isCancel(c)) {
        console->println(F(" cancel"));
        handler.string(str_buf, extra, State::CLI_CANCEL);
    } else if (str_len < sizeof(str_buf) - 1) {
        str_buf[str_len++] = c;
        str_buf[str_len] = 0;
        console->print(c);
    }
}

void Impl::setHandler(Cli::ValueHandler handler, uint32_t extra, uint32_t limit, uint8_t base) {
    this->handler.value = handler;
    val_width = getDigits(val_limit = limit, val_base = base);
    val_value = 0;
    val_len = 0;
    setProcessor(&Impl::processValue, extra);
}

void Impl::setHandler(
        Cli::ValueHandler handler, uint32_t extra, uint32_t limit, uint32_t defval, uint8_t base) {
    backspace(val_width);
    setHandler(handler, extra, limit, base);
    val_value = defval;
    val_len = val_width;
    if (base == 10) {
        printDec(val_value, val_len);
    } else {
        printHex(val_value, val_len);
    }
}

bool Impl::checkLimit(char c, uint8_t &n) const {
    c = toUpperCase(c);
    if (val_base == 10 && isDigit(c)) {
        n = c - '0';
        const uint32_t limit = val_limit / 10;
        return val_value < limit || (val_value == limit && n <= (val_limit % 10));
    }
    if (val_base == 16 && isHexadecimalDigit(c)) {
        n = c - 'A' + 10;
        const uint32_t limit = val_limit / 16;
        return val_value < limit || (val_value == limit && n <= (val_limit % 16));
    }
    return false;
}

void Impl::processValue(char c) {
    uint8_t n;
    if (checkLimit(c, n)) {
        val_value *= val_base;
        val_value += n;
        val_len++;
        console->print(c);
        return;
    }

    State state;
    if (isBackspace(c)) {
        if (val_len) {
            val_value /= val_base;
            val_len--;
            backspace();
            return;
        }
        state = State::CLI_DELETE;
    } else if (isSpace(c) && val_len) {
        backspace(val_len);
        val_len = val_width;
        if (val_base == 10) {
            printDec(val_value, val_len);
        } else {
            printHex(val_value, val_len);
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
    handler.value(val_value, extra, state);
}

}  // namespace impl
}  // namespace libcli

// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=4:
