SRC=$(wildcard *.md)
OUT=$(patsubst %.md,%.pdf,$(SRC))

all: $(OUT)

%.pdf: %.md
	pandoc --filter pandoc-numbering -V geometry:margin=1.25in -V numbersections --latex-engine=xelatex $< -o $@
