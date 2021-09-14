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

class Cli {
public:
    enum State : uint8_t {
        CLI_SPACE,    // an input is terminated by space.
        CLI_NEWLINE,  // an input is terminated by newline.
        CLI_DELETE,   // current input is canceled and back to previous.
        CLI_CANCEL,   // whole input is canceled.
    };

    // Callback function of |readLetter|.
    typedef void (*LetterHandler)(char letter, uintptr_t extra);
    // Callback function of |readString|.
    typedef void (*StringHandler)(char *string, uintptr_t extra, State state);
    // Callback function of |readHex| and |readDec|.
    typedef void (*ValueHandler)(uint32_t value, uintptr_t extra, State state);

    // Initialize function.
    void begin(Stream &console);
    // Event loop.
    void loop();

    void readLetter(LetterHandler handler, uintptr_t extra);
    void readString(StringHandler hadler, uintptr_t extra);
    void readHex8(ValueHandler handler, uintptr_t extra);
    void readHex16(ValueHandler handler, uintptr_t extra);
    void readHex32(ValueHandler handler, uintptr_t extra);
    void readHex8(ValueHandler handler, uintptr_t extra, uint8_t val8);
    void readHex16(ValueHandler handler, uintptr_t extra, uint16_t val16);
    void readHex32(ValueHandler handler, uintptr_t extra, uint32_t val32);
    void readDec8(ValueHandler handler, uintptr_t extra);
    void readDec16(ValueHandler handler, uintptr_t extra);
    void readDec32(ValueHandler handler, uintptr_t extra);
    void readDec8(ValueHandler handler, uintptr_t extra, uint8_t val8);
    void readDec16(ValueHandler handler, uintptr_t extra, uint16_t val16);
    void readDec32(ValueHandler handler, uintptr_t extra, uint32_t val32);

    size_t backspace(int8_t n = 1);
    size_t printHex8(uint8_t val8);
    size_t printHex16(uint16_t val16);
    size_t printHex20(uint32_t val20);
    size_t printHex24(uint32_t val24);
    size_t printHex32(uint32_t val32);
    size_t printDec8(uint8_t val8);
    size_t printDec16(uint16_t val16);
    size_t printDec32(uint32_t val32);

    template <typename T>
    size_t print(T value) {
        return _console->print(value);
    }
    template <typename T>
    size_t println(T value) {
        return _console->println(value);
    }
    size_t println() { return _console->println(); }

private:
    Stream *_console;

    void (Cli::*_processor)(char c);

    union {
        LetterHandler letter;
        StringHandler string;
        ValueHandler value;
    } _handler;
    uintptr_t _extra;

    uint8_t _valueLen;
    uint32_t _value;
    enum Bits : int8_t {
        BITS8 = 8,
        BITS16 = 16,
        BITS32 = 32,
    } _valueBits;

    uint8_t _stringLen;
    char _string[80];

    void processNop(char c) { (void)c; }
    void processLetter(char c);
    void processString(char c);
    void processHex(char c);
    void processDec(char c);

    void readHex(ValueHandler, uintptr_t extra, int8_t bits, uint32_t defval = 0);
    void setHex(Bits bits, uint32_t defval);
    void readDec(ValueHandler, uintptr_t extra, int8_t bits, uint32_t defval = 0);
    void setDec(Bits bits, uint32_t defval);
    bool acceptDec(char c) const;
};

extern class Cli Cli;

}  // namespace libcli

#endif

// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=4:
