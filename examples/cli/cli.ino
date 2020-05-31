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
    return cli.readUint16(handleDump, DUMP_ADDRESS, last_addr);
  }
  if (extra == DUMP_ADDRESS) {
    last_addr = value;
    if (state == Cli::State::CLI_SPACE) {
      return cli.readUint8(handleDump, DUMP_LENGTH);
    }
    value = 16;
  }
  if (state == Cli::State::CLI_SPACE) cli.println();
  cli.print(F("dump memory: "));
  cli.printUint16(last_addr);
  cli.print(' ');
  cli.println(value);
  return true;
}

static bool handleMemory(Cli::State state, uint16_t value, uintptr_t extra) {
  uint16_t index = extra;
  if (state == Cli::State::CLI_DELETE) {
    if (extra == MEMORY_ADDRESS) return false;
    cli.backspace();
    if (index == 0)
      return cli.readUint16(handleMemory, MEMORY_ADDRESS, last_addr);
    index--;
    return cli.readUint8(handleMemory, MEMORY_INDEX(index), mem_buffer[index]);
  }
  if (extra == MEMORY_ADDRESS) {
    last_addr = value;
    return cli.readUint8(handleMemory, MEMORY_INDEX(0));
  }

  mem_buffer[index++] = value;
  if (state == Cli::State::CLI_SPACE) {
    if (index < sizeof(mem_buffer))
      return cli.readUint8(handleMemory, MEMORY_INDEX(index));
    cli.println();
  }
  cli.print(F("write memory: "));
  cli.printUint16(last_addr);
  for (uint8_t i = 0; i < index; i++) {
    cli.print(' ');
    cli.printUint8(mem_buffer[i]);
  }
  cli.println();
  return true;
}

static bool handleLoad(Cli::State state, char *line, uintptr_t extra) {
  (void)state;
  (void)extra;
  cli.print(F("load file: "));
  cli.println(line);
  return true;
}

static bool handleCommand(char c, uintptr_t extra) {
  (void)extra;
  if (c == 's') {
    cli.println(F("step"));
    return true;
  }
  if (c == 'd') {
    cli.print(F("dump "));
    return cli.readUint16(handleDump, DUMP_ADDRESS);
  }
  if (c == 'l') {
    cli.print(F("load "));
    return cli.readLine(handleLoad, 0);
  }
  if (c == 'm') {
    cli.print(F("memory "));
    return cli.readUint16(handleMemory, MEMORY_ADDRESS);
  }
  if (c == '?') {
    cli.println(F("s(tep) d(ump) l(oad) m(emory)"));
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
