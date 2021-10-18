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
#include <stdint.h>

#include "libcli.h"

namespace libcli {
namespace impl {

/** Implementation detail of libcli. */
struct Impl final {
    /** Get the singleton instance. */
    static constexpr Impl &instance() { return impl; }

    void begin(Stream &stream) {
        console = &stream;
        setProcessor(&Impl::processNop, 0);
    }

    void setCallback(Cli::LetterCallback callback, uintptr_t context);
    void setCallback(Cli::StringCallback callback, uintptr_t context, char *buffer, size_t size,
            bool hasDefval, bool word = false);
    void setCallback(Cli::NumberCallback callback, uint32_t context, uint32_t limit, uint8_t base);
    void setCallback(Cli::NumberCallback callback, uint32_t context, uint32_t limit,
            uint32_t defval, uint8_t base);

    void process(char c) { (this->*processor)(c); }

    size_t backspace(int8_t n = 1);
    size_t printHex(uint32_t number, uint8_t width = 0);
    size_t printDec(uint32_t number, uint8_t width = 0);

    /** No copy constructor. */
    Impl(Impl const &) = delete;
    /** No assignment operator. */
    void operator=(Impl const &) = delete;

    /** Delegate methods for Print. */
    size_t write(uint8_t val) { return console->write(val); }
    size_t write(const uint8_t *buf, size_t size) { return console->write(buf, size); }
    int availableForWrite() { return console->availableForWrite(); }

    /** Delegate methods for Stream. */
    int available() { return console->available(); }
    int read() { return console->read(); }
    int peek() { return console->peek(); }
#if defined (ESP_PLATFORM)
    void flush() { return console->flush(); }
#endif

private:
    /** The singleton of Impl. */
    static Impl impl;

    Stream *console;

    void setProcessor(void (Impl::*processor)(char), uintptr_t context) {
        this->processor = processor;
        this->context = context;
    }

    void (Impl::*processor)(char c);
    union {
        Cli::LetterCallback letter;
        Cli::StringCallback string;
        Cli::NumberCallback number;
    } callback;
    uintptr_t context;

    size_t str_limit;
    size_t str_len;
    bool str_word;
    char *str_buffer;

    uint32_t num_value;
    uint32_t num_limit;
    uint8_t num_base;
    uint8_t num_len;
    uint8_t num_width;

    /** Hidden efault constructor. */
    Impl() {}

    void processNop(char c) { (void)c; }
    void processLetter(char c);
    void processString(char c);
    void processNumber(char c);
    bool checkLimit(char c, uint8_t &n) const;
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
