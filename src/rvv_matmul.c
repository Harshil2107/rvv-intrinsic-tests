#include "common.h"
#include <riscv_vector.h>

#ifdef ENABLE_PARSEC_HOOKS
#include <hooks.h>
#endif

// matrix multiplication
// A[n][o], B[m][o] --> C[n][m];
void matmul_golden(double **a, double **b, double **c, int n, int m, int o) {
  for (int i = 0; i < n; ++i)
    for (int j = 0; j < m; ++j) {
      c[i][j] = 0;
      for (int k = 0; k < o; ++k)
        c[i][j] += a[i][k] * b[j][k];
    }
}

void matmul(double **a, double **b, double **c, int n, int m, int o) {
  size_t vlmax = __riscv_vsetvlmax_e64m1();
  for (int i = 0; i < n; ++i) {
    for (int j = 0; j < m; ++j) {
      double *ptr_a = &a[i][0];
      double *ptr_b = &b[j][0];
      int k = o;
      vfloat64m1_t vec_s = __riscv_vfmv_v_f_f64m1(0, vlmax);
      vfloat64m1_t vec_zero = __riscv_vfmv_v_f_f64m1(0, vlmax);
      for (size_t vl; k > 0; k -= vl, ptr_a += vl, ptr_b += vl) {
        vl = __riscv_vsetvl_e64m1(k);

        vfloat64m1_t vec_a = __riscv_vle64_v_f64m1(ptr_a, vl);
        vfloat64m1_t vec_b = __riscv_vle64_v_f64m1(ptr_b, vl);

        vbool64_t mask = __riscv_vmfne_vv_f64m1_b64(vec_a, vec_zero, vl);

        vec_s = __riscv_vfmacc_vv_f64m1_tumu(mask, vec_s, vec_a, vec_b, vl);
      }

      vfloat64m1_t vec_sum;
      vec_sum = __riscv_vfredusum_vs_f64m1_f64m1(vec_s, vec_zero, vlmax);
      double sum = __riscv_vfmv_f_s_f64m1_f64(vec_sum);
      c[i][j] = sum;
    }
  }
}

int main(int argc, char** argv) {
  const int N = 8;
  const int M = 8;
  const int O = 7;
  uint32_t seed = 0xdeadbeef;
  srand(seed);

  // data gen
  double **A = alloc_array_2d(N, O);
  double **B = alloc_array_2d(M, O);
  gen_rand_2d(A, N, O);
  gen_rand_2d(B, M, O);

  // compute
  double **golden = alloc_array_2d(N, M);
  double **actual = alloc_array_2d(N, M);
  matmul_golden(A, B, golden, N, M, O);
#ifdef ENABLE_PARSEC_HOOKS
  __parsec_roi_begin();
#endif
  matmul(A, B, actual, N, M, O);

  // compare
  printf("%s: %s\n", argv[0], compare_2d(golden, actual, N, M) ? "pass" : "fail");
#ifdef ENABLE_PARSEC_HOOKS
  __parsec_roi_end();
#endif
}
