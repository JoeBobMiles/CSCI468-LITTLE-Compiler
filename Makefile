
source_dir = src
grammar = TINY.g
antlr_dir = 'antlr'
antlr_jar = 'bin/antlr-3.5.2-complete.jar'

$(antlr_dir):
	@echo "Building ANTLR grammar ..."
	java -jar $(antlr_jar) $(grammar) -o $(antlr_dir) 2>&1 | (grep -v '^warning(24):' || true)
	@echo "Done"

clean:
	rm -rf $(antlr_dir)

tags:
	ctags 

$(VERBOSE).SILENT:
