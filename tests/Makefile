.SILENT: all setup clean

CFLAGS = -Wall -Wextra -Werror -Wpedantic -std=c99
OUTDIR = build

all: anv_metalloc anv_arr

setup:
	mkdir -p $(OUTDIR)

anv_metalloc: setup
	$(CC) $(CFLAGS) anv_metalloc.c -o $(OUTDIR)/anv_metalloc.o
	./$(OUTDIR)/anv_metalloc.o

anv_arr: setup
	$(CC) $(CFLAGS) anv_arr.c -o $(OUTDIR)/anv_arr.o
	./$(OUTDIR)/anv_arr.o

.PHONY: clean
clean:
	rm -rdf $(OUTDIR)
