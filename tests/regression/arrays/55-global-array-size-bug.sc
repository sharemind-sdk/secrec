int[[1]] arr = {1,2,3};

int[[1]] emptyArr;

uint n = size(arr);

void main () {
    assert (size(arr) == 3);
    assert (size(emptyArr) == 0);
    assert (n == 3);

    arr = 0; // set values
    assert (size(arr) == 3);

    arr = emptyArr; // init with global
    assert (size(arr) == 0);

    emptyArr = {1, 2, 3, 4, 5}; // init with local
    assert (size(emptyArr) == 5);
}
