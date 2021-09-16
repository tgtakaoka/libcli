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

#include "libcli.h"

#include <stdint.h>

namespace libcli {

struct Cli::Impl {
    Stream *console;

    void (Cli::Impl::*_processor)(char c);
    union {
        LetterHandler letter;
        StringHandler string;
        ValueHandler value;
    } _handler;
    uintptr_t _extra;

    uint8_t _valueLen;
    uint8_t _valueWidth;
    uint32_t _value;
    uint32_t _valueMax;

    uint8_t _stringLen;
    char _string[80];

    void processNop(char c) { (void)c; }
    void processLetter(char c);
    void processString(char c);
    void processHex(char c);
    void processDec(char c);
    void processValue(char c, uint8_t base);

    void setHex(uint8_t digits, uint32_t value);
    void setDec(uint32_t max, uint32_t value);
    bool acceptValue(char c, uint8_t base) const;

    size_t backspace(int8_t n = 1);
    size_t printHex(uint32_t value, uint8_t width);
    size_t printDec(uint32_t value, uint8_t width);
};

/**
 * Global variables.
 */
class Cli Cli;
static Cli::Impl impl;

/** Default constructor. */
Cli::Cli() : _impl(impl) {}

/**
 * Virual methods of Print class.
 */

size_t Cli::write(uint8_t val) {
    return _impl.console->write(val);
}

size_t Cli::write(const uint8_t *buffer, size_t size) {
    return _impl.console->write(buffer, size);
}

int Cli::availableForWrite() {
    return _impl.console->availableForWrite();
}

/**
 * Virtual methods of Stream class.
 */

int Cli::available() {
    return _impl.console->available();
}

int Cli::read() {
    return _impl.console->read();
}

int Cli::peek() {
    return _impl.console->peek();
}

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

size_t Cli::printHex(uint32_t value, uint8_t width) {
    return _impl.printHex(value, width);
}

size_t Cli::Impl::printHex(uint32_t value, uint8_t width) {
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

size_t Cli::printDec(uint32_t value, uint8_t width) {
    return _impl.printDec(value, width);
}

size_t Cli::Impl::printDec(uint32_t value, uint8_t width) {
    for (uint8_t n = decDigits(value); n < width; n++) {
        console->print(' ');
    }
    console->print(value, DEC);
    return width;
}

size_t Cli::backspace(int8_t n) {
    return _impl.backspace(n);
}

size_t Cli::Impl::backspace(int8_t n) {
    size_t s = 0;
    while (n-- > 0)
        s += console->print(F("\b \b"));
    return s;
}

void Cli::readLetter(LetterHandler handler, uintptr_t extra) {
    _impl._processor = &Cli::Impl::processLetter;
    _impl._handler.letter = handler;
    _impl._extra = extra;
}

void Cli::readString(StringHandler handler, uintptr_t extra) {
    _impl._processor = &Cli::Impl::processString;
    _impl._handler.string = handler;
    _impl._extra = extra;
    _impl._stringLen = 0;
    *_impl._string = 0;
}

void Cli::readHex(ValueHandler handler, uintptr_t extra, uint8_t digits) {
    _impl._processor = &Cli::Impl::processHex;
    _impl._handler.value = handler;
    _impl._extra = extra;
    _impl.setHex(digits, 0);
    _impl._valueLen = 0;
}

void Cli::readHex(ValueHandler handler, uintptr_t extra, uint8_t digits, uint32_t defval) {
    _impl._processor = &Cli::Impl::processHex;
    _impl._handler.value = handler;
    _impl._extra = extra;
    _impl.setHex(digits, defval);
    backspace(_impl._valueWidth);
    printHex(defval, _impl._valueWidth);
}

void Cli::Impl::setHex(uint8_t digits, uint32_t defval) {
    if (digits >= 8)
        digits = 8;
    _valueWidth = _valueLen = digits;
    _valueMax = (1L << digits * 4);  // 0 if digits is 8.
    _value = defval;
}

void Cli::readDec(ValueHandler handler, uintptr_t extra, uint32_t limit) {
    _impl._processor = &Cli::Impl::processDec;
    _impl._handler.value = handler;
    _impl._extra = extra;
    _impl.setDec(limit, 0);
    _impl._valueLen = 0;
}

void Cli::readDec(ValueHandler handler, uintptr_t extra, uint32_t limit, uint32_t defval) {
    _impl._processor = &Cli::Impl::processDec;
    _impl._handler.value = handler;
    _impl._extra = extra;
    _impl.setDec(limit, defval);
    backspace(_impl._valueWidth);
    printDec(defval, _impl._valueWidth);
}

void Cli::Impl::setDec(uint32_t limit, uint32_t defval) {
    _valueWidth = _valueLen = decDigits(limit);
    _valueMax = limit + 1;
    _value = defval;
}

void Cli::Impl::processLetter(char c) {
    _handler.letter(c, _extra);
}

void Cli::Impl::processString(char c) {
    if (isNewline(c)) {
        console->println();
        _handler.string(_string, _extra, CLI_NEWLINE);
    } else if (isBackspace(c)) {
        if (_stringLen > 0) {
            backspace();
            _string[--_stringLen] = 0;
        }
    } else if (isCancel(c)) {
        console->println(F(" cancel"));
        _handler.string(_string, _extra, CLI_CANCEL);
    } else if (_stringLen < sizeof(_string) - 1) {
        console->print(c);
        _string[_stringLen++] = c;
        _string[_stringLen] = 0;
    }
}

bool Cli::Impl::acceptValue(char c, uint8_t base) const {
    if (base == 16) {
        return isHexadecimalDigit(c) && _valueLen < _valueWidth;
    }
    if (isDigit(c)) {
        const uint32_t limit = (_valueMax == 0) ? 429496729L : _valueMax / 10;
        const uint8_t n = c - '0';
        return _value < limit || (_value == limit && n < (_valueMax % 10));
    }
    return false;
}

void Cli::Impl::processHex(char c) {
    processValue(c, 16);
}

void Cli::Impl::processDec(char c) {
    processValue(c, 10);
}

void Cli::Impl::processValue(char c, uint8_t base) {
    if (acceptValue(c, base)) {
        c = toUpperCase(c);
        console->print(c);
        _valueLen++;
        _value *= base;
        _value += isDigit(c) ? c - '0' : c - 'A' + 10;
        return;
    }

    State state;
    if (isBackspace(c)) {
        if (_valueLen > 0) {
            backspace();
            _value /= base;
            _valueLen--;
            return;
        }
        state = CLI_DELETE;
    } else if (isSpace(c) && _valueLen > 0) {
        backspace(_valueLen);
        if (base == 10) {
            setDec(_valueMax - 1, _value);
            printDec(_value, _valueWidth);
        } else {
            setHex(_valueWidth, _value);
            printHex(_value, _valueWidth);
        }
        if (isNewline(c)) {
            console->println();
            state = CLI_NEWLINE;
        } else {
            console->print(' ');
            state = CLI_SPACE;
        }
    } else if (isCancel(c)) {
        console->println(F(" cancel"));
        state = CLI_CANCEL;
    } else {
        return;
    }
    _handler.value(_value, _extra, state);
}

void Cli::begin(Stream &console) {
    _impl.console = &console;
    _impl._processor = &Cli::Impl::processNop;
}

void Cli::loop() {
    if (available()) {
        (_impl.*_impl._processor)(read());
    }
}

}  // namespace libcli

// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=4:
