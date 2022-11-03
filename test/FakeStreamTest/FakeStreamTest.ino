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

#include <Arduino.h>

#include <AUnit.h>

#include <libcli/fake/FakeStream.h>

test(printTest, empty) {
    libcli::fake::FakeStream stream;

    assertEqual(stream.printerLength(), 0);
    assertEqual(stream.printerText(), "");
    stream.flush();
}

test(printTest, writeChar) {
    libcli::fake::FakeStream stream;

    assertEqual((int)stream.write('c'), 1);
    assertEqual(stream.printerText(), "c");

    assertEqual((int)stream.write('x'), 1);
    assertEqual(stream.printerText(), "cx");

    assertEqual((int)stream.println(), 2);
    assertEqual(stream.printerText(), "cx\r\n");

    stream.flush();
    assertEqual(stream.printerLength(), 0);
    assertEqual(stream.printerText(), "");
}

test(printTest, writeText) {
    libcli::fake::FakeStream stream;

    assertEqual((int)stream.write((const uint8_t *)"abcde", 3), 3);
    assertEqual(stream.printerText(), "abc");

    assertEqual((int)stream.write((const uint8_t *)"xyzw", 4), 4);
    assertEqual(stream.printerText(), "abcxyzw");

    stream.flush();
    assertEqual(stream.printerLength(), 0);
    assertEqual(stream.printerText(), "");
}

test(streamTest, empty) {
    libcli::fake::FakeStream stream;

    assertEqual(stream.available(), 0);
    assertEqual(stream.peek(), -1);
    assertEqual(stream.read(), -1);
}

test(streamTest, letter) {
    libcli::fake::FakeStream stream;

    stream.setInput('A');
    assertEqual(stream.available(), 1);
    assertEqual(stream.peek(), (int)'A');
    assertEqual(stream.read(), (int)'A');
    assertEqual(stream.available(), 0);

    stream.setInput('x');
    assertEqual(stream.available(), 1);
    assertEqual(stream.peek(), (int)'x');
    assertEqual(stream.read(), (int)'x');
    assertEqual(stream.available(), 0);
}

test(streamTest, text) {
    libcli::fake::FakeStream stream;

    {
        char text[] = {'a', 'b', 'c', 0};
        stream.setInput(text);
    }
    assertEqual(stream.available(), 3);
    assertEqual(stream.peek(), (int)'a');
    assertEqual(stream.read(), (int)'a');
    assertEqual(stream.available(), 2);
    assertEqual(stream.peek(), (int)'b');
    assertEqual(stream.read(), (int)'b');
    assertEqual(stream.available(), 1);
    assertEqual(stream.peek(), (int)'c');
    assertEqual(stream.read(), (int)'c');
    assertEqual(stream.available(), 0);
    assertEqual(stream.peek(), -1);
    assertEqual(stream.read(), -1);
    assertEqual(stream.available(), 0);
    assertEqual(stream.peek(), -1);
    assertEqual(stream.read(), -1);
}

test(streamTest, text_P) {
    libcli::fake::FakeStream stream;

    stream.setInput(PSTR("abc"));
    assertEqual(stream.available(), 3);
    assertEqual(stream.peek(), (int)'a');
    assertEqual(stream.read(), (int)'a');
    assertEqual(stream.available(), 2);
    assertEqual(stream.peek(), (int)'b');
    assertEqual(stream.read(), (int)'b');
    assertEqual(stream.available(), 1);
    assertEqual(stream.peek(), (int)'c');
    assertEqual(stream.read(), (int)'c');
    assertEqual(stream.available(), 0);
    assertEqual(stream.peek(), -1);
    assertEqual(stream.read(), -1);
    assertEqual(stream.available(), 0);
    assertEqual(stream.peek(), -1);
    assertEqual(stream.read(), -1);
}


test(streamTest, text_F) {
    libcli::fake::FakeStream stream;

    stream.setInput(F("abc"));
    assertEqual(stream.available(), 3);
    assertEqual(stream.peek(), (int)'a');
    assertEqual(stream.read(), (int)'a');
    assertEqual(stream.available(), 2);
    assertEqual(stream.peek(), (int)'b');
    assertEqual(stream.read(), (int)'b');
    assertEqual(stream.available(), 1);
    assertEqual(stream.peek(), (int)'c');
    assertEqual(stream.read(), (int)'c');
    assertEqual(stream.available(), 0);
    assertEqual(stream.peek(), -1);
    assertEqual(stream.read(), -1);
    assertEqual(stream.available(), 0);
    assertEqual(stream.peek(), -1);
    assertEqual(stream.read(), -1);
}

void setup() {}

void loop() {
    aunit::TestRunner::run();
}

// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=4:
