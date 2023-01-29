.SILENT: all setup run-testsuite-2-demo clean

CFLAGS = -Wall -Wextra -Werror -pedantic -std=c99
OUTDIR = build

all: run-testsuite-2-demo

setup:
	mkdir -p $(OUTDIR)

run-testsuite-2-demo: setup
	$(CC) $(CFLAGS) testsuite_2_demo.c -o $(OUTDIR)/testsuite_2_demo.o
	./$(OUTDIR)/testsuite_2_demo.o

.PHONY: clean
clean:
	rm -rdf $(OUTDIR)
