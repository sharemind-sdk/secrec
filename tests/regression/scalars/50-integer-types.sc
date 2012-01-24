void main () {
    { // supported integer types
        int8  i8;  uint8  u8;
        int16 i16; uint16 u16;
        int32 i32; uint32 u32;
        int64 i64; uint64 u64;
        assert (true);
    }

    { // integer literals are typed based on context
        int8  i8   = 1;  uint8  u8 = 1;
        int16 i16  = 1; uint16 u16 = 1;
        int32 i32  = 1; uint32 u32 = 1;
        int64 i64  = 1; uint64 u64 = 1;
        assert (true);
    }

    { // context from expression
        int8  i8   = 1+1;  uint8  u8 = 1+1;
        int16 i16  = 1*1; uint16 u16 = 1+1;
        int32 i32  = 1-1; uint32 u32 = 1+1;
        int64 i64  = 1/1; uint64 u64 = 1+1;
        assert (true);
    }

    {
        int8  i8   = - 1;
        int16 i16  = - 1;
        int32 i32  = - 1;
        int64 i64  = - 1;
        assert (true);
    }

    return;
}
