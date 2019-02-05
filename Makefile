target   = test
language = TINY

source_dir  = src
runtime_dir = runtime
antlr_dir   = antlr
build_dir   = build

main            = $(source_dir)/main.cpp
source          = $(filter-out $(main),$(shell find $(source_dir) -type f -name '*.cpp'))
objects         = $(patsubst %.cpp,$(build_dir)/%.o,$(source))
antlr_source    = $(patsubst %,$(antlr_dir)/$(language)%.cpp,Lexer Parser)
antlr_objects   = $(patsubst %.cpp,$(build_dir)/%.o,$(antlr_source))
runtime_source  = $(shell find $(runtime_dir) -type f -name '*.cpp')
runtime_objects = $(patsubst %.cpp,$(build_dir)/%.o,$(runtime_source))

ANTLR      = java -jar antlr-4.7.2-complete.jar
ANTLRFLAGS = -Dlanguage=Cpp -no-listener -no-visitor

override CXXFLAGS := -Wall -Wextra -I$(source_dir) -I$(antlr_dir) -I$(runtime_dir) $(CXXFLAGS)
override LDLIBS   := $(LDLIBS)
override LDFLAGS  := $(LDFLAGS)

$(build_dir)/$(target): $(main) $(antlr_objects) $(runtime_objects) $(objects) | $(build_dir)
	@echo "  CXX   $@"
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDLIBS) $(LDFLAGS)

$(objects): $(build_dir)/%.o: %.cpp | $(build_dir)/$(source_dir)
	@echo "  CXX   $@"
	$(CXX) -c $(CXXFLAGS) -o $@ $^ $(LDLIBS) $(LDFLAGS)

$(antlr_objects): $(build_dir)/%.o: %.cpp | $(build_dir)/$(antlr_dir)
	@echo "  CXX   $@"
	$(CXX) -c $(CXXFLAGS) -iquote $(runtime_dir) -o $@ $^ $(LDLIBS) $(LDFLAGS)

$(runtime_objects): $(build_dir)/%.o: %.cpp | $(build_dir)/$(runtime_dir)
	@echo "  CXX   $@"
	$(CXX) -c $(CXXFLAGS) -o $@ $^ $(LDLIBS) $(LDFLAGS)

$(antlr_source): $(antlr_dir)

$(antlr_dir): $(language).g
	@echo "  ANTLR $^"
	$(ANTLR) $(ANTLRFLAGS) $< -o $(antlr_dir)

$(build_dir):
	@echo "* MKDIR $@"
	mkdir -p $@

$(build_dir)/%:
	@echo "  MKDIR $@"
	mkdir -p $@
	find $* -type d -exec mkdir -p $(build_dir)/{} \;

clean:
	rm -rf $(antlr_dir) $(build_dir)

tags:
	ctags -R $(antlr_source) $(source_dir) $(runtime_dir)

$(VERBOSE).SILENT:
