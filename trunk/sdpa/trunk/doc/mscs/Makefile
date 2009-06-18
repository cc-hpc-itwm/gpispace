SRCS=$(wildcard *.msc)
SVGS=$(SRCS:.msc=.svg)
PNGS=$(SRCS:.msc=.png)

OBJS=$(PNGS) $(SVGS)

MSCGEN="mscgen"

.PHONY: default all clean

default: all

%.svg : %.msc
	-@echo "\t[SVG] $< -> $@"
	@$(MSCGEN) -T svg -i $< -o $@

%.png : %.msc
	-@echo "\t[PNG] $< -> $@"
	@$(MSCGEN) -T png -i $< -o $@

all: $(OBJS)

clean:
	-@rm -f $(OBJS)

