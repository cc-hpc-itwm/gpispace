SRCS=$(wildcard *.msc)
EPSS=$(SRCS:.msc=.eps)
SVGS=$(SRCS:.msc=.svg)
PNGS=$(SRCS:.msc=.png)
PDFS=$(SRCS:.msc=.pdf)

OBJS=$(EPSS) $(PNGS) $(SVGS) $(PDFS)

MSCGEN="mscgen"
EPSTOPDF="epstopdf"

.PHONY: default all clean

default: all

%.eps : %.msc
	-@echo "\t[EPS] $< -> $@"
	@$(MSCGEN) -T eps -i $< -o $@

%.svg : %.msc
	-@echo "\t[SVG] $< -> $@"
	@$(MSCGEN) -T svg -i $< -o $@

%.png : %.msc
	-@echo "\t[PNG] $< -> $@"
	@$(MSCGEN) -T png -i $< -o $@

%.pdf : %.eps
	-@echo "\t[PDF] $< -> $@"
	@$(EPSTOPDF) $<


all: $(OBJS)

clean:
	-@rm -f $(OBJS)

