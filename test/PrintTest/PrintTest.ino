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

#include <libcli.h>
#include <libcli/fake/FakeStream.h>

#define NL "\r\n"
#define BS "\b \b"

using Cli = libcli::Cli;
using FakeStream = libcli::fake::FakeStream;

test(printTest, printHex) {
    FakeStream stream;
    Cli cli;
    cli.begin(stream);

    assertEqual(cli.printHex(0x1234), (size_t)4);
    assertEqual(stream.printerText(), "1234");
    stream.flush();

    assertEqual(cli.printHex(0x1234, 8), (size_t)8);
    assertEqual(stream.printerText(), "00001234");
    stream.flush();

    assertEqual(cli.printHex(0x1234, -8), (size_t)8);
    assertEqual(stream.printerText(), "1234    ");
    stream.flush();

    assertEqual(cli.printHex(0x12345678, 4), (size_t)8);
    assertEqual(stream.printerText(), "12345678");
    stream.flush();

    assertEqual(cli.printHex(0x12345678, -4), (size_t)8);
    assertEqual(stream.printerText(), "12345678");
    stream.flush();
}

test(printTest, printlnHex) {
    FakeStream stream;
    Cli cli;
    cli.begin(stream);

    assertEqual(cli.printlnHex(0x1234), (size_t)6);
    assertEqual(stream.printerText(), "1234" NL);
    stream.flush();

    assertEqual(cli.printlnHex(0x1234, 8), (size_t)10);
    assertEqual(stream.printerText(), "00001234" NL);
    stream.flush();

    assertEqual(cli.printlnHex(0x1234, -8), (size_t)10);
    assertEqual(stream.printerText(), "1234    " NL);
    stream.flush();

    assertEqual(cli.printlnHex(0x12345678, 4), (size_t)10);
    assertEqual(stream.printerText(), "12345678" NL);
    stream.flush();

    assertEqual(cli.printlnHex(0x12345678, -4), (size_t)10);
    assertEqual(stream.printerText(), "12345678" NL);
    stream.flush();
}

test(printTest, printDec) {
    FakeStream stream;
    Cli cli;
    cli.begin(stream);

    assertEqual(cli.printDec(1234), (size_t)4);
    assertEqual(stream.printerText(), "1234");
    stream.flush();

    assertEqual(cli.printDec(1234, 8), (size_t)8);
    assertEqual(stream.printerText(), "    1234");
    stream.flush();

    assertEqual(cli.printDec(1234, -8), (size_t)8);
    assertEqual(stream.printerText(), "1234    ");
    stream.flush();

    assertEqual(cli.printDec(12345678, 4), (size_t)8);
    assertEqual(stream.printerText(), "12345678");
    stream.flush();

    assertEqual(cli.printDec(12345678, -4), (size_t)8);
    assertEqual(stream.printerText(), "12345678");
    stream.flush();
}

test(printTest, printlnDec) {
    FakeStream stream;
    Cli cli;
    cli.begin(stream);

    assertEqual(cli.printlnDec(1234), (size_t)6);
    assertEqual(stream.printerText(), "1234" NL);
    stream.flush();

    assertEqual(cli.printlnDec(1234, 8), (size_t)10);
    assertEqual(stream.printerText(), "    1234" NL);
    stream.flush();

    assertEqual(cli.printlnDec(1234, -8), (size_t)10);
    assertEqual(stream.printerText(), "1234    " NL);
    stream.flush();

    assertEqual(cli.printlnDec(12345678, 4), (size_t)10);
    assertEqual(stream.printerText(), "12345678" NL);
    stream.flush();

    assertEqual(cli.printlnDec(12345678, -4), (size_t)10);
    assertEqual(stream.printerText(), "12345678" NL);
    stream.flush();
}

