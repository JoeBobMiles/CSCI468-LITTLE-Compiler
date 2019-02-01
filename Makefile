
default: build

build:
	@echo -n "Building ANTLR grammar ..."
	java -cp ".;./antlr-3.5.2-complete.jar" org.antlr.Tool *.g
	@echo " Done"

clean:
	rm -f *.interp *.tokens *.h *.c

$(VERBOSE).SILENT:
