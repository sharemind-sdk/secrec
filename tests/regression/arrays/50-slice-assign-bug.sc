void main() {
    uint8 [[1]] src(1);
    uint8 [[1]] dest(1);
    uint m = 1;
    dest[: 0 + m] = src[:];
}
