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
using State = libcli::Cli::State;
using LetterCallback = libcli::Cli::LetterCallback;
using StringCallback = libcli::Cli::StringCallback;
using FakeStream = libcli::fake::FakeStream;

void inject(Cli &cli, int n = 10) {
    while (--n >= 0)
        cli.loop();
}

test(ReadTextTest, readLetter) {
    FakeStream stream;
    Cli cli;
    cli.begin(stream);

    const LetterCallback callback = [](char letter, uintptr_t context) {
        *reinterpret_cast<char *>(context) = letter;
    };

    char result = 0;
    cli.readLetter(callback, reinterpret_cast<uintptr_t>(&result));
    inject(cli);
    assertEqual(result, (char)0);

    stream.setInput('x');
    inject(cli);
    assertEqual(result, 'x');
    assertEqual(stream.printerText(), "");
}

struct Result {
    char *text = nullptr;
    State state;
    uintptr_t context() { return reinterpret_cast<uintptr_t>(this); }
    void set(char *t, State s) {
        text = t;
        state = s;
    }
    static const StringCallback callback;
};

const StringCallback Result::callback = [](char *text, uintptr_t context, State state) {
    reinterpret_cast<Result *>(context)->set(text, state);
};

test(ReadTextTest, readWord) {
    FakeStream stream;
    Cli cli;
    cli.begin(stream);

    char buffer[10];
    Result result;
    cli.readWord(Result::callback, result.context(), buffer, sizeof(buffer));
    assertEqual(buffer, "");                // buffer is initialized
    assertEqual(stream.printerText(), "");  // nothing printted
    inject(cli);
    assertEqual(result.text, (char *)nullptr);  // no callback

    stream.setInput("word");
    inject(cli);
    assertEqual(stream.printerText(), "word");  // inputted text are printted
    assertEqual(result.text, (char *)nullptr);  // no callback

    stream.setInput(' ');  // word delimitter
    inject(cli);
    assertEqual(stream.printerText(), "word ");   // delimiter is printted
    assertEqual(result.text, buffer);             // callback happens
    assertEqual(result.state, State::CLI_SPACE);  // delimmitted by space
    assertEqual(buffer, "word");
}

test(ReadTextTest, readWord_newline) {
    FakeStream stream;
    Cli cli;
    cli.begin(stream);

    char buffer[10];
    Result result;
    cli.readWord(Result::callback, result.context(), buffer, sizeof(buffer));
    assertEqual(buffer, "");                // buffer is initialized
    assertEqual(stream.printerText(), "");  // nothing printted
    inject(cli);
    assertEqual(result.text, (char *)nullptr);  // no callback

    stream.setInput("word");
    inject(cli);
    assertEqual(stream.printerText(), "word");  // inputted text are printted
    assertEqual(result.text, (char *)nullptr);  // no callback

    stream.setInput('\n');  // word newline
    inject(cli);
    assertEqual(stream.printerText(), "word ");     // space is printted
    assertEqual(result.text, buffer);               // callback happens
    assertEqual(result.state, State::CLI_NEWLINE);  // delimmitted by space
    assertEqual(buffer, "word");
}

test(ReadTextTest, readWord_defaultValue) {
    FakeStream stream;
    Cli cli;
    cli.begin(stream);

    char buffer[10];
    strcpy(buffer, "word");
    cli.printStr(buffer);
    assertEqual(stream.printerText(), "word");
    stream.flush();

    Result result;
    cli.readWord(Result::callback, result.context(), buffer, sizeof(buffer), true);
    assertEqual(buffer, "word");            // default value is in buffer
    assertEqual(stream.printerText(), "");  // nothing printted
    inject(cli);
    assertEqual(result.text, (char *)nullptr);

    stream.setInput(' ');
    inject(cli);
    assertEqual(stream.printerText(), " ");
    assertEqual(result.text, "word");
    assertEqual(result.state, State::CLI_SPACE);
}

test(ReadTextTest, readWord_edit) {
    FakeStream stream;
    Cli cli;
    cli.begin(stream);

    char buffer[10];
    strcpy(buffer, "word");
    cli.printStr(buffer);
    assertEqual(stream.printerText(), "word");
    stream.flush();

    Result result;
    cli.readWord(Result::callback, result.context(), buffer, sizeof(buffer), true);
    stream.setInput("\b\bR\brD ");
    inject(cli);
    assertEqual(stream.printerText(), BS BS "R" BS "rD ");
    assertEqual(result.text, "worD");
    assertEqual(result.state, State::CLI_SPACE);
}

