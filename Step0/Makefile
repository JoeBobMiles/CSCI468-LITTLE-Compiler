default: build run

build:
	@echo -n "Building ANTLR grammar ..."
	antlr4 *.g4
	javac -cp ".;../antlr-4.7.2-complete.jar" *.java
	@echo " Done"

run:
	@echo "Running ANTLR TestRig..."
	java -cp ".;../antlr-4.7.2-complete.jar" \
		org.antlr.v4.gui.TestRig hello r -tree

clean:
	rm -f *.interp *.java *.tokens *.class

$(VERBOSE).SILENT:
