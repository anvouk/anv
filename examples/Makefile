.SILENT: all setup run-testsuite-2-demo run-stb-ds-demo-auto clean

CFLAGS = -Wall -Wextra -Wpedantic -std=c99
OUTDIR = build

all: run-testsuite-2-demo run-stb-ds-demo-auto

setup:
	mkdir -p $(OUTDIR)

run-testsuite-2-demo: setup
	$(CC) $(CFLAGS) testsuite_2_demo.c -o $(OUTDIR)/testsuite_2_demo.o
	./$(OUTDIR)/testsuite_2_demo.o

run-stb-ds-demo-auto: setup
	$(CC) $(CFLAGS) stb_ds_demo_auto.c -o $(OUTDIR)/stb_ds_demo_auto.o
	./$(OUTDIR)/stb_ds_demo_auto.o

.PHONY: clean
clean:
	rm -rdf $(OUTDIR)
