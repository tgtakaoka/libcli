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
#include <stddef.h>
#include <stdint.h>

#define LIBCLI_VERSION_MAJOR 1
#define LIBCLI_VERSION_MINOR 2
#define LIBCLI_VERSION_PATCH 5
#define LIBCLI_VERSION_STRING "1.2.5"

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
    typedef void (*LetterCallback)(char letter, uintptr_t context);
    /** Callback function of |readWord| and |readLine|. */
    typedef void (*StringCallback)(char *string, uintptr_t context, State state);
    /** Callback function of |readHex| and |readDec|. */
    typedef void (*NumberCallback)(uint32_t number, uintptr_t context, State state);

    /**
     * Read a single letter.
     */
    void readLetter(LetterCallback callback, uintptr_t context);

    /**
     * Read a string delimitted by space into |buffer| which has |size| bytes. If |hasDefval| is
     * true, |buffer| contains a default value.
     */
    void readWord(StringCallback callback, uintptr_t context, char *buffer, size_t size,
            bool hasDefval = false);

    /**
     * Read a string delimitted by newline into |buffer| which has |size| bytes. If |hasDefval| is
     * true, |buffer| contains a default value.
     */
    void readLine(StringCallback callback, uintptr_t context, char *buffer, size_t size,
            bool hasDefval = false);
    /**
     * Read hexadecimal number less or equals to |limit|.
     */
    void readHex(NumberCallback callback, uintptr_t context, uint32_t limit = UINT32_MAX);

    /**
     * Read hexadecimal number less or equals to |limit| with |defval| as default.
     */
    void readHex(NumberCallback callback, uintptr_t context, uint32_t limit, uint32_t defval);

    /**
     * Read decimal number less or equal to |limit|.
     */
    void readDec(NumberCallback callback, uintptr_t context, uint32_t limit = UINT32_MAX);

    /**
     * Read decimal number less or equal to |limit| with |defval| as default.
     */
    void readDec(NumberCallback callback, uintptr_t context, uint32_t limit, uint32_t defval);

    /**
     * Print |number| in 0-prefixed hexadecimal format of |width| chars. Negative |width| means left
     * aligned.
     */
    size_t printHex(uint32_t number, int8_t width = 0);

    /**
     * Print |number| in right-aligned decimal format of |width| chars. Negative |width| means left
     * aligned.
     */
    size_t printDec(uint32_t number, int8_t width = 0);

    /**
     * Print |number| in 0-prefixed hexadecimal format of |width| chars and newline.
     */
    size_t printlnHex(uint32_t number, int8_t width = 0);

    /**
     * Print |number| in right-aligned decimal format of |width| chars and newline.
     */
    size_t printlnDec(uint32_t number, int8_t width = 0);

    /*
     * Print |text| in right-aligned of |width| chars. Negative |width| means left-aligned.
     */
    size_t printStr(const __FlashStringHelper *text, int8_t width = 0);
    size_t printStr(const char *text, int8_t witdh = 0);
    size_t printStr_P(const /*PROGMEM*/ char *text_P, int8_t witdh = 0);

    /*
     * Print |text| in right-aligned of |width| chars and newline.
     */
    size_t printlnStr(const __FlashStringHelper *text, int8_t width = 0);
    size_t printlnStr(const char *text, int8_t witdh = 0);
    size_t printlnStr_P(const /*PROGMEM*/ char *text_P, int8_t witdh = 0);

    /**
     * Print backspace |n| times.
     */
    size_t backspace(int8_t n = 1);

    /** Virtual methods of Print. */
    size_t write(uint8_t val) override;
    size_t write(const uint8_t *buffer, size_t size) override;
    int availableForWrite() override;

    /** Virtual methods of Stream. */
    int available() override;
    int read() override;
    int peek() override;
    void flush() override;

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