test(printTest, printNum) {
    FakeStream stream;
    Cli cli;
    cli.begin(stream);

    assertEqual(cli.printNum(1234), (size_t)4);
    assertEqual(stream.printerText(), "1234");
    stream.flush();

    assertEqual(cli.printNum(01234, OCT), (size_t)4);
    assertEqual(stream.printerText(), "1234");
    stream.flush();

    assertEqual(cli.printNum(0x123, BIN), (size_t)9);
    assertEqual(stream.printerText(), "100100011");
    stream.flush();

    assertEqual(cli.printNum(0x123, BIN, 16), (size_t)16);
    assertEqual(stream.printerText(), "0000000100100011");
    stream.flush();

    assertEqual(cli.printNum(01234, OCT, -8), (size_t)8);
    assertEqual(stream.printerText(), "1234    ");
    stream.flush();

    assertEqual(cli.printNum(01234, OCT, 8), (size_t)8);
    assertEqual(stream.printerText(), "00001234");
    stream.flush();

    assertEqual(cli.printNum(01234567, OCT, -4), (size_t)7);
    assertEqual(stream.printerText(), "1234567");
    stream.flush();
}

test(printTest, printlnNum) {
    FakeStream stream;
    Cli cli;
    cli.begin(stream);

    assertEqual(cli.printlnNum(1234), (size_t)6);
    assertEqual(stream.printerText(), "1234" NL);
    stream.flush();

    assertEqual(cli.printlnNum(01234, OCT), (size_t)6);
    assertEqual(stream.printerText(), "1234" NL);
    stream.flush();

    assertEqual(cli.printlnNum(0x123, BIN), (size_t)11);
    assertEqual(stream.printerText(), "100100011" NL);
    stream.flush();

    assertEqual(cli.printlnNum(0x123, BIN, 16), (size_t)18);
    assertEqual(stream.printerText(), "0000000100100011" NL);
    stream.flush();

    assertEqual(cli.printlnNum(01234, OCT, -8), (size_t)10);
    assertEqual(stream.printerText(), "1234    " NL);
    stream.flush();

    assertEqual(cli.printlnNum(01234, OCT, 8), (size_t)10);
    assertEqual(stream.printerText(), "00001234" NL);
    stream.flush();

    assertEqual(cli.printlnNum(01234567, OCT, -4), (size_t)9);
    assertEqual(stream.printerText(), "1234567" NL);
    stream.flush();
}

test(printTest, printStr) {
    FakeStream stream;
    Cli cli;
    cli.begin(stream);

    assertEqual(cli.printStr("1234"), (size_t)4);
    assertEqual(stream.printerText(), "1234");
    stream.flush();

    assertEqual(cli.printStr("1234", 8), (size_t)8);
    assertEqual(stream.printerText(), "    1234");
    stream.flush();

    assertEqual(cli.printStr("1234", -8), (size_t)8);
    assertEqual(stream.printerText(), "1234    ");
    stream.flush();

    assertEqual(cli.printStr("12345678", 4), (size_t)8);
    assertEqual(stream.printerText(), "12345678");
    stream.flush();

    assertEqual(cli.printStr("12345678", -4), (size_t)8);
    assertEqual(stream.printerText(), "12345678");
    stream.flush();
}

test(printTest, printlnStr) {
    FakeStream stream;
    Cli cli;
    cli.begin(stream);

    assertEqual(cli.printlnStr("1234"), (size_t)6);
    assertEqual(stream.printerText(), "1234" NL);
    stream.flush();

    assertEqual(cli.printlnStr("1234", 8), (size_t)10);
    assertEqual(stream.printerText(), "    1234" NL);
    stream.flush();

    assertEqual(cli.printlnStr("1234", -8), (size_t)10);
    assertEqual(stream.printerText(), "1234    " NL);
    stream.flush();

    assertEqual(cli.printlnStr("12345678", 4), (size_t)10);
    assertEqual(stream.printerText(), "12345678" NL);
    stream.flush();

    assertEqual(cli.printlnStr("12345678", -4), (size_t)10);
    assertEqual(stream.printerText(), "12345678" NL);
    stream.flush();
}

test(printTest, printStrF) {
    FakeStream stream;
    Cli cli;
    cli.begin(stream);

    assertEqual(cli.printStr(F("1234")), (size_t)4);
    assertEqual(stream.printerText(), "1234");
    stream.flush();

    assertEqual(cli.printStr(F("1234"), 8), (size_t)8);
    assertEqual(stream.printerText(), "    1234");
    stream.flush();

    assertEqual(cli.printStr(F("1234"), -8), (size_t)8);
    assertEqual(stream.printerText(), "1234    ");
    stream.flush();

    assertEqual(cli.printStr(F("12345678"), 4), (size_t)8);
    assertEqual(stream.printerText(), "12345678");
    stream.flush();

    assertEqual(cli.printStr(F("12345678"), -4), (size_t)8);
    assertEqual(stream.printerText(), "12345678");
    stream.flush();
}

