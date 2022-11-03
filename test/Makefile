# Copyright 2022 Tadashi G. Takaoka
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
	@echo '"make test"         run tests on host'

TESTS := $(foreach t,$(wildcard */Makefile),$(t:%/Makefile=%))
TEST_BINS := $(foreach t,$(TESTS),$(t)/$(t).out)

define build-test # test
$(1)/$(1).out: $(1)/$(1).ino
	$(MAKE) -C $(1)

endef

$(eval $(foreach t,$(TESTS),$(call build-test,$(t))))

test: $(TEST_BINS)
	@for t in $(TESTS); do \
	    ./$${t}/$${t}.out; \
	done

clean:
	@for t in $(TESTS); do \
	    $(MAKE) -C $${t} clean; \
	done
