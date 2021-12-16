.SILENT: all all-build all-run clean

CFLAGS = -Wall -Wextra -Werror -pedantic -std=c99
OUTDIR = build

all: all-build all-run

all-build:
	mkdir -p $(OUTDIR)
	$(CC) $(CFLAGS) anv_leaks.c -o $(OUTDIR)/anv_leaks.o
	$(CC) $(CFLAGS) anv_metalloc.c -o $(OUTDIR)/anv_metalloc.o

all-run:
	cd $(OUTDIR); for x in *.o; do ./$$x; done

.PHONY: clean
clean:
	rm -rdf $(OUTDIR)
