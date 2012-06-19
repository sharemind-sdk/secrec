void main () {
    int n = 10;
    uint [[1]] au64 (n);
    au64[0] = 0;
    au64[n - 1] = 0; // fails, but shouldn't
}