test(printTest, printlnStrF) {
    FakeStream stream;
    Cli cli;
    cli.begin(stream);

    assertEqual(cli.printlnStr(F("1234")), (size_t)6);
    assertEqual(stream.printerText(), "1234" NL);
    stream.flush();

    assertEqual(cli.printlnStr(F("1234"), 8), (size_t)10);
    assertEqual(stream.printerText(), "    1234" NL);
    stream.flush();

    assertEqual(cli.printlnStr(F("1234"), -8), (size_t)10);
    assertEqual(stream.printerText(), "1234    " NL);
    stream.flush();

    assertEqual(cli.printlnStr(F("12345678"), 4), (size_t)10);
    assertEqual(stream.printerText(), "12345678" NL);
    stream.flush();

    assertEqual(cli.printlnStr(F("12345678"), -4), (size_t)10);
    assertEqual(stream.printerText(), "12345678" NL);
    stream.flush();
}

test(printTest, printStr_P) {
    FakeStream stream;
    Cli cli;
    cli.begin(stream);

    assertEqual(cli.printStr_P(PSTR("1234")), (size_t)4);
    assertEqual(stream.printerText(), "1234");
    stream.flush();

    assertEqual(cli.printStr_P(PSTR("1234"), 8), (size_t)8);
    assertEqual(stream.printerText(), "    1234");
    stream.flush();

    assertEqual(cli.printStr_P(PSTR("1234"), -8), (size_t)8);
    assertEqual(stream.printerText(), "1234    ");
    stream.flush();

    assertEqual(cli.printStr_P(PSTR("12345678"), 4), (size_t)8);
    assertEqual(stream.printerText(), "12345678");
    stream.flush();

    assertEqual(cli.printStr_P(PSTR("12345678"), -4), (size_t)8);
    assertEqual(stream.printerText(), "12345678");
    stream.flush();
}

test(printTest, printlnStr_P) {
    FakeStream stream;
    Cli cli;
    cli.begin(stream);

    assertEqual(cli.printlnStr_P(PSTR("1234")), (size_t)6);
    assertEqual(stream.printerText(), "1234" NL);
    stream.flush();

    assertEqual(cli.printlnStr_P(PSTR("1234"), 8), (size_t)10);
    assertEqual(stream.printerText(), "    1234" NL);
    stream.flush();

    assertEqual(cli.printlnStr_P(PSTR("1234"), -8), (size_t)10);
    assertEqual(stream.printerText(), "1234    " NL);
    stream.flush();

    assertEqual(cli.printlnStr_P(PSTR("12345678"), 4), (size_t)10);
    assertEqual(stream.printerText(), "12345678" NL);
    stream.flush();

    assertEqual(cli.printlnStr_P(PSTR("12345678"), -4), (size_t)10);
    assertEqual(stream.printerText(), "12345678" NL);
    stream.flush();
}

test(printTest, backspace) {
    FakeStream stream;
    Cli cli;
    cli.begin(stream);

    assertEqual(cli.printStr("abc"), (size_t)3);
    assertEqual(stream.printerText(), "abc");
    assertEqual(cli.backspace(), (size_t)3);
    assertEqual(stream.printerText(), "abc" BS);
    stream.flush();

    assertEqual(cli.printStr("abc"), (size_t)3);
    assertEqual(stream.printerText(), "abc");
    assertEqual(cli.backspace(2), (size_t)6);
    assertEqual(stream.printerText(), "abc" BS BS);
    stream.flush();

    assertEqual(cli.printStr("abc"), (size_t)3);
    assertEqual(stream.printerText(), "abc");
    assertEqual(cli.backspace(4), (size_t)12);
    assertEqual(stream.printerText(), "abc" BS BS BS BS);
    stream.flush();
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
