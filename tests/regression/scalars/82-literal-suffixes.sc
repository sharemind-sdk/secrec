
int f(uint8 x) { return 0; }
int f(uint16 x) { return 1; }
int f(uint32 x) { return 2; }
int f(uint64 x) { return 3; }
int f(int8 x) { return 4; }
int f(int16 x) { return 5; }
int f(int32 x) { return 6; }
int f(int64 x) { return 7; }
int f(float32 x) { return 8; }
int f(float64 x) { return 9; }

void g(int[[1u8]] x) { }
void g(int[[1i16]] x) { }

void main()
{
    assert (f(255u8) == 0);
    assert (f(65535u16) == 1);
    assert (f(4294967295u32) == 2);
    assert (f(18446744073709551615u64) == 3);

    assert (f(127i8) == 4);
    assert (f(32767i16) == 5);
    assert (f(2147483647i32) == 6);
    assert (f(9223372036854775807i64) == 7);

    assert (f(0.0f32) == 8);
    assert (f(1e-5f32) == 8);
    assert (f(10e5f32) == 8);
    assert (f(0.0f64) == 9);
    assert (f(128e10f64) == 9);

    assert (f(0.0) == 9);
    assert (f(0) == 7);

    assert (0 == 0x0u8);
    assert (0x0u8 == 0);
    assert (0x0u16 == 0);
    assert (0x0u32 == 0);
    assert (0x0u64 == 0);
    assert (0x0i8 == 0);
    assert (0x0i16 == 0);
    assert (0x0i32 == 0);
    assert (0x0i64 == 0);
    assert (0x0f32 == 0);
    assert (0x0f64 == 0);
}
