/*
 * Copyright 2023 Tadashi G. Takaoka
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

#ifndef __LIBCLI_TYPES_H__
#define __LIBCLI_TYPES_H__

#include <stdint.h>

#include <Arduino.h>

namespace libcli {

/** What terminates user input. */
enum State : uint8_t {
    CLI_SPACE,    // an input is terminated by space.
    CLI_NEWLINE,  // an input is terminated by newline.
    CLI_DELETE,   // current input is canceled and back to previous.
    CLI_CANCEL,   // whole input is canceled.
};

/** Callback function of |readLetter|. */
using LetterCallback = void (*)(char letter, uintptr_t context);

/** Callback function of |readWord| and |readLine|. */
using StringCallback = void (*)(char *string, uintptr_t context, State state);

/** Callback function of |readHex| and |readDec|. */
using NumberCallback = void (*)(uint32_t number, uintptr_t context, State state);

}  // namespace libcli

#endif

// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=4:
