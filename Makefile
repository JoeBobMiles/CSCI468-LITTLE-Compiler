program_name = test
language     = TINY
build_dir    = build

s = .cpp
o = .o
d = .mk
e =

main   = $(source_dir)/main$s
target = $(build_dir)/$(program_name)$e

source_dir = src
source     = $(filter-out $(main),$(shell find $(source_dir) -type f -name "*$s"))
objects    = $(patsubst %$s,$(build_dir)/%$o,$(source))

antlr_dir     = antlr
antlr_source  = $(patsubst %,$(antlr_dir)/$(language)%$s,Lexer Parser)
antlr_objects = $(patsubst %$s,$(build_dir)/%$o,$(antlr_source))

runtime_dir     = runtime
runtime_source  = $(shell find $(runtime_dir) -type f -name "*$s")
runtime_objects = $(patsubst %$s,$(build_dir)/%$o,$(runtime_source))

ANTLR      = java -jar bin/antlr-4.7.2-complete.jar
ANTLRFLAGS = -Dlanguage=Cpp -no-listener -no-visitor

CPPDIRS = -I$(source_dir) -I$(antlr_dir) -I$(runtime_dir) -iquote $(runtime_dir)

override CXXFLAGS := -Wall -Wextra -Wno-attributes $(CPPDIRS) $(CXXFLAGS)
override LDLIBS   := $(LDLIBS)
override LDFLAGS  := $(LDFLAGS)

all: $(target)

$(target): $(main) $(antlr_objects) $(runtime_objects) $(objects)
	@echo "  CXX   $@"
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDLIBS) $(LDFLAGS)

$(build_dir)/%$o: %$s
	@echo "  CXX   $@"
	mkdir -p $(dir $@)
	$(CXX) -c $(CXXFLAGS) -o $@ $^ $(LDLIBS) $(LDFLAGS)

$(antlr_source): $(language).g
	@echo "  ANTLR $^"
	$(ANTLR) $(ANTLRFLAGS) $< -o $(antlr_dir)

.PHONY: clean cleaner tags

clean:
	rm -f $(target)
	[ -e $(build_dir)/$(source_dir) ] && find $(build_dir)/$(source_dir) -type f -not -name "*$d" -delete || true

cleaner:
	rm -rf $(antlr_dir) $(build_dir)

tags:
	ctags -R $(antlr_dir) $(source_dir) $(runtime_dir)

$(VERBOSE).SILENT:
