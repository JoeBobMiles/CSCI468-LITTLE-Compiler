default: | build clean

build:
	antlr4 $(wildcard *.g4)
	javac Hello*.java

clean:
	rm -f *.interp *.java *.tokens
