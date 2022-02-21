pandoc --filter pandoc-numbering --toc --template \
~/Repositories/pandoc-templates/default.latex -V geometry:margin=1.25in -V \
numbersections --latex-engine=xelatex intro.md c.md processes.md \
interprocess_communication.md shell.md threads.md producer_comsumer.md -o \
multitask.pdf \
