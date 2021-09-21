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
#include "libcli.h"

using namespace libcli;

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

/** Returns number of hexadecimal digits of |value|. */
static uint8_t hexDigits(uint32_t value) {
    uint8_t n = 0;
    do {
        n++;
        value >>= 4;
    } while (value);
    return n;
}

size_t Impl::printHex(uint32_t value, uint8_t width) {
    for (uint8_t n = hexDigits(value); n < width; n++) {
        console->print('0');
    }
    console->print(value, HEX);
    return width;
}

/** Returns number of decimal digits of |value|. */
static uint8_t decDigits(uint32_t value) {
    uint8_t n = 0;
    do {
        n++;
        value /= 10;
    } while (value);
    return n;
}

size_t Impl::printDec(uint32_t value, uint8_t width) {
    for (uint8_t n = decDigits(value); n < width; n++) {
        console->print(' ');
    }
    console->print(value, DEC);
    return width;
}

size_t Impl::backspace(int8_t n) {
    size_t s = 0;
    while (n-- > 0)
        s += console->print(F("\b \b"));
    return s;
}

void Impl::setHexHandler(Cli::ValueHandler handler, uintptr_t extra, uint8_t digits) {
    this->handler.value = handler;
    setProcessor(&Impl::processHex, extra);
    setHex(digits, 0);
    val_len = 0;
}

void Impl::setHexHandler(
        Cli::ValueHandler handler, uintptr_t extra, uint8_t digits, uint32_t defval) {
    this->handler.value = handler;
    setProcessor(&Impl::processHex, extra);
    setHex(digits, defval);
    backspace(val_width);
    printHex(defval, val_width);
}

void Impl::setDecHandler(Cli::ValueHandler handler, uintptr_t extra, uint32_t max) {
    this->handler.value = handler;
    setProcessor(&impl::Impl::processDec, extra);
    setDec(max, 0);
    val_len = 0;
}

void Impl::setDecHandler(
        Cli::ValueHandler handler, uintptr_t extra, uint32_t max, uint32_t defval) {
    this->handler.value = handler;
    setProcessor(&impl::Impl::processDec, extra);
    setDec(max, defval);
    backspace(val_width);
    printDec(defval, val_width);
}

void Impl::setHex(uint8_t digits, uint32_t value) {
    if (digits >= 8)
        digits = 8;
    val_width = val_len = digits;
    val_max = (1L << digits * 4);  // 0 if digits is 8.
    val = value;
}

void Impl::setDec(uint32_t max, uint32_t value) {
    val_width = val_len = decDigits(max);
    val_max = max + 1;
    val = value;
}

void Impl::processString(char c) {
    if (isNewline(c)) {
        console->println();
        handler.string(str_buf, extra, Cli::State::CLI_NEWLINE);
    } else if (isBackspace(c)) {
        if (str_len > 0) {
            str_buf[--str_len] = 0;
            backspace();
        }
    } else if (isCancel(c)) {
        console->println(F(" cancel"));
        handler.string(str_buf, extra, Cli::State::CLI_CANCEL);
    } else if (str_len < sizeof(str_buf) - 1) {
        str_buf[str_len++] = c;
        str_buf[str_len] = 0;
        console->print(c);
    }
}

bool Impl::acceptValue(char c, uint8_t base) const {
    if (base == 16) {
        return isHexadecimalDigit(c) && val_len < val_width;
    }
    if (isDigit(c)) {
        const uint32_t limit = (val_max == 0) ? 429496729L : val_max / 10;
        const uint8_t n = c - '0';
        return val < limit || (val == limit && n < (val_max % 10));
    }
    return false;
}

void Impl::processHex(char c) {
    processValue(c, 16);
}

void Impl::processDec(char c) {
    processValue(c, 10);
}

void Impl::processValue(char c, uint8_t base) {
    if (acceptValue(c, base)) {
        c = toUpperCase(c);
        console->print(c);
        val_len++;
        val *= base;
        val += isDigit(c) ? c - '0' : c - 'A' + 10;
        return;
    }

    Cli::State state;
    if (isBackspace(c)) {
        if (val_len > 0) {
            backspace();
            val /= base;
            val_len--;
            return;
        }
        state = Cli::State::CLI_DELETE;
    } else if (isSpace(c) && val_len > 0) {
        backspace(val_len);
        if (base == 10) {
            setDec(val_max, val);
            printDec(val, val_width);
        } else {
            setHex(val_width, val);
            printHex(val, val_width);
        }
        if (isNewline(c)) {
            console->println();
            state = Cli::State::CLI_NEWLINE;
        } else {
            console->print(' ');
            state = Cli::State::CLI_SPACE;
        }
    } else if (isCancel(c)) {
        console->println(F(" cancel"));
        state = Cli::State::CLI_CANCEL;
    } else {
        return;
    }
    handler.value(val, extra, state);
}

}  // namespace impl
}  // namespace libcli

// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=4:
