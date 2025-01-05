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

#include <stdio.h>

#include <Arduino.h>

#include <AUnit.h>

#include <libcli.h>
#include <libcli/fake/FakeStream.h>

#define NL "\r\n"
#define BS "\b \b"

using Cli = libcli::Cli;
using State = libcli::Cli::State;
using NumberCallback = libcli::Cli::NumberCallback;
using FakeStream = libcli::fake::FakeStream;

void inject(Cli &cli, int n = 10) {
    while (--n >= 0)
        cli.loop();
}

struct Result {
    uint32_t number;
    State state;
    bool valid = false;
    uintptr_t context() { return reinterpret_cast<uintptr_t>(this); }
    void set(uint32_t n, State s) {
        number = n;
        state = s;
        valid = true;
    }
    static const NumberCallback callback;
};

const NumberCallback Result::callback = [](uint32_t number, uintptr_t context, State state) {
    reinterpret_cast<Result *>(context)->set(number, state);
};

test(ReadNumberTest, readHex) {
    FakeStream stream;
    Cli cli;
    cli.begin(stream);

    Result result;
    cli.readHex(Result::callback, result.context());
    assertFalse(result.valid);              // no call back
    assertEqual(stream.printerText(), "");  // no output
    inject(cli);
    assertFalse(result.valid);  // no call back

    stream.setInput("abCdxyzEfw ");
    inject(cli);
    assertEqual(stream.printerText(), "abCdEf");  // non-hexadecimals are filtered out
    assertFalse(result.valid);                    // no call back
    stream.flush();

    stream.setInput(' ');
    inject(cli);
    assertEqual(stream.printerText(), BS BS BS BS BS BS "00ABCDEF ");
    assertTrue(result.valid);  // called back
    assertEqual(result.number, (uint32_t)0x00ABCDEF);
    assertEqual(result.state, State::CLI_SPACE);
}

test(ReadNumberTest, readHex_limit) {
    FakeStream stream;
    Cli cli;
    cli.begin(stream);

    Result result;
    cli.readHex(Result::callback, result.context(), 0xABCC);
    assertFalse(result.valid);              // no call back
    assertEqual(stream.printerText(), "");  // no output
    inject(cli);
    assertFalse(result.valid);  // no call back

    stream.setInput("abCdxyzEfw ");
    inject(cli);
    assertEqual(stream.printerText(), "abC");  // limit is in effect
    assertFalse(result.valid);                 // no call back
    stream.flush();

    stream.setInput("c ");
    inject(cli);
    assertEqual(stream.printerText(), "c" BS BS BS BS "ABCC ");
    assertTrue(result.valid);  // called back
    assertEqual(result.number, (uint32_t)0xABCC);
    assertEqual(result.state, State::CLI_SPACE);

    stream.flush();
    result.valid = false;

    cli.readHex(Result::callback, result.context(), 0xFF);
    stream.setInput("aBCdxyzEfw ");
    inject(cli);
    assertEqual(stream.printerText(), "aB");  // limit is in effect
    assertFalse(result.valid);                // no call back
    stream.flush();

    stream.setInput("c\n");
    inject(cli);
    assertEqual(stream.printerText(), BS BS "AB ");
    assertTrue(result.valid);  // called back
    assertEqual(result.number, (uint32_t)0xAB);
    assertEqual(result.state, State::CLI_NEWLINE);
}

test(ReadNumberTest, readHex_defaultValue) {
    FakeStream stream;
    Cli cli;
    cli.begin(stream);

    const uint32_t defval = 0x1234;
    cli.printHex(defval, sizeof(defval) * 2);
    assertEqual(stream.printerText(), "00001234");
    stream.flush();

    Result result;
    cli.readHex(Result::callback, result.context(), UINT32_MAX, defval);
    assertFalse(result.valid);  // no call back
    assertEqual(stream.printerText(), BS BS BS BS BS BS BS BS "00001234");
    stream.flush();

    stream.setInput("\b\bff ");
    inject(cli);
    assertEqual(stream.printerText(), BS BS "ff" BS BS BS BS BS BS BS BS "000012FF ");
    assertTrue(result.valid);  // called back
    assertEqual(result.number, (uint32_t)0x000012FF);
    assertEqual(result.state, State::CLI_SPACE);
}

test(ReadNumberTest, readHex_edit) {
    FakeStream stream;
    Cli cli;
    cli.begin(stream);

    const uint16_t defval = 0x1234;
    cli.printHex(defval, sizeof(defval) * 2);
    assertEqual(stream.printerText(), "1234");
    stream.flush();

    Result result;
    cli.readHex(Result::callback, result.context(), UINT16_MAX, defval);
    assertFalse(result.valid);  // no call back
    assertEqual(stream.printerText(), BS BS BS BS "1234");
    stream.flush();

    stream.setInput("\b\b\baBcde ");
    inject(cli);
    assertEqual(stream.printerText(), BS BS BS "aBc" BS BS BS BS "1ABC ");
    assertTrue(result.valid);  // called back
    assertEqual(result.number, (uint32_t)0x1ABC);
    assertEqual(result.state, State::CLI_SPACE);
}

test(ReadNumberTest, readHex_delete) {
    FakeStream stream;
    Cli cli;
    cli.begin(stream);

    const uint16_t defval = 0x1234;
    cli.printHex(defval, sizeof(defval) * 2);
    assertEqual(stream.printerText(), "1234");
    stream.flush();

    Result result;
    cli.readHex(Result::callback, result.context(), UINT16_MAX, defval);
    assertFalse(result.valid);  // no call back
    assertEqual(stream.printerText(), BS BS BS BS "1234");
    stream.flush();

    stream.setInput("\b\b\bA\b\b\b");
    inject(cli);
    assertEqual(stream.printerText(), BS BS BS "A" BS BS);
    assertTrue(result.valid);  // called back
    assertEqual(result.number, (uint32_t)0);
    assertEqual(result.state, State::CLI_DELETE);
}

test(ReadNumberTest, readHex_cancel) {
    FakeStream stream;
    Cli cli;
    cli.begin(stream);

    Result result;
    cli.readHex(Result::callback, result.context());

    stream.setInput("1234abcd\x03");  // C-c is cancel
    inject(cli);
    assertEqual(stream.printerText(), "1234abcd cancel" NL);
    assertTrue(result.valid);  // called back
    assertEqual(result.number, (uint32_t)0x1234ABCD);
    assertEqual(result.state, State::CLI_CANCEL);
}

test(ReadNumberTest, readDec) {
    FakeStream stream;
    Cli cli;
    cli.begin(stream);

    Result result;
    cli.readDec(Result::callback, result.context());
    assertFalse(result.valid);              // no call back
    assertEqual(stream.printerText(), "");  // no output
    inject(cli);
    assertFalse(result.valid);  // no call back

    stream.setInput("12abCD");
    inject(cli);
    assertEqual(stream.printerText(), "12");  // non-hexadecimals are filtered out
    assertFalse(result.valid);                // no call back
    stream.flush();

    stream.setInput(' ');
    inject(cli);
    assertEqual(stream.printerText(), BS BS "        12 ");
    assertTrue(result.valid);  // called back
    assertEqual(result.number, (uint32_t)12);
    assertEqual(result.state, State::CLI_SPACE);
}

test(ReadNumberTest, readDec_limit) {
    FakeStream stream;
    Cli cli;
    cli.begin(stream);

    Result result;
    cli.readDec(Result::callback, result.context(), 9999);
    assertFalse(result.valid);              // no call back
    assertEqual(stream.printerText(), "");  // no output
    inject(cli);
    assertFalse(result.valid);  // no call back

    stream.setInput("9900000");
    inject(cli);
    assertEqual(stream.printerText(), "9900");  // limit is in effect
    assertFalse(result.valid);                  // no call back
    stream.flush();

    stream.setInput("\b\b99 ");
    inject(cli);
    assertEqual(stream.printerText(), BS BS "99" BS BS BS BS "9999 ");
    assertTrue(result.valid);  // called back
    assertEqual(result.number, (uint32_t)9999);
    assertEqual(result.state, State::CLI_SPACE);

    stream.flush();
    result.valid = false;

    cli.readDec(Result::callback, result.context(), 16);
    stream.setInput("aBCd1x9");
    inject(cli);
    assertEqual(stream.printerText(), "1");  // limit is in effect
    assertFalse(result.valid);               // no call back
    stream.flush();

    stream.setInput("6\n");
    inject(cli);
    assertEqual(stream.printerText(), "6" BS BS "16 ");
    assertTrue(result.valid);  // called back
    assertEqual(result.number, (uint32_t)16);
    assertEqual(result.state, State::CLI_NEWLINE);
}

test(ReadNumberTest, readDec_defaultValue) {
    FakeStream stream;
    Cli cli;
    cli.begin(stream);

    const uint32_t defval = 1234;
    cli.printDec(defval);
    assertEqual(stream.printerText(), "1234");
    stream.flush();

    Result result;
    cli.readDec(Result::callback, result.context(), 9999, defval);
    assertFalse(result.valid);  // no call back
    assertEqual(stream.printerText(), BS BS BS BS "1234");
    stream.flush();

    stream.setInput("\b\b99 ");
    inject(cli);
    assertEqual(stream.printerText(), BS BS "99" BS BS BS BS "1299 ");
    assertTrue(result.valid);  // called back
    assertEqual(result.number, (uint32_t)1299);
    assertEqual(result.state, State::CLI_SPACE);
}

test(ReadNumberTest, readDec_edit) {
    FakeStream stream;
    Cli cli;
    cli.begin(stream);

    const uint16_t defval = 1;
    cli.printDec(defval, 5);
    assertEqual(stream.printerText(), "    1");
    stream.flush();

    Result result;
    cli.readDec(Result::callback, result.context(), 10000, defval);
    assertFalse(result.valid);  // no call back
    assertEqual(stream.printerText(), BS BS BS BS BS "    1");
    stream.flush();

    stream.setInput("\b\b\b\b\b10000 ");
    inject(cli, 20);
    assertEqual(stream.printerText(), BS BS BS BS BS "10000" BS BS BS BS BS "10000 ");
    assertTrue(result.valid);  // called back
    assertEqual(result.number, (uint32_t)10000);
    assertEqual(result.state, State::CLI_SPACE);
}

test(ReadNumberTest, readDec_delete) {
    FakeStream stream;
    Cli cli;
    cli.begin(stream);

    const uint16_t defval = 1234;
    cli.printDec(defval, 5);
    assertEqual(stream.printerText(), " 1234");
    stream.flush();

    Result result;
    cli.readDec(Result::callback, result.context(), 99999, defval);
    assertFalse(result.valid);  // no call back
    assertEqual(stream.printerText(), BS BS BS BS BS " 1234");
    stream.flush();

    stream.setInput("\b\b\b\b5\b\b\b");
    inject(cli);
    assertEqual(stream.printerText(), BS BS BS BS "5" BS BS);
    assertTrue(result.valid);  // called back
    assertEqual(result.number, (uint32_t)0);
    assertEqual(result.state, State::CLI_DELETE);
}

test(ReadNumberTest, readDec_cancel) {
    FakeStream stream;
    Cli cli;
    cli.begin(stream);

    Result result;
    cli.readDec(Result::callback, result.context());

    stream.setInput("1234abcd\x03");  // C-c is cancel
    inject(cli);
    assertEqual(stream.printerText(), "1234 cancel" NL);
    assertTrue(result.valid);  // called back
    assertEqual(result.number, (uint32_t)1234);
    assertEqual(result.state, State::CLI_CANCEL);
}

test(ReadNumberTest, readNum) {
    FakeStream stream;
    Cli cli;
    cli.begin(stream);

    Result result;
    cli.readNum(Result::callback, result.context(), OCT);
    assertFalse(result.valid);              // no call back
    assertEqual(stream.printerText(), "");  // no output
    inject(cli);
    assertFalse(result.valid);  // no call back

    stream.setInput("1289CD");
    inject(cli);
    assertEqual(stream.printerText(), "12");  // non-octals are filtered out
    assertFalse(result.valid);                // no call back
    stream.flush();

    stream.setInput(' ');
    inject(cli);
    assertEqual(stream.printerText(), BS BS "00000000012 ");
    assertTrue(result.valid);  // called back
    assertEqual(result.number, (uint32_t)012);
    assertEqual(result.state, State::CLI_SPACE);
}

