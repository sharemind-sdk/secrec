all: semantics.pdf

semantics.pdf: semantics.tex references.bib
	latex semantics
	bibtex semantics
	latex semantics
	pdflatex semantics

distclean: clean
	rm -f *.pdf

clean:
	rm -f *.aux *.log *.out *.tdo *.toc *.backup *.dvi *.bbl *.blg
