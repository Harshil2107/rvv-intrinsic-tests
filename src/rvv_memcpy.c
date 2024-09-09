#include "common.h"
#include <riscv_vector.h>
#include <string.h>

#ifdef ENABLE_PARSEC_HOOKS
#include <hooks.h>
#endif

void *memcpy_vec(void *dst, void *src, size_t n) {
  void *save = dst;
  // copy data byte by byte
  for (size_t vl; n > 0; n -= vl, src += vl, dst += vl) {
    vl = __riscv_vsetvl_e8m8(n);
    vuint8m8_t vec_src = __riscv_vle8_v_u8m8(src, vl);
    __riscv_vse8_v_u8m8(dst, vec_src, vl);
  }
  return save;
}

int main(int argc, char** argv) {
  const int N = 127;
  const uint32_t seed = 0xdeadbeef;
  srand(seed);

  // data gen
  double A[N];
  gen_rand_1d(A, N);

  // compute
  double golden[N], actual[N];
  memcpy(golden, A, sizeof(A));
#ifdef ENABLE_PARSEC_HOOKS
  __parsec_roi_begin();
#endif
  memcpy_vec(actual, A, sizeof(A));

  // compare
  printf("%s: %s\n", argv[0], compare_1d(golden, actual, N) ? "pass" : "fail");
#ifdef ENABLE_PARSEC_HOOKS
  __parsec_roi_end();
#endif
}
