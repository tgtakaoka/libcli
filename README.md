![<https://github.com/tgtakaoka/libcli/blob/main/LICENSE.md>](https://img.shields.io/badge/License-Apache%202.0-blue.svg)
![<https://github.com/tgtakaoka/libcli/releases>](https://img.shields.io/github/v/release/tgtakaoka/libcli.svg?maxAge=3600)
![badge](https://github.com/tgtakaoka/libcli/actions/workflows/ccpp.yml/badge.svg)
![badge](https://github.com/tgtakaoka/libcli/actions/workflows/arduino-ci.yml/badge.svg)
![<https://github.com/tgtakaoka/libcli/actions/workflows/platformio-ci.yml>](https://github.com/tgtakaoka/libcli/actions/workflows/platformio-ci.yml/badge.svg)

# libcli for Arduino IDE

Support library to help implementing asynchronous command line
interface.

Because Arduino API for serial console is mostly synchronous and
blocking, it is not easy to write a sketch which accepts human
interaction from serial console while controlling hardware etc. Of
course we can use `Serial.available()` in `loop()` and carefully use
`Serial.read()` not to block program flow.

So `libcli` library comes for help. This library offers a Cli instance
as a command line interface which can

  - read a letter, a word string, a line of string, decimal,
    hexadecimal, octal, and binary number from serial console
    asynchronously.

  - act as `Stream` and do any form of `print()` and `println()`.

  - print decimal number with left or right aligned in a specified width
    format, such as `"%-6d"` or `"%6d"` in `printf()`.

  - print hexadecimal, octal, and binary number in specified
    fixed-width format, such as `"%08X"` `"%04O"` in `printf()`.

  - print string with left or right aligned in a specified width, such
    as `"%-8s"` or `"%8s"` in `printf()`.

Asynchronous read can be implemented with ***request*** and
***callback***. For example, to read a 16-bit hexadecimal, call
`readHex()` ***request*** with a `callback` function pointer,
`UINT16_MAX` as a `limit` of input, and a `context`. When human
finishes input with space or enter key, the `callback` function will
be called with input `value`, the `context` passed in the
***request***, and a input `state`.

You can find concrete examples at
[libasm](https://github.com/tgtakaoka/libasm/blob/main/src/arduino_example.h)
and
[retro-bionic](https://github.com/tgtakaoka/retro-bionic/blob/main/debugger/debugger.cpp).

``` C++
libcli::Cli cli;
using State = libcli::State;

/** NumberCallback function */
void handleHex16(uint32_t value, uintptr_t context, State state) {
    if (state == State::CLI_SPACE || state == State::CLI_ENTER) {
        cli.printHex(value, sizeof(uint16_t) * 2);
    }
}

void begin() {
    Serial.begin(9600);
    cli.begin(Serial);
    cli.print(F("16bit Hex? "));
    cli.readHex(handleHex16, context, UINT16_MAX);
}

void loop() {
    cli.loop();
    /* do other stuff */
}
```

The version 1.4 API has the following functions.

``` C++
/** void (*LetterCallback)(char letter, uintptr_t context); */
using LetterCallback = libcli::LetterCallback;
void readLetter(LetterCallback callback, uintptr_t context);

/** void (*StringCallback)(char *string, uintptr_t context, State state); */
using StringCallback = libcli::StringCallback;
void readWord(StringCallback callback, uintptr_t context, char *buffer, size_t size, bool hasDefval = false);
void readLine(StringCallback callback, uintptr_t context, char *buffer, size_t size, bool hasDefval = false);

/** void (*NumberCallback)(uint32_t number, uintptr_t context, State state); */
using NumberCallback = libcli::NumberCallback;
void readHex(NumberCallback callback, uintptr_t context, uint32_t limit = UINT32_MAX);
void readHex(NumberCallback callback, uintptr_t context, uint32_t limit, uint32_t defVal);
void readDec(NumberCallback callback, uintptr_t context, uint32_t limit = UINt32_MAX);
void readDec(NumberCallback callback, uintptr_t context, uint32_t limit, uint32_t defVal);
void readNum(NumberCallback callback, uintptr_t context, uint8_t radix = 10, uint32_t limit = UINt32_MAX);
void readNum(NumberCallback callback, uintptr_t context, uint8_t radix = 10, uint32_t limit, uint32_t defVal);

void printStr(const char *text, int8_t width = 0);
void printStr(const __FlashStringHelper *text, int8_t width = 0);
void printStr_P(const /*PROGMEM*/ char *text_P, int8_t width = 0);
void printHex(uint32_t number, int8_t width = 0);
void printDec(uint32_t number, int8_t width = 0);
void printNum(uint32_t number, uint8_t radix = 10, int8_t width = 0);
void printlnStr(const __FlashStringHelper *text, int8_t width = 0);
void printlnStr_P(const /*PROGMEM*/ char *text_P, int8_t width = 0);
void printlnHex(uint32_t number, int8_t width = 0);
void printlnDec(uint32_t number, int8_t width = 0);
void printlnNum(uint32_t number, uint8_t radix = 10, int8_t width = 0);
void backspace(int8_t n = 1);
```

<div class="note">

More information about this library can be found at
[GitHub](https://github.com/tgtakaoka/libcli)

</div>
