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

class Cli Cli;

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

size_t Cli::write(uint8_t val) {
    return _console->write(val);
}

size_t Cli::write(const uint8_t *buffer, size_t size) {
    return _console->write(buffer, size);
}

int Cli::availableForWrite() {
    return _console->availableForWrite();
}

/**
 * Virtual methods of Stream class.
 */

int Cli::available() {
    return _console->available();
}

int Cli::read() {
    return _console->read();
}

int Cli::peek() {
    return _console->peek();
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
    for (uint8_t n = hexDigits(value); n < width; n++) {
        print('0');
    }
    print(value, HEX);
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
    for (uint8_t n = decDigits(value); n < width; n++) {
        print(' ');
    }
    print(value, DEC);
    return width;
}

size_t Cli::backspace(int8_t n) {
    size_t s = 0;
    while (n-- > 0)
        s += print(F("\b \b"));
    return s;
}

void Cli::readLetter(LetterHandler handler, uintptr_t extra) {
    _processor = &Cli::processLetter;
    _handler.letter = handler;
    _extra = extra;
}

void Cli::readString(StringHandler handler, uintptr_t extra) {
    _processor = &Cli::processString;
    _handler.string = handler;
    _extra = extra;
    _stringLen = 0;
    *_string = 0;
}

void Cli::readHex(ValueHandler handler, uintptr_t extra, uint8_t digits) {
    _processor = &Cli::processHex;
    _handler.value = handler;
    _extra = extra;
    setHex(digits, 0);
    _valueLen = 0;
}

void Cli::readHex(ValueHandler handler, uintptr_t extra, uint8_t digits, uint32_t defval) {
    _processor = &Cli::processHex;
    _handler.value = handler;
    _extra = extra;
    setHex(digits, defval);
    backspace(_valueWidth);
    printHex(defval, _valueWidth);
}

void Cli::setHex(uint8_t digits, uint32_t value) {
    if (digits >= 8)
        digits = 8;
    _valueWidth = _valueLen = digits;
    _valueMax = (1L << digits * 4);  // 0 if digits is 8.
    _value = value;
}

void Cli::readDec(ValueHandler handler, uintptr_t extra, uint32_t limit) {
    _processor = &Cli::processDec;
    _handler.value = handler;
    _extra = extra;
    setDec(limit, 0);
    _valueLen = 0;
}

void Cli::readDec(ValueHandler handler, uintptr_t extra, uint32_t limit, uint32_t defval) {
    _processor = &Cli::processDec;
    _handler.value = handler;
    _extra = extra;
    setDec(limit, defval);
    backspace(_valueWidth);
    printDec(defval, _valueWidth);
}

void Cli::setDec(uint32_t limit, uint32_t defval) {
    _valueWidth = _valueLen = decDigits(limit);
    _valueMax = limit + 1;
    _value = defval;
}

void Cli::processLetter(char c) {
    _handler.letter(c, _extra);
}

void Cli::processString(char c) {
    if (isNewline(c)) {
        println();
        _handler.string(_string, _extra, CLI_NEWLINE);
    } else if (isBackspace(c)) {
        if (_stringLen > 0) {
            backspace();
            _string[--_stringLen] = 0;
        }
    } else if (isCancel(c)) {
        println(F(" cancel"));
        _handler.string(_string, _extra, CLI_CANCEL);
    } else if (_stringLen < sizeof(_string) - 1) {
        print(c);
        _string[_stringLen++] = c;
        _string[_stringLen] = 0;
    }
}

bool Cli::acceptHex(char c) const {
    if (isHexadecimalDigit(c)) {
        return _valueLen < _valueWidth;
    }
    return false;
}

void Cli::processHex(char c) {
    if (acceptHex(c)) {
        c = toUpperCase(c);
        print(c);
        _valueLen++;
        _value *= 16;
        _value += isDigit(c) ? c - '0' : c - 'A' + 10;
        return;
    }

    State state;
    if (isBackspace(c)) {
        if (_valueLen > 0) {
            backspace();
            _value /= 16;
            _valueLen--;
            return;
        }
        state = CLI_DELETE;
    } else if (isSpace(c) && _valueLen > 0) {
        backspace(_valueLen);
        setHex(_valueWidth, _value);
        printHex(_value, _valueWidth);
        if (isNewline(c)) {
            println();
            state = CLI_NEWLINE;
        } else {
            print(' ');
            state = CLI_SPACE;
        }
    } else if (isCancel(c)) {
        println(F(" cancel"));
        state = CLI_CANCEL;
    } else {
        return;
    }
    _handler.value(_value, _extra, state);
}

static uint32_t acceptDecLimit(uint32_t max) {
    if (max == 0)
        return 429496729L;  // (1L << 32) ? 10;
    return max / 10;
}

bool Cli::acceptDec(char c) const {
    if (isDigit(c)) {
        const uint32_t limit = acceptDecLimit(_valueMax);
        if (_value < limit) {
            return true;
        } else if (_value == limit) {
            const uint8_t n = c - '0';
            return n < _valueMax % 10;
        } else {
            return false;
        }
    }
    return false;
}

void Cli::processDec(char c) {
    if (acceptDec(c)) {
        print(c);
        _valueLen++;
        _value *= 10;
        _value += c - '0';
        return;
    }

    State state;
    if (isBackspace(c)) {
        if (_valueLen > 0) {
            backspace();
            _value /= 10;
            _valueLen--;
            return;
        }
        state = CLI_DELETE;
    } else if (isSpace(c) && _valueLen > 0) {
        backspace(_valueLen);
        setDec(_valueMax - 1, _value);
        printDec(_value, _valueWidth);
        if (isNewline(c)) {
            println();
            state = CLI_NEWLINE;
        } else {
            print(' ');
            state = CLI_SPACE;
        }
    } else if (isCancel(c)) {
        println(F(" cancel"));
        state = CLI_CANCEL;
    } else {
        return;
    }
    _handler.value(_value, _extra, state);
}

void Cli::begin(Stream &console) {
    _console = &console;
    _processor = &Cli::processNop;
}

void Cli::loop() {
    if (available()) {
        (this->*_processor)(read());
    }
}

}  // namespace libcli

// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=4:
