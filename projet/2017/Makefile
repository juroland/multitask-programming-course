PANDOC_ARGS=--filter pandoc-numbering -V geometry:margin=1in -V numbersections --latex-engine=xelatex
TARGETS=shell.pdf
all: $(TARGETS)

%.pdf: %.md
	pandoc $(PANDOC_ARGS) $< -o $@

clean:
	rm -f $(TARGETS)
