#include "common.h"
#include <riscv_vector.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <assert.h>

#ifdef ENABLE_PARSEC_HOOKS
#include <hooks.h>
#endif

// reference https://github.com/riscv/riscv-v-spec/blob/master/example/strlen.s
size_t strlen_vec(char *src, size_t* vl) {
  size_t vlmax = __riscv_vsetvlmax_e8m8();
  char *copy_src = src;
  long first_set_bit = -1;
  while (first_set_bit < 0) {
    vint8m8_t vec_src = __riscv_vle8ff_v_i8m8(copy_src, vl, vlmax);
    vbool1_t string_terminate = __riscv_vmseq_vx_i8m8_b1(vec_src, 0, *vl);

    copy_src += *vl;
    first_set_bit = __riscv_vfirst_m_b1(string_terminate, *vl);
  }
  copy_src -= *vl - first_set_bit;

  return (size_t)(copy_src - src);
}

int main(int argc, char** argv) {
  const uint32_t seed = 0xdeadbeef;
  srand(seed);

  // data gen
  size_t vlmax = __riscv_vsetvlmax_e8m8();
  long page_size = sysconf(_SC_PAGE_SIZE);
  unsigned fault_idx = (rand() % (vlmax-1)) + 1;
  unsigned n = page_size > vlmax ? 2 : 2*(vlmax/page_size) + 1;
  unsigned size = n * page_size;
  char* ptr = mmap(NULL, size, PROT_READ | PROT_WRITE,
                   MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  assert(ptr != MAP_FAILED);
  gen_string(ptr, size-page_size);
  char* s0 = ptr + size - page_size - vlmax - fault_idx;

  munmap(ptr+size-page_size, page_size);

  // compute
  size_t golden, actual, vl;
  golden = strlen(s0);
#ifdef ENABLE_PARSEC_HOOKS
  __parsec_roi_begin();
#endif
  actual = strlen_vec(s0, &vl);
  
  // compare
  printf("%s: %s\n", argv[0], (golden == actual) && (fault_idx == vl) ? "pass" : "fail");
#ifdef ENABLE_PARSEC_HOOKS
  __parsec_roi_end();
#endif
}
