OBJ = rapport.pdf
LATEX = pdflatex

all: $(OBJ)

%.pdf: %.tex
	pdflatex $< $@
	pdflatex $< $@
	pdflatex $< $@


clean:
	rm -f *.aux *.log *.nav *.out *.snm *~ *.toc *.pdf *.lof .*.swp

