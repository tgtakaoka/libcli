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

#ifndef __libcli_h__
#define __libcli_h__

#include <Arduino.h>
#include <stdint.h>

class Cli {
public:
  enum State {
    CLI_SPACE,
    CLI_NEWLINE,
    CLI_DELETE,
  };
  typedef bool (*CommandHandler)(char c);
  typedef bool (*InputHandler)(State state, uint16_t value, uintptr_t extra);
  typedef bool (*LineHandler)(State state, char *line, uintptr_t extra);

  void begin(Stream &console);
  void loop();
  void setPrompt(void (*prompt)(Stream &console));

  void readCommand(CommandHandler handler);
  bool readUint8(InputHandler handler, uintptr_t extra);
  bool readUint16(InputHandler handler, uintptr_t extra);
  bool readUint8(InputHandler handler, uintptr_t extra, uint8_t value);
  bool readUint16(InputHandler handler, uintptr_t extra, uint16_t value);
  bool readLetter(InputHandler handler, uintptr_t extra);
  bool readLine(LineHandler hadler, uintptr_t extra);

  size_t backspace(int8_t n = 1);
  size_t printUint8(uint8_t value);
  size_t printUint16(uint16_t value);
  template<typename T>
  size_t print(T value) {
    return _console->print(value);
  }
  template<typename T>
  size_t println(T value) {
    return _console->println(value);
  }
  size_t println() {
    return _console->println();
  }

private:
  Stream *_console;

  enum Mode {
    READ_COMMAND,
    READ_UINT,
    READ_CHAR,
    READ_LINE,
  } _mode;

  CommandHandler _commandHandler;
  InputHandler _inputHandler;
  LineHandler _lineHandler;
  uintptr_t _extra;

  void setUint(uint8_t digits, uint16_t value);
  State appendUint(char c);
  uint8_t _uintLen;
  uint8_t _uintDigits;
  uint16_t _uintValue;

  uint8_t _lineLen;
  char _lineBuffer[80];

  bool _printPrompt;
  void (*_prompt)(Stream&);


  bool readUint(InputHandler, uintptr_t extra, int8_t digits, uint16_t value = 0);
  void readCommand();
  void processCommand(char c);
  void processUint(char c);
  void processLetter(char c);
  void processLine(char c);
};

extern Cli Cli;

#endif

// Local Variables:
// mode: c++
// c-basic-offset: 2
// tab-width: 2
// End:
// vim: set ft=cpp et ts=2 sw=2:
