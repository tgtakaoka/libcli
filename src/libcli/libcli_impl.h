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

#ifndef __LIBCLI_IMPL_H__
#define __LIBCLI_IMPL_H__

#include <Arduino.h>

#include "libcli_types.h"

namespace libcli {

class Cli;

namespace impl {

/** Implementation detail of libcli. */
struct Impl final {
private:
    friend Cli;

    Impl() : console(nullptr), processor(&Impl::processNop), context(0) {}

    void begin(Stream &stream) { console = &stream; }
    void loop() {
        if (available())
            (this->*processor)(read());
    }

    void setCallback(LetterCallback callback, uintptr_t context);
    void setCallback(StringCallback callback, uintptr_t context, char *buffer, size_t size,
            bool hasDefval, bool word);
    void setCallback(NumberCallback callback, uintptr_t context, uint32_t limit, bool hex);
    void setCallback(NumberCallback callback, uintptr_t context, uint32_t limit,
            uint32_t defval, bool hex);

    size_t backspace(int8_t n);
    size_t printHex(uint32_t number, int8_t width, bool newline);
    size_t printDec(uint32_t number, int8_t width, bool newline);
    size_t printStr(const __FlashStringHelper *str, int8_t width, bool newline);
    size_t printStr(const char *str, int8_t width, bool newline);

    /** Delegate methods for Print. */
    size_t write(uint8_t val) { return console->write(val); }
    size_t write(const uint8_t *buf, size_t size) { return console->write(buf, size); }
    int availableForWrite() { return console->availableForWrite(); }

    /** Delegate methods for Stream. */
    int available() { return console->available(); }
    int read() { return console->read(); }
    int peek() { return console->peek(); }
    void flush() { return console->flush(); }

    using Processor = void (Impl::*)(char c);

    Stream *console;
    Processor processor;
    union {
        LetterCallback letter;
        StringCallback string;
        NumberCallback number;
    } callback;
    uintptr_t context;

    size_t str_limit;
    size_t str_len;
    bool str_word;
    char *str_buffer;

    uint32_t num_value;
    uint32_t num_limit;
    bool num_hex;
    uint8_t num_len;
    uint8_t num_width;

    void setProcessor(Processor processor_, uintptr_t context_) {
        processor = processor_;
        context = context_;
    }
    void processNop(char c) { (void)c; }
    void processLetter(char c);
    void processString(char c);
    void processNumber(char c);
    bool checkLimit(char c, uint8_t &n) const;
    size_t pad_left(int8_t len, int8_t width, char pad);
    size_t pad_right(int8_t len, int8_t width, char pad);

    /** No copy constructor. */
    Impl(Impl const &) = delete;
    /** No assignment operator. */
    void operator=(Impl const &) = delete;
};

}  // namespace impl
}  // namespace libcli

#endif

// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=4:
