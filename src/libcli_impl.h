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
    static constexpr Impl &instance() { return _impl; }

    void begin(Stream &stream) {
        console = &stream;
        setProcessor(&Impl::processNop, 0);
    }

    /** Delegate methods of Print. */
    size_t write(uint8_t val) { return console->write(val); }
    size_t write(const uint8_t *buf, size_t size) { return console->write(buf, size); }
    int availableForWrite() { return console->availableForWrite(); }

    /** Delegate methods of Stream. */
    int available() { return console->available(); }
    int read() { return console->read(); }
    int peek() { return console->peek(); }

    void setHandler(Cli::LetterHandler handler, uintptr_t extra) {
        this->handler.letter = handler;
        setProcessor(&Impl::processLetter, extra);
    }
    void setHandler(Cli::StringHandler handler, uintptr_t extra) {
        this->handler.string = handler;
        str_buf[str_len = 0] = 0;
        setProcessor(&Impl::processString, extra);
    }
    void setHandler(Cli::ValueHandler handler, uint32_t extra, uint32_t limit, uint8_t base);
    void setHandler(Cli::ValueHandler handler, uint32_t extra, uint32_t limit, uint32_t defval,
            uint8_t base);

    void process(char c) { (this->*processor)(c); }

    size_t backspace(int8_t n = 1);
    size_t printHex(uint32_t value, uint8_t width = 0);
    size_t printDec(uint32_t value, uint8_t width = 0);

    /** The singleton of Impl. */
    static Impl _impl;

    /** No copy constructor. */
    Impl(Impl const &) = delete;
    /** No assignment operator. */
    void operator=(Impl const &) = delete;

private:
    Stream *console;

    void setProcessor(void (Impl::*proc)(char), uintptr_t ext) {
        processor = proc;
        extra = ext;
    }

    void (Impl::*processor)(char c);
    union {
        Cli::LetterHandler letter;
        Cli::StringHandler string;
        Cli::ValueHandler value;
    } handler;
    uintptr_t extra;

    uint8_t str_len;
    char str_buf[80 + 1];

    uint32_t val_limit;
    uint32_t val_value;
    uint8_t val_base;
    uint8_t val_len;
    uint8_t val_width;

private:
    /** Hidden efault constructor. */
    Impl() {}

    void processNop(char c) { (void)c; }
    void processLetter(char c) { handler.letter(c, extra); }
    void processString(char c);
    void processValue(char c);
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
