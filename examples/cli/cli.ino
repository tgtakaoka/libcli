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

static uint16_t last_addr;
#define DUMP_ADDRESS 0
#define DUMP_LENGTH  1
static uint8_t mem_buffer[4];
#define MEMORY_ADDRESS (sizeof(mem_buffer) + 0x10)
#define MEMORY_INDEX(index) (index)

static bool handleDump(Cli::State state, uint16_t value, uintptr_t extra) {
  if (state == Cli::State::CLI_DELETE) {
    if (extra == DUMP_ADDRESS) return false;
    Cli.backspace();
    return Cli.readUint16(handleDump, DUMP_ADDRESS, last_addr);
  }
  if (extra == DUMP_ADDRESS) {
    last_addr = value;
    if (state == Cli::State::CLI_SPACE) {
      return Cli.readUint8(handleDump, DUMP_LENGTH);
    }
    value = 16;
  }
  if (state == Cli::State::CLI_SPACE) Cli.println();
  Cli.print(F("dump memory: "));
  Cli.printUint16(last_addr);
  Cli.print(' ');
  Cli.println(value);
  return true;
}

static bool handleMemory(Cli::State state, uint16_t value, uintptr_t extra) {
  uint16_t index = extra;
  if (state == Cli::State::CLI_DELETE) {
    if (extra == MEMORY_ADDRESS) return false;
    Cli.backspace();
    if (index == 0)
      return Cli.readUint16(handleMemory, MEMORY_ADDRESS, last_addr);
    index--;
    return Cli.readUint8(handleMemory, MEMORY_INDEX(index), mem_buffer[index]);
  }
  if (extra == MEMORY_ADDRESS) {
    last_addr = value;
    return Cli.readUint8(handleMemory, MEMORY_INDEX(0));
  }

  mem_buffer[index++] = value;
  if (state == Cli::State::CLI_SPACE) {
    if (index < sizeof(mem_buffer))
      return Cli.readUint8(handleMemory, MEMORY_INDEX(index));
    Cli.println();
  }
  Cli.print(F("write memory: "));
  Cli.printUint16(last_addr);
  for (uint8_t i = 0; i < index; i++) {
    Cli.print(' ');
    Cli.printUint8(mem_buffer[i]);
  }
  Cli.println();
  return true;
}

static bool handleLoad(Cli::State state, char *line, uintptr_t extra) {
  (void)state;
  (void)extra;
  Cli.print(F("load file: "));
  Cli.println(line);
  return true;
}

static bool handleCommand(char c, uintptr_t extra) {
  (void)extra;
  if (c == 's') {
    Cli.println(F("step"));
    return true;
  }
  if (c == 'd') {
    Cli.print(F("dump "));
    return Cli.readUint16(handleDump, DUMP_ADDRESS);
  }
  if (c == 'l') {
    Cli.print(F("load "));
    return Cli.readLine(handleLoad, 0);
  }
  if (c == 'm') {
    Cli.print(F("memory "));
    return Cli.readUint16(handleMemory, MEMORY_ADDRESS);
  }
  if (c == '?') {
    Cli.println(F("s(tep) d(ump) l(oad) m(emory)"));
    return true;
  }
  return false;
}

void prompter(class Cli &console, uintptr_t extra) {
  (void)extra;
  console.print(F("> "));
}

void setup() {
  Console.begin(9600);
  Cli.begin(Console);
  Cli.setPrompter(prompter, 0);
  Cli.readCommand(handleCommand, 0);
}

void loop() {
  Cli.loop();
}

// Local Variables:
// mode: c++
// c-basic-offset: 2
// tab-width: 2
// End:
// vim: set ft=cpp et ts=2 sw=2:
