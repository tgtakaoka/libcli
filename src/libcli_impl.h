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
class Impl final {
public:
    static constexpr Impl &instance() { return _impl; }

    void begin(Stream &stream) {
        console = &stream;
        _proc.processor = &impl::Impl::processNop;
    }

    /** Delegate methods of Print. */
    size_t write(uint8_t val) { return console->write(val); }
    size_t write(const uint8_t *buffer, size_t size) { return console->write(buffer, size); }
    int availableForWrite() { return console->availableForWrite(); }

    /** Delegate methods of Stream. */
    int available() { return console->available(); }
    int read() { return console->read(); }
    int peek() { return console->peek(); }

    struct {
        void (Impl::*processor)(char c);
        union {
            Cli::LetterHandler letter;
            Cli::StringHandler string;
            Cli::ValueHandler value;
        } handler;
        uintptr_t extra;

        void setHandler(Cli::LetterHandler handler, uintptr_t extra) {
            this->processor = &impl::Impl::processLetter;
            this->extra = extra;
            this->handler.letter = handler;
        }
        void setHandler(Cli::StringHandler handler, uintptr_t extra) {
            this->processor = &impl::Impl::processLetter;
            this->extra = extra;
            this->handler.string = handler;
        }
        void setHandler(Cli::ValueHandler handler, uintptr_t extra, void (Impl::*proc)(char)) {
            this->processor = proc;
            this->extra = extra;
            this->handler.value = handler;
        }
        void process(char c, Impl &impl) {
            (impl.*processor)(c);
        }
    } _proc;

    struct {
        uint8_t len;
        uint8_t width;
        uint32_t val;
        uint32_t max;
    } _value;

    struct {
        uint8_t len;
        char buf[80];
    } _string;

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

    /** The singleton of Impl. */
    static Impl _impl;

    /** No copy constructor. */
    Impl(Impl const &) = delete;
    /** No assignment operator. */
    void operator=(Impl const &) = delete;

private:
    Stream *console;

    /** Hidden efault constructor. */
    Impl() {}

    void processNop(char c) { (void)c; }
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
