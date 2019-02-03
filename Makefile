
TARGET = TINY.g
BUILD_DIR = build/

$(BUILD_DIR):
	@echo -n "Building ANTLR grammar ..."
	java -jar antlr-4.7.2-complete.jar -Dlanguage=Cpp $(TARGET) -o $(BUILD_DIR)
	@echo " Done"

clean:
	rm -rf $(BUILD_DIR)

$(VERBOSE).SILENT:
