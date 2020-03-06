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

#include <stdint.h>

class Cli Cli;

static bool isBackspace(char c) {
  return c == '\b' || c == '\x7f';
}

static bool isEscape(char c) {
  return c == '\x1b';
}

static bool isNewline(char c) {
  return c == '\n' || c == '\r';
}

static size_t printHex4(Stream &console, uint8_t val8) {
  val8 &= 0x0f;
  val8 += (val8 < 10) ? '0' : ('A' - 10);
  return console.print(static_cast<char>(val8));
}

static size_t printHex8(Stream &console, uint8_t val8) {
  const size_t n = printHex4(console, val8 >> 4);
  return printHex4(console, val8) + n;
}

size_t Cli::printUint8(uint8_t val8) {
  return printHex8(*_console, val8);
}

size_t Cli::printUint16(uint16_t val16) {
  const size_t n = printUint8(static_cast<uint8_t>(val16 >> 8));
  return printUint8(static_cast<uint8_t>(val16 >> 0)) + n;
}

size_t Cli::backspace(int8_t n) {
  size_t s = 0;
  while (n-- > 0)
    s += print(F("\b \b"));
  return s;
}

void Cli::readUint8(InputHandler handler, uintptr_t extra) {
  readUint(handler, extra, -2);
}

void Cli::readUint16(InputHandler handler, uintptr_t extra) {
  readUint(handler, extra, -4);
}

void Cli::readUint8(InputHandler handler, uintptr_t extra, uint8_t value) {
  readUint(handler, extra, 2, value);
}

void Cli::readUint16(InputHandler handler, uintptr_t extra, uint16_t value) {
  readUint(handler, extra, 4, value);
}

void Cli::readCommand() {
  _mode = READ_COMMAND;
  _printPrompt = true;
}

void Cli::readCommand(CommandHandler handler) {
  _commandHandler = handler;
  readCommand();
}

void Cli::readLetter(InputHandler handler, uintptr_t extra) {
  _mode = READ_CHAR;
  _inputHandler = handler;
  _extra = extra;
}

void Cli::readLine(LineHandler handler, uintptr_t extra) {
  _mode = READ_LINE;
  _lineHandler = handler;
  _lineLen = 0;
  *_lineBuffer = 0;
  _extra = extra;
}

void Cli::readUint(InputHandler handler, uintptr_t extra, int8_t digits, uint16_t value) {
  if (digits < 0) {
    _uintLen = 0;
    _uintDigits = -digits;
    _uintValue = 0;
  } else {
    backspace(digits);
    setUint(digits, value);
  }
  _inputHandler = handler;
  _extra = extra;
  _mode = READ_UINT;
}

void Cli::setUint(uint8_t digits, uint16_t value) {
  _uintLen = _uintDigits = digits;
  _uintValue = value;
  if (digits == 2) {
    printUint8(static_cast<uint8_t>(value));
  } else {
    printUint16(value);
  }
}

void Cli::processUint(char c) {
  if (isHexadecimalDigit(c) && _uintLen < _uintDigits) {
    c = toUpperCase(c);
    print(c);
    _uintLen++;
    _uintValue *= 16;
    _uintValue += isDigit(c) ? c - '0' : c - 'A' + 10;
  } else if (isBackspace(c)) {
    if (_uintLen == 0) {
      if (_inputHandler(CLI_DELETE, 0, _extra))
        readCommand();
      return;
    }
    backspace();
    _uintValue /= 16;
    _uintLen--;
  } else if (isSpace(c) && _uintLen > 0) {
    backspace(_uintLen);
    setUint(_uintDigits, _uintValue);
    State state;
    if (isNewline(c)) {
      println();
      state = CLI_NEWLINE;
    } else {
      print(' ');
      state = CLI_SPACE;
    }
    if (_inputHandler(state, _uintValue, _extra))
      readCommand();
  } else if (isEscape(c)) {
    println(F(" cancel"));
    readCommand();
  }
}

void Cli::processLine(char c) {
  if (isNewline(c)) {
    println();
    if (_lineHandler(CLI_NEWLINE, _lineBuffer, _extra))
      readCommand();
  } else if (isBackspace(c)) {
    if (_lineLen > 0) {
      backspace();
      _lineBuffer[--_lineLen] = 0;
    }
  } else if (isEscape(c)) {
    println(F(" cancel"));
    readCommand();
  } else if (_lineLen < sizeof(_lineBuffer) - 1) {
    print(c);
    _lineBuffer[_lineLen++] = c;
    _lineBuffer[_lineLen] = 0;
  }
}

void Cli::processCommand(char c) {
  if (_commandHandler(c))
    readCommand();
}

void Cli::processLetter(char c) {
  if (_inputHandler(CLI_NEWLINE, c, _extra))
    readCommand();
}

void Cli::setPrompt(void (*prompt)(Stream&)) {
  _prompt = prompt;
}

void Cli::begin(Stream &console) {
  _console = &console;
}

void Cli::loop() {
  if (_console->available()) {
    const char c = _console->read();
    switch (_mode) {
    case READ_COMMAND:
      processCommand(c);
      break;
    case READ_UINT:
      processUint(c);
      break;
    case READ_CHAR:
      processLetter(c);
      break;
    case READ_LINE:
      processLine(c);
      break;
    }
  }
  if (_printPrompt && _prompt) {
    _printPrompt = false;
    _prompt(*_console);
  }
}

// Local Variables:
// mode: c++
// c-basic-offset: 2
// tab-width: 2
// End:
// vim: set ft=cpp et ts=2 sw=2:
