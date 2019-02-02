target   = out
main     = $(source_dir)/main.c
language = TINY

ANTLR      = java -jar bin/antlr-3.5.2-complete.jar
ANTLRFLAGS = -o $(antlr_dir)

include       = include
antlr_source  = $(patsubst %,$(antlr_dir)/$(language)%,Lexer.c Parser.c)
antlr_objects = $(patsubst $(antlr_dir)/%.c,$(build)/%.o,$(antlr_source))

source_dir   = src
source       = $(filter-out $(main),$(wildcard $(source_dir)/*.c))
objects      = $(patsubst $(source_dir)/%.c,$(build)/%.o,$(source))
grammar_file = $(language).g
antlr_dir    = antlr
build        = build

override CFLAGS  := -Isrc -I$(antlr_dir) -I$(include) -Wall -Wextra $(CFLAGS)
override LDLIBS  := $(LDLIBS)
override LDFLAGS := $(LDFLAGS)

$(target): $(main) $(build) $(antlr_dir) $(antlr_objects) $(objects)
	$(CC) $(CFLAGS) -o $@ $< $(LDLIBS) $(LDFLAGS)

debug:
	echo $(objects)
	echo $(antlr_objects)

$(antlr_source): %: $(antlr_dir)

$(antlr_objects): $(build)/%.o: $(antlr_dir)/%.c
	$(CC) -c $(CFLAGS) -o $@ $^ $(LDLIBS) $(LDFLAGS)

$(objecs): $(build)/%.o: $(source_dir)/%.c
	$(CC) -c $(CFLAGS) -o $@ $^ $(LDLIBS) $(LDFLAGS)

$(antlr_dir): $(grammar_file)
	@echo "Building ANTLR grammar ..."
	$(ANTLR) $(ANTLRFLAGS) $^ 2>&1 | (grep -v '^warning(24):' || true)
	@echo "Done"

$(build):
	mkdir $(build)

clean:
	rm -rf $(antlr_dir) $(build)

tags: $(antlr_dir)
	ctags $(antlr_source) $(source)

$(VERBOSE).SILENT:
