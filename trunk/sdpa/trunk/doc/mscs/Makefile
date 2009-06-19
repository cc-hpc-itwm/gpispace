SRCS=$(wildcard *.msc)
EPSS=$(SRCS:.msc=.eps)
SVGS=$(SRCS:.msc=.svg)
PNGS=$(SRCS:.msc=.png)

OBJS=$(EPSS) $(PNGS) $(SVGS)

MSCGEN="mscgen"

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

all: $(OBJS)

clean:
	-@rm -f $(OBJS)

