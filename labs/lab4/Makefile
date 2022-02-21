PANDOC_ARGS=--filter pandoc-numbering -V geometry:margin=1.25in -V numbersections --latex-engine=xelatex
TARGETS=interprocess_communication.pdf
all: $(TARGETS)

%.pdf: %.md
	pandoc $(PANDOC_ARGS) $< -o $@

clean:
	rm -f $(TARGETS)
