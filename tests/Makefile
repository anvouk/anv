CFLAGS = -Wno-pointer-to-int-cast -Wno-int-to-pointer-cast

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
