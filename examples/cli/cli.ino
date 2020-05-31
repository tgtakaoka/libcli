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

#include <libcli.h>

#define Console Serial

using libcli::Cli;
Cli cli;

static uint16_t last_addr;
#define DUMP_ADDRESS 0
#define DUMP_LENGTH  1
static uint8_t mem_buffer[4];
#define MEMORY_ADDRESS (sizeof(mem_buffer) + 0x10)
#define MEMORY_INDEX(index) (index)

static bool handleDump(Cli::State state, uint16_t value, uintptr_t extra) {
  if (state == Cli::State::CLI_DELETE) {
    if (extra == DUMP_ADDRESS) return false;
    cli.backspace();
    cli.readUint16(handleDump, DUMP_ADDRESS, last_addr);
    return false;
  }
  if (extra == DUMP_ADDRESS) {
    last_addr = value;
    if (state == Cli::State::CLI_SPACE) {
      cli.readUint8(handleDump, DUMP_LENGTH);
      return false;
    }
    value = 16;
  }
  if (state == Cli::State::CLI_SPACE) cli.println();
  cli.print(F("dump memory: "));
  cli.printUint16(last_addr);
  cli.print(' ');
  cli.println(value);
  cli.readCommand(handleCommand, 0);
  return true;
}

static bool handleMemory(Cli::State state, uint16_t value, uintptr_t extra) {
  uint16_t index = extra;
  if (state == Cli::State::CLI_DELETE) {
    if (extra == MEMORY_ADDRESS) return false;
    cli.backspace();
    if (index == 0) {
      cli.readUint16(handleMemory, MEMORY_ADDRESS, last_addr);
      return false;
    }
    index--;
    cli.readUint8(handleMemory, MEMORY_INDEX(index), mem_buffer[index]);
    return false;
  }
  if (extra == MEMORY_ADDRESS) {
    last_addr = value;
    cli.readUint8(handleMemory, MEMORY_INDEX(0));
    return false;
  }

  mem_buffer[index++] = value;
  if (state == Cli::State::CLI_SPACE) {
    if (index < sizeof(mem_buffer)) {
      cli.readUint8(handleMemory, MEMORY_INDEX(index));
      return false;
    }
    cli.println();
  }
  cli.print(F("write memory: "));
  cli.printUint16(last_addr);
  for (uint8_t i = 0; i < index; i++) {
    cli.print(' ');
    cli.printUint8(mem_buffer[i]);
  }
  cli.println();
  cli.readCommand(handleCommand, 0);
  return true;
}

static bool handleLoad(Cli::State state, char *line, uintptr_t extra) {
  (void)state;
  (void)extra;
  cli.print(F("load file: "));
  cli.println(line);
  cli.readCommand(handleCommand, 0);
  return true;
}

static bool handleCommand(char c, uintptr_t extra) {
  (void)extra;
  if (c == 's') {
    cli.println(F("step"));
    cli.readCommand(handleCommand, 0);
    return true;
  }
  if (c == 'd') {
    cli.print(F("dump "));
    cli.readUint16(handleDump, DUMP_ADDRESS);
  }
  if (c == 'l') {
    cli.print(F("load "));
    cli.readLine(handleLoad, 0);
  }
  if (c == 'm') {
    cli.print(F("memory "));
    cli.readUint16(handleMemory, MEMORY_ADDRESS);
  }
  if (c == '?') {
    cli.print(F("libcli (version "));
    cli.print(LIBCLI_VERSION_STRING);
    cli.println(F(") example"));
    cli.println(F("s(tep) d(ump) l(oad) m(emory)"));
    cli.readCommand(handleCommand, 0);
    return true;
  }
  return false;
}

void prompter(Cli &console, uintptr_t extra) {
  (void)extra;
  console.print(F("> "));
}

void setup() {
  Console.begin(9600);
  cli.begin(Console);
  cli.setPrompter(prompter, 0);
  cli.readCommand(handleCommand, 0);
}

void loop() {
  cli.loop();
}

// Local Variables:
// mode: c++
// c-basic-offset: 2
// tab-width: 2
// End:
// vim: set ft=cpp et ts=2 sw=2:
