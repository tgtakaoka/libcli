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

#include "libcli_impl.h"

#include <stdint.h>

namespace libcli {

Cli &Cli::instance() {
    /** The singleton of Cli. */
    static Cli cli{impl::Impl::instance()};
    return cli;
}

/**
 * Virual methods of Print class.
 */

size_t Cli::write(uint8_t val) {
    return _impl.write(val);
}

size_t Cli::write(const uint8_t *buffer, size_t size) {
    return _impl.write(buffer, size);
}

int Cli::availableForWrite() {
    return _impl.availableForWrite();
}

void Cli::flush() {
    _impl.flush();
}

/**
 * Virtual methods of Stream class.
 */

int Cli::available() {
    return _impl.available();
}

int Cli::read() {
    return _impl.read();
}

int Cli::peek() {
    return _impl.peek();
}

size_t Cli::printHex(uint32_t number, int8_t width) {
    return _impl.printHex(number, width, false);
}

size_t Cli::printDec(uint32_t number, int8_t width) {
    return _impl.printDec(number, width, false);
}

size_t Cli::printStr(const __FlashStringHelper *text, int8_t width) {
    return _impl.printStr(text, width, false);
}

size_t Cli::printStr(const char *text, int8_t width) {
    return _impl.printStr(text, width, false);
}

size_t Cli::printStr_P(const /*PROGMEM*/ char *text_P, int8_t width) {
    return _impl.printStr(reinterpret_cast<const __FlashStringHelper *>(text_P), width, false);
}

size_t Cli::printlnHex(uint32_t number, int8_t width) {
    return _impl.printHex(number, width, true);
}

size_t Cli::printlnDec(uint32_t number, int8_t width) {
    return _impl.printDec(number, width, true);
}

size_t Cli::printlnStr(const __FlashStringHelper *text, int8_t width) {
    return _impl.printStr(text, width, true);
}

size_t Cli::printlnStr(const char *text, int8_t width) {
    return _impl.printStr(text, width, true);
}

size_t Cli::printlnStr_P(const /*PROGMEM*/ char *text_P, int8_t width) {
    return _impl.printStr(reinterpret_cast<const __FlashStringHelper *>(text_P), width, true);
}

size_t Cli::backspace(int8_t n) {
    return _impl.backspace(n);
}

void Cli::readLetter(LetterCallback callback, uintptr_t context) {
    _impl.setCallback(callback, context);
}

void Cli::readWord(
        StringCallback callback, uintptr_t context, char *buffer, size_t size, bool hasDefval) {
    _impl.setCallback(callback, context, buffer, size, hasDefval, true);
}

void Cli::readLine(
        StringCallback callback, uintptr_t context, char *buffer, size_t size, bool hasDefval) {
    _impl.setCallback(callback, context, buffer, size, hasDefval, false);
}

void Cli::readHex(NumberCallback callback, uintptr_t context, uint32_t limit) {
    _impl.setCallback(callback, context, limit, true);
}

void Cli::readHex(NumberCallback callback, uintptr_t context, uint32_t limit, uint32_t defval) {
    _impl.setCallback(callback, context, limit, defval, true);
}

void Cli::readDec(NumberCallback callback, uintptr_t context, uint32_t limit) {
    _impl.setCallback(callback, context, limit, false);
}

void Cli::readDec(NumberCallback callback, uintptr_t context, uint32_t limit, uint32_t defval) {
    _impl.setCallback(callback, context, limit, defval, false);
}

void Cli::begin(Stream &console) {
    _impl.begin(console);
}

void Cli::loop() {
    if (available()) {
        _impl.process(read());
    }
}

}  // namespace libcli

// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=4:
