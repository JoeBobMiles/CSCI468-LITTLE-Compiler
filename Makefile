
TARGET = TINY.g
BUILD_DIR = build/

default: build

build:
	@echo -n "Building ANTLR grammar ..."
	java -cp ".;./antlr-3.5.2-complete.jar" org.antlr.Tool $(TARGET) -o $(BUILD_DIR)
	@echo " Done"

clean:
	rm -rf build/

$(VERBOSE).SILENT:
