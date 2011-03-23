DOC=semantics

all: $(DOC).pdf

$(DOC).pdf:
	latex $(DOC)
	latex $(DOC)
	pdflatex $(DOC)

distclean: clean
	rm -f *.pdf

clean:
	rm -f *.aux *.log *.out *.tdo *.toc *.backup *.dvi *.bbl *.blg
