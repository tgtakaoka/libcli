/*
 * Copyright 2022 Tadashi G. Takaoka
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

#ifndef __LIBCLI_FAKE_SERIAL_H__
#define __LIBCLI_FAKE_SERIAL_H__

namespace libcli {
namespace fake {

class FakeStream : public Stream {
public:
    FakeStream() : Stream() {}

    // Print
    size_t write(uint8_t data) override { return _printer.write((char)data); }

    size_t write(const uint8_t *data, size_t size) override {
        return data ? _printer.write((const char *)data, size) : 0;
    }

    void flush()
#if defined(ESP32) || defined(ARDUINO_ARCH_STM32)
#else
            override
#endif
    {
        _printer.clear();
    }

    // FakePrinter
    const char *printerText() const {
        return _printer.c_str();
    }

    int printerLength() const {
        return _printer.length();
    }

    // Stream
    int available() override {
        return _reader.available();
    }

    int peek() override {
        return available() ? _reader.peek() : -1;
    }

    int read() override {
        return available() ? _reader.read() : -1;
    }

    // FakeStream
    void setInput(char c) {
        _reader.set(c);
    }

    void setInput(const char *text) {
        _reader.set(text);
    }

    void setInput(const __FlashStringHelper *text_P) {
        setInput_P(reinterpret_cast<const /*PROGMEM*/ char *>(text_P));
    }

    void setInput_P(const /*PROGMEM*/ char *text_P) {
        _reader.set_P(text_P);
    }

private:
    struct {
        size_t write(char c) {
            _buffer += c;
            return sizeof(c);
        }

        size_t write(const char *text, const size_t size) {
            _buffer.reserve(_buffer.length() + size);
            for (const char *end = text + size; text < end; text++)
                write(*text);
            return size;
        }

        const char *c_str() const { return _buffer.c_str(); }

        int length() const { return _buffer.length(); }

        void clear() { _buffer = ""; }

    private:
        String _buffer;
    } _printer;

    struct {
        int available() const { return (_text_P ? _length_P : _buffer.length()) - _index; }

        int peek() const {
            return _text_P ? pgm_read_byte(_text_P + _index) : _buffer.charAt(_index);
        }

        int read() {
            const auto c = peek();
            _index++;
            return c;
        }

        void set(char c) {
            _buffer = c;
            _text_P = nullptr;
            _index = 0;
        }

        void set(const char *text) {
            _buffer = text;
            _text_P = nullptr;
            _index = 0;
        }

        void set_P(const /*PROGMEM*/ char *text_P) {
            _text_P = text_P;
            _length_P = strlen_P(text_P);
            _index = 0;
        }

    private:
        String _buffer;
        const /*PROGMEM*/ char*_text_P = nullptr;
        size_t _length_P = 0;
        uint8_t _index = 0;
    } _reader;
};

}  // namespace fake
}  // namespace libcli

#endif

// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=4:
