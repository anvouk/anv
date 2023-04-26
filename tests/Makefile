.SILENT: all setup clean

CFLAGS = -Wall -Wextra -Werror -Wpedantic -std=c99
OUTDIR = build

all: anv_metalloc anv_arr anv_halloc

setup:
	mkdir -p $(OUTDIR)

anv_metalloc: setup
	$(CC) $(CFLAGS) anv_metalloc.c -o $(OUTDIR)/anv_metalloc.o
	./$(OUTDIR)/anv_metalloc.o

anv_arr: setup
	$(CC) $(CFLAGS) anv_arr.c -o $(OUTDIR)/anv_arr.o
	./$(OUTDIR)/anv_arr.o

anv_halloc: setup
	$(CC) $(CFLAGS) anv_halloc.c -o $(OUTDIR)/anv_halloc.o
	./$(OUTDIR)/anv_halloc.o

.PHONY: clean
clean:
	rm -rdf $(OUTDIR)
