#include "common.h"
#include <assert.h>
#include <riscv_vector.h>
#include <string.h>

#ifdef ENABLE_PARSEC_HOOKS
#include <hooks.h>
#endif

// reference https://github.com/riscv/riscv-v-spec/blob/master/example/strcpy.s
char *strcpy_vec(char *dst, const char *src) {
  char *save = dst;
  size_t vlmax = __riscv_vsetvlmax_e8m8();
  long first_set_bit = -1;
  size_t vl;
  while (first_set_bit < 0) {
    vint8m8_t vec_src = __riscv_vle8ff_v_i8m8(src, &vl, vlmax);

    vbool1_t string_terminate = __riscv_vmseq_vx_i8m8_b1(vec_src, 0, vl);
    vbool1_t mask = __riscv_vmsif_m_b1(string_terminate, vl);

    __riscv_vse8_v_i8m8_m(mask, dst, vec_src, vl);

    src += vl;
    dst += vl;

    first_set_bit = __riscv_vfirst_m_b1(string_terminate, vl);
  }
  return save;
}

int main(int argc, char** argv) {
  const int N = 2000;
  const uint32_t seed = 0xdeadbeef;
  srand(seed);

  // data gen
  char s0[N];
  gen_string(s0, N);

  // compute
  char golden[N], actual[N];
  strcpy(golden, s0);
#ifdef ENABLE_PARSEC_HOOKS
  __parsec_roi_begin();
#endif
  strcpy_vec(actual, s0);

  // compare
  printf("%s: %s\n", argv[0], strcmp(golden, actual) == 0 ? "pass" : "fail");
#ifdef ENABLE_PARSEC_HOOKS
  __parsec_roi_end();
#endif
}
