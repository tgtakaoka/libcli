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

static size_t outHex(Stream &console, uint8_t val8) {
    val8 &= 0x0f;
    val8 += (val8 < 10) ? '0' : ('A' - 10);
    return console.print(static_cast<char>(val8));
}

static size_t outHex8(Stream &console, uint8_t val8) {
    const size_t n = outHex(console, val8 >> 4);
    return outHex(console, val8) + n;
}

size_t Cli::printHex8(uint8_t val8) {
    return outHex8(*_console, val8);
}

size_t Cli::printHex16(uint16_t val16) {
    const size_t n = printHex8(val16 >> 8);
    return printHex8(val16) + n;
}

size_t Cli::printHex20(uint32_t val20) {
    const size_t n = outHex(*_console, val20 >> 16);
    return printHex16(val20) + n;
}

size_t Cli::printHex24(uint32_t val24) {
    const size_t n = printHex8(val24 >> 16);
    return printHex16(val24) + n;
}

size_t Cli::printHex32(uint32_t val32) {
    const size_t n = printHex16(val32 >> 16);
    return printHex16(val32) + n;
}

size_t Cli::printDec8(uint8_t val8) {
    return _console->print(val8, DEC);
}

size_t Cli::printDec16(uint16_t val16) {
    return _console->print(val16, DEC);
}

size_t Cli::printDec32(uint32_t val32) {
    return _console->print(val32, DEC);
}

size_t Cli::backspace(int8_t n) {
    size_t s = 0;
    while (n-- > 0)
        s += print(F("\b \b"));
    return s;
}

void Cli::readHex8(ValueHandler handler, uintptr_t extra) {
    readHex(handler, extra, 8);
}

void Cli::readHex16(ValueHandler handler, uintptr_t extra) {
    readHex(handler, extra, 16);
}

void Cli::readHex32(ValueHandler handler, uintptr_t extra) {
    readHex(handler, extra, 32);
}

void Cli::readHex8(ValueHandler handler, uintptr_t extra, uint8_t val8) {
    readHex(handler, extra, -8, val8);
}

void Cli::readHex16(ValueHandler handler, uintptr_t extra, uint16_t val16) {
    readHex(handler, extra, -16, val16);
}

void Cli::readHex32(ValueHandler handler, uintptr_t extra, uint32_t val32) {
    readHex(handler, extra, -32, val32);
}

void Cli::readDec8(ValueHandler handler, uintptr_t extra) {
    readDec(handler, extra, 8);
}

void Cli::readDec16(ValueHandler handler, uintptr_t extra) {
    readDec(handler, extra, 16);
}

void Cli::readDec32(ValueHandler handler, uintptr_t extra) {
    readDec(handler, extra, 32);
}

void Cli::readDec8(ValueHandler handler, uintptr_t extra, uint8_t val8) {
    readDec(handler, extra, -8, val8);
}

void Cli::readDec16(ValueHandler handler, uintptr_t extra, uint16_t val16) {
    readDec(handler, extra, -16, val16);
}

void Cli::readDec32(ValueHandler handler, uintptr_t extra, uint32_t val32) {
    readDec(handler, extra, -32, val32);
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

void Cli::readHex(ValueHandler handler, uintptr_t extra, int8_t bits, uint32_t defval) {
    _processor = &Cli::processHex;
    _handler.value = handler;
    _extra = extra;
    if (bits >= 0) {
        _valueLen = 0;
        _valueBits = Bits(bits);
        _value = 0;
    } else {
        backspace(-bits / 4);
        setHex(Bits(-bits), defval);
    }
}

void Cli::setHex(Bits bits, uint32_t defval) {
    _valueLen = int8_t(bits) / 4;
    _valueBits = bits;
    _value = defval;
    switch (bits) {
    case BITS8:
        printHex8(defval);
        break;
    case BITS16:
        printHex16(defval);
        break;
    default:
        printHex32(defval);
        break;
    }
}

static uint8_t decDigits(uint32_t value) {
    if (value < 10)
        return 1;
    if (value < 100)
        return 2;
    if (value < 1000)
        return 3;
    if (value < 10000)
        return 4;
    if (value < 100000L)
        return 5;
    if (value < 1000000L)
        return 6;
    if (value < 10000000L)
        return 7;
    if (value < 100000000L)
        return 8;
    if (value < 1000000000L)
        return 9;
    return 10;
}

void Cli::readDec(ValueHandler handler, uintptr_t extra, int8_t bits, uint32_t defval) {
    _processor = &Cli::processDec;
    _handler.value = handler;
    _extra = extra;
    if (bits >= 0) {
        _valueLen = 0;
        _valueBits = Bits(bits);
        _value = 0;
    } else {
        _valueLen = decDigits(defval);
        _valueBits = Bits(-bits);
        _value = defval;
        backspace(_valueLen);
        printDec32(defval);
    }
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

void Cli::processHex(char c) {
    if (isHexadecimalDigit(c) && _valueLen < int8_t(_valueBits) / 4) {
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
        setHex(_valueBits, _value);
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

bool Cli::acceptDec(char c) const {
    if (isDigit(c)) {
        const uint8_t n = c - '0';
        const uint32_t v = _value;
        switch (_valueBits) {
        case BITS8:
            return v < 25 || (v == 25 && n < 6);
        case BITS16:
            return v < 6553 || (v == 6553 && n < 6);
        default:
            return v < 429496729L || (v == 429496729L && n < 6);
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
    } else if (isNewline(c) && _valueLen > 0) {
        println();
        state = CLI_NEWLINE;
    } else if (isSpace(c) && _valueLen > 0) {
        print(' ');
        state = CLI_SPACE;
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
    if (_console->available()) {
        (this->*_processor)(_console->read());
    }
}

}  // namespace libcli

// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=4:
