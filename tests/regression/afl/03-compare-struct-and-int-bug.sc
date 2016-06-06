struct weird { }

void main () {
    int x;
    weird w;
    w>x; // assert failure, should raise a type error
}
