Compilation is rather crude right now.
For example to produce working bytecode for multiplication test:

./build/bin/scc -I./tests/sharemind ./tests/sharemind/04-multiplication/main.sc -o t.sa
./path/to/vm/build/bin/smas t.sa -o t.sb