test(ReadTextTest, readWord_delete) {
    FakeStream stream;
    Cli cli;
    cli.begin(stream);

    char buffer[10];
    strcpy(buffer, "word");
    cli.printStr(buffer);
    assertEqual(stream.printerText(), "word");
    stream.flush();

    Result result;
    cli.readWord(Result::callback, result.context(), buffer, sizeof(buffer), true);
    stream.setInput("\b\b\b\b\b");  // delete word and previous space delimtter
    inject(cli);
    assertEqual(stream.printerText(), BS BS BS BS);
    assertEqual(result.text, "");
    assertEqual(result.state, State::CLI_DELETE);
}

test(ReadTextTest, readWord_cancel) {
    FakeStream stream;
    Cli cli;
    cli.begin(stream);

    char buffer[10];
    Result result;
    cli.readWord(Result::callback, result.context(), buffer, sizeof(buffer));
    stream.setInput("word\x03");  // C-c is cancel
    inject(cli);
    assertEqual(stream.printerText(), "word cancel" NL);
    assertEqual(result.text, "word");
    assertEqual(result.state, State::CLI_CANCEL);
}

test(ReadTextTest, readLine) {
    FakeStream stream;
    Cli cli;
    cli.begin(stream);

    char buffer[10];
    Result result;
    cli.readLine(Result::callback, result.context(), buffer, sizeof(buffer));
    assertEqual(buffer, "");                // buffer is initialized
    assertEqual(stream.printerText(), "");  // nothing printted
    inject(cli);
    assertEqual(result.text, (char *)nullptr);  // no callback

    stream.setInput("w1 w2");
    inject(cli);
    assertEqual(stream.printerText(), "w1 w2");  // inputted text are printted
    assertEqual(result.text, (char *)nullptr);   // no callback

    stream.setInput('\n');  // end of line
    inject(cli);
    assertEqual(stream.printerText(), "w1 w2 ");    // space is printted
    assertEqual(result.text, buffer);               // callback happens
    assertEqual(result.state, State::CLI_NEWLINE);  // delimmitted by space
    assertEqual(buffer, "w1 w2");
}

test(ReadTextTest, readLine_defaultValue) {
    FakeStream stream;
    Cli cli;
    cli.begin(stream);

    char buffer[10];
    strcpy(buffer, "w1 w2");
    cli.printStr(buffer);
    assertEqual(stream.printerText(), "w1 w2");
    stream.flush();

    Result result;
    cli.readLine(Result::callback, result.context(), buffer, sizeof(buffer), true);
    assertEqual(buffer, "w1 w2");           // default value is in buffer
    assertEqual(stream.printerText(), "");  // nothing printted
    inject(cli);
    assertEqual(result.text, (char *)nullptr);

    stream.setInput('\n');  // end of line
    inject(cli);
    assertEqual(stream.printerText(), " ");  // space is printted
    assertEqual(result.text, "w1 w2");
    assertEqual(result.state, State::CLI_NEWLINE);
}

test(ReadTextTest, readLine_edit) {
    FakeStream stream;
    Cli cli;
    cli.begin(stream);

    char buffer[10];
    strcpy(buffer, "w1 w2");
    cli.printStr(buffer);
    assertEqual(stream.printerText(), "w1 w2");
    stream.flush();

    Result result;
    cli.readLine(Result::callback, result.context(), buffer, sizeof(buffer), true);
    stream.setInput("\b\bw3\n");
    inject(cli);
    assertEqual(stream.printerText(), BS BS "w3 ");
    assertEqual(result.text, "w1 w3");
    assertEqual(result.state, State::CLI_NEWLINE);
}

test(ReadTextTest, readLine_delete) {
    FakeStream stream;
    Cli cli;
    cli.begin(stream);

    char buffer[10];
    strcpy(buffer, "w1 w2");
    cli.printStr(buffer);
    assertEqual(stream.printerText(), "w1 w2");
    stream.flush();

    Result result;
    cli.readLine(Result::callback, result.context(), buffer, sizeof(buffer), true);
    stream.setInput("\b\b\b\b\b\b\b\n");  // readLine can't accept CLI_DELETE
    inject(cli);
    assertEqual(stream.printerText(), BS BS BS BS BS " ");
    assertEqual(result.text, "");
    assertEqual(result.state, State::CLI_NEWLINE);
}

test(ReadTextTest, readLine_cancel) {
    FakeStream stream;
    Cli cli;
    cli.begin(stream);

    char buffer[10];
    Result result;
    cli.readLine(Result::callback, result.context(), buffer, sizeof(buffer));
    stream.setInput("w1 w2\x03");  // C-c is cancel
    inject(cli);
    assertEqual(stream.printerText(),
            "w1 w2"
            " cancel" NL);
    assertEqual(result.text, "w1 w2");
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
