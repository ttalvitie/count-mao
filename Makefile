CXX ?= g++
CFLAGS_COMMON ?= -std=c++17 -Wall -Wno-sign-compare -march=native -I.
CFLAGS_DEBUG ?= $(CFLAGS_COMMON) -g
CFLAGS_RELEASE ?= $(CFLAGS_COMMON) -DNDEBUG -O2
LDFLAGS ?= -lgmpxx -lgmp -pthread
BIN_SRCS := $(shell find . -name '*.bin.cpp' | sed -e 's/^\.\///')
ALL_SRCS := $(shell find . -name '*.cpp' | sed -e 's/^\.\///')
BINS := $(BIN_SRCS:%.bin.cpp=%)
SKIP_BINS ?= test/%.release
DEBUG_BINS := $(filter-out $(SKIP_BINS),$(BINS:%=%.debug))
RELEASE_BINS := $(filter-out $(SKIP_BINS),$(BINS:%=%.release))

MAO_COUNT_MODULES := $(shell find mao -name '*.cpp' | sed -e 's/.cpp$$//')

.PHONY: clean all debug release

all: debug release

debug: $(DEBUG_BINS)

release: $(RELEASE_BINS)

define BIN_RULE
$(1).debug: $(1).bin.debug.o $(DEPS_$(1):%=%.debug.o) $(MAO_COUNT_MODULES:%=%.debug.o)
	$(CXX) $(CFLAGS_DEBUG) $(1).bin.debug.o $(DEPS_$(1):%=%.debug.o) $(MAO_COUNT_MODULES:%=%.debug.o) -o $(1).debug $(LDFLAGS)
$(1).release: $(1).bin.release.o $(DEPS_$(1):%=%.release.o) $(MAO_COUNT_MODULES:%=%.release.o)
	$(CXX) $(CFLAGS_RELEASE) $(1).bin.release.o $(DEPS_$(1):%=%.release.o) $(MAO_COUNT_MODULES:%=%.release.o) -o $(1).release $(LDFLAGS)
endef
$(foreach b,$(BINS),$(eval $(call BIN_RULE,$(b))))

%.debug.o: %.cpp
	$(CXX) $(CFLAGS_DEBUG) -MMD -c $< -o $@

%.release.o: %.cpp
	$(CXX) $(CFLAGS_RELEASE) -MMD -c $< -o $@

clean:
	rm -f $(ALL_SRCS:%.cpp=%.debug.d) $(ALL_SRCS:%.cpp=%.release.d) $(ALL_SRCS:%.cpp=%.debug.o) $(ALL_SRCS:%.cpp=%.release.o) $(BIN_SRCS:%.bin.cpp=%.debug) $(BIN_SRCS:%.bin.cpp=%.release)

-include $(ALL_SRCS:%.cpp=%.debug.d) $(ALL_SRCS:%.cpp=%.release.d)
