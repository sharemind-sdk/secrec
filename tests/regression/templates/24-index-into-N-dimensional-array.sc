template<type T>
T random_float(T data) {
    return 0.0;
}

template <dim N>
float32[[N]] randomize(float32[[N]] data) {
    uint s = size(data);
    for (uint i = 0; i < s; ++ i)
        data[i] = random_float(0::float32);

    return data;
}

void main() {
    float32 t1;
    randomize(t1);
}
