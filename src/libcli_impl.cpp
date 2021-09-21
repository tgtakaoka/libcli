
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

/**
 * Virual methods of Print class.
 */

size_t Impl::write(uint8_t val) {
    return console->write(val);
}

size_t Impl::write(const uint8_t *buffer, size_t size) {
    return console->write(buffer, size);
}

int Impl::availableForWrite() {
    return console->availableForWrite();
}

/**
 * Virtual methods of Stream class.
 */

int Impl::available() {
    return console->available();
}

int Impl::read() {
    return console->read();
}

int Impl::peek() {
    return console->peek();
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

void Impl::setHex(uint8_t digits, uint32_t value) {
    if (digits >= 8)
        digits = 8;
    _value.width = _value.len = digits;
    _value.max = (1L << digits * 4);  // 0 if digits is 8.
    _value.val = value;
}

void Impl::setDec(uint32_t max, uint32_t value) {
    _value.width = _value.len = decDigits(max);
    _value.max = max + 1;
    _value.val = value;
}

void Impl::processLetter(char c) {
    _proc.handler.letter(c, _proc.extra);
}

void Impl::processString(char c) {
    if (isNewline(c)) {
        console->println();
        _proc.handler.string(_string.buf, _proc.extra, Cli::State::CLI_NEWLINE);
    } else if (isBackspace(c)) {
        if (_string.len > 0) {
            backspace();
            _string.buf[--_string.len] = 0;
        }
    } else if (isCancel(c)) {
        console->println(F(" cancel"));
        _proc.handler.string(_string.buf, _proc.extra, Cli::State::CLI_CANCEL);
    } else if (_string.len < sizeof(_string.buf) - 1) {
        console->print(c);
        _string.buf[_string.len++] = c;
        _string.buf[_string.len] = 0;
    }
}

bool Impl::acceptValue(char c, uint8_t base) const {
    if (base == 16) {
        return isHexadecimalDigit(c) && _value.len < _value.width;
    }
    if (isDigit(c)) {
        const uint32_t limit = (_value.max == 0) ? 429496729L : _value.max / 10;
        const uint8_t n = c - '0';
        return _value.val < limit || (_value.val == limit && n < (_value.max % 10));
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
        _value.len++;
        _value.val *= base;
        _value.val += isDigit(c) ? c - '0' : c - 'A' + 10;
        return;
    }

    Cli::State state;
    if (isBackspace(c)) {
        if (_value.len > 0) {
            backspace();
            _value.val /= base;
            _value.len--;
            return;
        }
        state = Cli::State::CLI_DELETE;
    } else if (isSpace(c) && _value.len > 0) {
        backspace(_value.len);
        if (base == 10) {
            setDec(_value.max, _value.val);
            printDec(_value.val, _value.width);
        } else {
            setHex(_value.width, _value.val);
            printHex(_value.val, _value.width);
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
    _proc.handler.value(_value.val, _proc.extra, state);
}

}  // namespace impl
}  // namespace libcli

// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=4:
