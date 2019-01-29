
default: build # run

build:
	@echo -n "Building ANTLR grammar ..."
	java -cp ".;../antlr-3.5.2-complete.jar" org.antlr.Tool -o . *.g4
	@echo " Done"

run:
	@echo "Running ANTLR TestRig..."
	java -cp ".;../antlr-4.7.2-complete.jar" \
		org.antlr.v4.gui.TestRig hello r -tree

clean:
	rm -f *.interp *.java *.tokens *.class

$(VERBOSE).SILENT:
