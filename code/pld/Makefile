all: ide_ic1.jed ide_ic2.jed


%.jed:	%.pld
	./5vcomp $<

clean:
	rm -f *.abs *.sim *.pdf

.PHONY: clean all

