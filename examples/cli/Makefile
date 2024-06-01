# Copyright 2021 Tadashi G. Takaoka
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

help:
	@echo '"make clean"    remove unnecessary files'
	@echo '"make pio"      run PlatformIO CI'

ifdef INSIDE_EMACS
ifeq ($(filter $(PIO_FLAGS),"--no-ansi"),)
PIO_FLAGS += --no-ansi
endif
endif

ENVS=$(shell grep -Po '^\[env:\K[^]]+' platformio.ini)
BOARDS=$(shell grep -Po '^board *= *\K[\w]+' platformio.ini)

define pio-ci # board
	pio $(PIO_FLAGS) ci -l ../.. $(PIO_CI_FLAGS) -b $(1) cli.ino

endef

pio:
	$(foreach board,$(BOARDS),$(call pio-ci,$(board)))

pio-boards: platformio.ini
	@echo $(BOARDS)
pio-envs: platformio.ini
	@echo $(ENVS)

clean: 
	rm -rf $$(find . -type d -a -name .pio)

.PHONY: help clean arduino pio

# Local Variables:
# mode: makefile-gmake
# End:
# vim: set ft=make:
