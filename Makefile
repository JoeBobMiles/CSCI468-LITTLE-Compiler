
grammar = TINY.g
build_dir = build

default:
	@echo -n "Building ANTLR grammar ..."
	java -jar antlr-3.5.2-complete.jar $(grammar) -o $(build_dir)
	@echo " Done"

clean:
	rm -rf $(build_dir)

$(VERBOSE).SILENT:
