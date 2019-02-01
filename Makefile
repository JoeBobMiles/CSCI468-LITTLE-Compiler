
default: build

build:
	@echo -n "Building ANTLR grammar ..."
	java -cp ".;./antlr-3.5.2-complete.jar" org.antlr.Tool *.g -o build/
	@echo " Done"

clean:
	rm -rf build/

$(VERBOSE).SILENT:
