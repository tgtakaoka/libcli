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

#include <string.h>

#include "libcli_impl.h"

namespace libcli {
namespace impl {

namespace {

bool isBackspace(char c) {
    return c == '\b' || c == '\x7f';
}

bool isCancel(char c) {
    return c == '\x03';
}

bool isNewline(char c) {
    return c == '\n' || c == '\r';
}

/** Returns number of digits of |number| in |radix|. */
int_fast8_t getDigits(uint32_t number, uint_fast8_t radix) {
    int_fast8_t n = 0;
    do {
        n++;
        number /= radix;
    } while (number);
    return n;
}

}  // namespace

size_t Impl::pad_left(int_fast8_t len, int_fast8_t width, char pad) {
    size_t size = 0;
    for (auto n = width - len; n > 0; n--)
        size += console->print(pad);
    return size;
}

size_t Impl::pad_right(int_fast8_t len, int_fast8_t width, char pad) {
    size_t size = 0;
    for (auto n = width + len; n < 0; n++)
        size += console->print(pad);
    return size;
}

size_t Impl::printNum(uint32_t number, int_fast8_t width, uint_fast8_t radix, bool newline) {
    const auto len = getDigits(number, radix);
    auto size = pad_left(len, width, radix == 10 ? ' ' : '0');
    size += console->print(number, radix);
    size += pad_right(len, width, ' ');
    if (newline)
        size += console->println();
    return size;
}

size_t Impl::printStr(const __FlashStringHelper *text, int_fast8_t width, bool newline) {
    const auto l = strlen_P(reinterpret_cast<const char *>(text));
    const auto len = (l < INT8_MAX) ? l : INT8_MAX;
    auto size = pad_left(len, width, ' ');
    size += console->print(text);
    size += pad_right(len, width, ' ');
    if (newline)
        size += console->println();
    return size;
}

size_t Impl::printStr(const char *text, int_fast8_t width, bool newline) {
    const auto l = strlen(text);
    const auto len = (l < INT8_MAX) ? l : INT8_MAX;
    auto size = pad_left(len, width, ' ');
    size += console->print(text);
    size += pad_right(len, width, ' ');
    if (newline)
        size += console->println();
    return size;
}

size_t Impl::backspace(int_fast8_t n) {
    size_t s = 0;
    while (n--)
        s += console->print(F("\b \b"));
    return s;
}

void Impl::setCallback(LetterCallback callback, uintptr_t context) {
    this->callback.letter = callback;
    setProcessor(&Impl::processLetter, context);
}

void Impl::processLetter(char c) {
    callback.letter(c, context);
}

void Impl::setCallback(StringCallback callback, uintptr_t context, char *buffer, size_t size,
        bool hasDefval, bool word) {
    this->callback.string = callback;
    str_buffer = buffer;
    str_limit = size - 1;
    if (hasDefval) {
        str_len = strlen(str_buffer);
    } else {
        str_buffer[str_len = 0] = 0;
    }
    str_word = word;
    setProcessor(&Impl::processString, context);
}

void Impl::processString(char c) {
    if (isNewline(c)) {
        console->print(' ');
        callback.string(str_buffer, context, CLI_NEWLINE);
    } else if (isSpace(c) && str_word) {
        if (str_len) {  // can't accept leading spaces in word
            console->print(c);
            callback.string(str_buffer, context, CLI_SPACE);
        }
    } else if (isBackspace(c)) {
        if (str_len) {
            str_buffer[--str_len] = 0;
            backspace(1);
        } else if (str_word) {
            callback.string(str_buffer, context, CLI_DELETE);
        }
    } else if (isCancel(c)) {
        console->println(F(" cancel"));
        callback.string(str_buffer, context, CLI_CANCEL);
    } else if (str_len < str_limit) {
        str_buffer[str_len++] = c;
        str_buffer[str_len] = 0;
        console->print(c);
    }
}

void Impl::setCallback(
        NumberCallback callback, uintptr_t context, uint_fast8_t radix, uint32_t limit) {
    this->callback.number = callback;
    num_width = getDigits(num_limit = limit, num_radix = radix);
    num_value = 0;
    num_len = 0;
    setProcessor(&Impl::processNumber, context);
}

void Impl::setCallback(NumberCallback callback, uintptr_t context, uint_fast8_t radix,
        uint32_t limit, uint32_t defval) {
    setCallback(callback, context, limit, radix);
    backspace(num_width);
    num_value = defval;
    num_len = num_width;
    printNum(num_value, num_len, radix, false);
}

bool Impl::checkLimit(char c, uint_fast8_t &n) const {
    if (num_len >= num_width)
        return false;
    c = toUpperCase(c);
    if (num_radix <= 10 && c >= '0' && c < num_radix + '0') {
        n = c - '0';
        const auto limit = num_limit / num_radix;
        return num_value < limit || (num_value == limit && n <= (num_limit % num_radix));
    }
    if (num_radix == 16 && isHexadecimalDigit(c)) {
        n = isDigit(c) ? c - '0' : c - 'A' + 10;
        const auto limit = num_limit / 16;
        return num_value < limit || (num_value == limit && n <= (num_limit % 16));
    }
    return false;
}

void Impl::processNumber(char c) {
    uint_fast8_t n;
    if (checkLimit(c, n)) {
        num_value *= num_radix;
        num_value += n;
        num_len++;
        console->print(c);
        return;
    }

    State state;
    if (isBackspace(c)) {
        if (num_len) {
            num_value /= num_radix;
            num_len--;
            backspace(1);
            return;
        }
        state = CLI_DELETE;
    } else if (isSpace(c) && num_len) {
        backspace(num_len);
        num_len = num_width;
        printNum(num_value, num_len, num_radix, false);
        if (isNewline(c)) {
            console->print(' ');
            state = CLI_NEWLINE;
        } else {
            console->print(c);
            state = CLI_SPACE;
        }
    } else if (isCancel(c)) {
        console->println(F(" cancel"));
        state = CLI_CANCEL;
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
