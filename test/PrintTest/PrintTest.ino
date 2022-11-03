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

test(printTest, printHex) {
    libcli::fake::FakeStream stream;
    auto &cli = libcli::Cli::instance();
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
    libcli::fake::FakeStream stream;
    auto &cli = libcli::Cli::instance();
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
    libcli::fake::FakeStream stream;
    auto &cli = libcli::Cli::instance();
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
    libcli::fake::FakeStream stream;
    auto &cli = libcli::Cli::instance();
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

test(printTest, printStr) {
    libcli::fake::FakeStream stream;
    auto &cli = libcli::Cli::instance();
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
    libcli::fake::FakeStream stream;
    auto &cli = libcli::Cli::instance();
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
    libcli::fake::FakeStream stream;
    auto &cli = libcli::Cli::instance();
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
    libcli::fake::FakeStream stream;
    auto &cli = libcli::Cli::instance();
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
    libcli::fake::FakeStream stream;
    auto &cli = libcli::Cli::instance();
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
    libcli::fake::FakeStream stream;
    auto &cli = libcli::Cli::instance();
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
    libcli::fake::FakeStream stream;
    auto &cli = libcli::Cli::instance();
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
