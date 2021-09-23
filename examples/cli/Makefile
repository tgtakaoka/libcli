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

PIO_ENVS=$(shell grep -Po '^\[env:\K[^]]+' platformio.ini)
LIB_ENVS=$(PIO_ENVS:%=.pio/libdeps/%)
PIO_BOARDS=$(shell grep -Po '^board *= *\K[\w]+' platformio.ini)

install_local_lib:
	for lib in $(LIB_ENVS); do \
	    test -h $${lib}/libcli && continue; \
	    mkdir -p $${lib} && rm -rf $${lib}/libcli; \
	    ln -s $${PWD}/../.. $${lib}/libcli; \
	done

pio: install_local_lib
	pio --no-ansi run $(PIO_ENVS:%=-e %)

pio-boards: platformio.ini
	@echo $(PIO_BOARDS)
pio-envs: platformio.ini
	@echo $(PIO_ENVS)

clean: 
	rm -rf $$(find . -type d -a -name .pio)

.PHONY: help clean arduino pio

# Local Variables:
# mode: makefile-gmake
# End:
# vim: set ft=make:
