void main() {
  uint64 fpuState = __fpu_state;
  assert(fpuState == 0);
  assert(fpuState == __fpu_state);

  uint64 test = 42;
  __set_fpu_state(test);
  assert(__fpu_state == 42);

  __set_fpu_state(43);
  __set_fpu_state(44);
  __set_fpu_state(45);

  uint64 fpuState2 = __fpu_state;
  assert(fpuState2 == 45);
  assert(fpuState2 == __fpu_state);

  __set_fpu_state(fpuState2 + 1);
  assert(__fpu_state == 46);
  __set_fpu_state(__fpu_state + 1);
  assert(__fpu_state == 47);
  __set_fpu_state(1 + __fpu_state);
  assert(__fpu_state == 48);
  __set_fpu_state(__fpu_state);
  assert(__fpu_state == 48);
}
