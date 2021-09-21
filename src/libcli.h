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

#ifndef __LIBCLI_H__
#define __LIBCLI_H__

#include <Arduino.h>
#include <stdint.h>

#define LIBCLI_VERSION_MAJOR 1
#define LIBCLI_VERSION_MINOR 0
#define LIBCLI_VERSION_PATCH 4
#define LIBCLI_VERSION_STRING "1.0.4"

namespace libcli {

namespace impl {
class Impl;
}

/** Library interface of libcli. */
class Cli final : public Stream {
public:
    /** Get the singleton instance. */
    static Cli &instance();
    /** Initialize with |console| as command line interface. */
    void begin(Stream &console);
    /** Event loop; shold be called in Sketch's main loop(). */
    void loop();

    /** A state what terminates user input. */
    enum State : uint8_t {
        CLI_SPACE,    // an input is terminated by space.
        CLI_NEWLINE,  // an input is terminated by newline.
        CLI_DELETE,   // current input is canceled and back to previous.
        CLI_CANCEL,   // whole input is canceled.
    };

    /** Callback function of |readLetter|. */
    typedef void (*LetterHandler)(char letter, uintptr_t extra);
    /** Callback function of |readString|. */
    typedef void (*StringHandler)(const char *string, uintptr_t extra, State state);
    /** Callback function of |readHex| and |readDec|. */
    typedef void (*ValueHandler)(uint32_t value, uintptr_t extra, State state);

    /** Read a single letter. */
    void readLetter(LetterHandler handler, uintptr_t extra);
    /** Read a string delimitted by newline. */
    void readString(StringHandler hadler, uintptr_t extra);
    /** Read hexadecimal number in |digits| format. */
    void readHex(ValueHandler handler, uintptr_t extra, uint8_t digits);
    /** Read hexadecimal number in |digits| format with |defval| as default. */
    void readHex(ValueHandler handler, uintptr_t extra, uint8_t digits, uint32_t defval);
    /** Read decimal number less or equal to |limit|. */
    void readDec(ValueHandler handler, uintptr_t extra, uint32_t limit);
    /** Read decimal number less or equal to |limit| with |defval| as default. */
    void readDec(ValueHandler handler, uintptr_t extra, uint32_t limit, uint32_t defval);

    /** Print |value| in 0-prefixed hexadecimal format of |width| chars. */
    size_t printHex(uint32_t value, uint8_t width = 0);
    /** Print |value| in right-aligned decimal format of |width| chars. */
    size_t printDec(uint32_t value, uint8_t width = 0);
    /** Print backspace |n| times. */
    size_t backspace(int8_t n = 1);

    /** Virtual methods of Print. */
    size_t write(uint8_t val) override;
    size_t write(const uint8_t *buffer, size_t size) override;
    int availableForWrite() override;

    /** Virtual methods of Stream. */
    int available() override;
    int read() override;
    int peek() override;

    /** No copy constructor. */
    Cli(Cli const &) = delete;
    /** No assignment operator. */
    void operator=(Cli const &) = delete;

private:
    /** Implemetation detail. */
    impl::Impl &_impl;

    /** The singleton is implemented in Cli::instance(). */
    friend class impl::Impl;
    Cli(impl::Impl &impl) : Stream(), _impl(impl) {}
};

}  // namespace libcli

#endif

// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=4:
