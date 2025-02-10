all: ide_ic1.jed ide_ic2v3.jed


*.jed: *.pld
	./5vcomp $<

clean:
	rm -f *.abs *.sim 

.PHONY: clean all

