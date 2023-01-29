.SILENT: all setup anv_metalloc clean

CFLAGS = -Wall -Wextra -Werror -pedantic -std=c99
OUTDIR = build

all: anv_metalloc

setup:
	mkdir -p $(OUTDIR)

anv_metalloc: setup
	$(CC) $(CFLAGS) anv_metalloc.c -o $(OUTDIR)/anv_metalloc.o
	./$(OUTDIR)/anv_metalloc.o

.PHONY: clean
clean:
	rm -rdf $(OUTDIR)
