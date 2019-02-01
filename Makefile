
TARGET = TINY.g
BUILD_DIR = build/

default:
	@echo -n "Building ANTLR grammar ..."
	java -jar antlr-3.5.2-complete.jar $(TARGET) -o $(BUILD_DIR)
	@echo " Done"

clean:
	rm -rf build/

$(VERBOSE).SILENT:
