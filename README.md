# CSCI468 Project

## Setup

You will need at minimum a text editor, a terminal, GNU Make, a JRE, and a JDK
to work with this project (as it currently stands).

## Working with ANTLR TestRig

When the Makefile runs a project, it will use the ANTLR TestRig. This
application takes in any input and save it to a buffer to be evaluated. TestRig
waits for an EOF (End of File) to indicate to it that you are done typing the
text you want it to evaluate. This varies by system, on *nix systems, it is ^D
(Ctrl-D). On Windows, it is ^Z (Ctrl-Z). Once you have typed this character,
you will need to indicate to TestRig that you want to proceed by hitting the
Enter key. This will cause TestRig to display the lexer's output.
