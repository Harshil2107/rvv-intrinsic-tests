#include "common.h"
#include <riscv_vector.h>
#include <string.h>

#ifdef ENABLE_PARSEC_HOOKS
#include <hooks.h>
#endif

// reference https://github.com/riscv/riscv-v-spec/blob/master/example/strlen.s
size_t strlen_vec(char *src) {
  size_t vlmax = __riscv_vsetvlmax_e8m8();
  char *copy_src = src;
  long first_set_bit = -1;
  size_t vl;
  while (first_set_bit < 0) {
    vint8m8_t vec_src = __riscv_vle8ff_v_i8m8(copy_src, &vl, vlmax);
    vbool1_t string_terminate = __riscv_vmseq_vx_i8m8_b1(vec_src, 0, vl);

    copy_src += vl;

    first_set_bit = __riscv_vfirst_m_b1(string_terminate, vl);
  }
  copy_src -= vl - first_set_bit;

  return (size_t)(copy_src - src);
}

int main(int argc, char** argv) {
  const uint32_t seed = 0xdeadbeef;
  srand(seed);

  int N = rand() % 2000;

  // data gen
  char s0[N];
  gen_string(s0, N);

  // compute
  size_t golden, actual;
  golden = strlen(s0);
#ifdef ENABLE_PARSEC_HOOKS
  __parsec_roi_begin();
#endif
  actual = strlen_vec(s0);

  // compare
  printf("%s: %s\n", argv[0], golden == actual ? "pass" : "fail");
#ifdef ENABLE_PARSEC_HOOKS
  __parsec_roi_end();
#endif
}