test(ReadNumberTest, readNum_limit) {
    FakeStream stream;
    Cli cli;
    cli.begin(stream);

    Result result;
    cli.readNum(Result::callback, result.context(), OCT, 07777);
    assertFalse(result.valid);              // no call back
    assertEqual(stream.printerText(), "");  // no output
    inject(cli);
    assertFalse(result.valid);  // no call back

    stream.setInput("770000");
    inject(cli);
    assertEqual(stream.printerText(), "7700");  // limit is in effect
    assertFalse(result.valid);                  // no call back
    stream.flush();

    stream.setInput("\b\b77 ");
    inject(cli);
    assertEqual(stream.printerText(), BS BS "77" BS BS BS BS "7777 ");
    assertTrue(result.valid);  // called back
    assertEqual(result.number, (uint32_t)07777);
    assertEqual(result.state, State::CLI_SPACE);

    stream.flush();
    result.valid = false;

    cli.readNum(Result::callback, result.context(), OCT, 017);
    stream.setInput("aBCd1x9");
    inject(cli);
    assertEqual(stream.printerText(), "1");  // limit is in effect
    assertFalse(result.valid);               // no call back
    stream.flush();

    stream.setInput("6\n");
    inject(cli);
    assertEqual(stream.printerText(), "6" BS BS "16 ");
    assertTrue(result.valid);  // called back
    assertEqual(result.number, (uint32_t)016);
    assertEqual(result.state, State::CLI_NEWLINE);
}

test(ReadNumberTest, readNum_defaultValue) {
    FakeStream stream;
    Cli cli;
    cli.begin(stream);

    const uint32_t defval = 01234;
    cli.printNum(defval, OCT);
    assertEqual(stream.printerText(), "1234");
    stream.flush();

    Result result;
    cli.readNum(Result::callback, result.context(), OCT, 07777, defval);
    assertFalse(result.valid);  // no call back
    assertEqual(stream.printerText(), BS BS BS BS "1234");
    stream.flush();

    stream.setInput("\b\b77 ");
    inject(cli);
    assertEqual(stream.printerText(), BS BS "77" BS BS BS BS "1277 ");
    assertTrue(result.valid);  // called back
    assertEqual(result.number, (uint32_t)01277);
    assertEqual(result.state, State::CLI_SPACE);
}

test(ReadNumberTest, readNum_edit) {
    FakeStream stream;
    Cli cli;
    cli.begin(stream);

    const uint16_t defval = 1;
    cli.printNum(defval, OCT, 5);
    assertEqual(stream.printerText(), "00001");
    stream.flush();

    Result result;
    cli.readNum(Result::callback, result.context(), OCT, 010000, defval);
    assertFalse(result.valid);  // no call back
    assertEqual(stream.printerText(), BS BS BS BS BS "00001");
    stream.flush();

    stream.setInput("\b\b\b\b\b10000 ");
    inject(cli, 20);
    assertEqual(stream.printerText(), BS BS BS BS BS "10000" BS BS BS BS BS "10000 ");
    assertTrue(result.valid);  // called back
    assertEqual(result.number, (uint32_t)010000);
    assertEqual(result.state, State::CLI_SPACE);
}

test(ReadNumberTest, readNum_delete) {
    FakeStream stream;
    Cli cli;
    cli.begin(stream);

    const uint16_t defval = 01234;
    cli.printNum(defval, OCT, 5);
    assertEqual(stream.printerText(), "01234");
    stream.flush();

    Result result;
    cli.readNum(Result::callback, result.context(), OCT, 010000, defval);
    assertFalse(result.valid);  // no call back
    assertEqual(stream.printerText(), BS BS BS BS BS "01234");
    stream.flush();

    stream.setInput("\b\b\b\b5\b\b\b");
    inject(cli);
    assertEqual(stream.printerText(), BS BS BS BS "5" BS BS);
    assertTrue(result.valid);  // called back
    assertEqual(result.number, (uint32_t)0);
    assertEqual(result.state, State::CLI_DELETE);
}

test(ReadNumberTest, readNum_cancel) {
    FakeStream stream;
    Cli cli;
    cli.begin(stream);

    Result result;
    cli.readNum(Result::callback, result.context(), OCT);

    stream.setInput("123489ab\x03");  // C-c is cancel
    inject(cli);
    assertEqual(stream.printerText(), "1234 cancel" NL);
    assertTrue(result.valid);  // called back
    assertEqual(result.number, (uint32_t)01234);
    assertEqual(result.state, State::CLI_CANCEL);
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
