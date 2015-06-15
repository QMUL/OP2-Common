//
// auto-generated by op2.py
//

//user function
inline void res_calc(const double *x1, const double *x2, const double *q1, const double *q2,
                     const double *adt1, const double *adt2, double *res1, double *res2) {
  double dx,dy,mu, ri, p1,vol1, p2,vol2, f;

  dx = x1[0] - x2[0];
  dy = x1[1] - x2[1];

  ri   = 1.0f/q1[0];
  p1   = gm1*(q1[3]-0.5f*ri*(q1[1]*q1[1]+q1[2]*q1[2]));
  vol1 =  ri*(q1[1]*dy - q1[2]*dx);

  ri   = 1.0f/q2[0];
  p2   = gm1*(q2[3]-0.5f*ri*(q2[1]*q2[1]+q2[2]*q2[2]));
  vol2 =  ri*(q2[1]*dy - q2[2]*dx);

  mu = 0.5f*((*adt1)+(*adt2))*eps;

  f = 0.5f*(vol1* q1[0]         + vol2* q2[0]        ) + mu*(q1[0]-q2[0]);
  res1[0] += f;
  res2[0] -= f;
  f = 0.5f*(vol1* q1[1] + p1*dy + vol2* q2[1] + p2*dy) + mu*(q1[1]-q2[1]);
  res1[1] += f;
  res2[1] -= f;
  f = 0.5f*(vol1* q1[2] - p1*dx + vol2* q2[2] - p2*dx) + mu*(q1[2]-q2[2]);
  res1[2] += f;
  res2[2] -= f;
  f = 0.5f*(vol1*(q1[3]+p1)     + vol2*(q2[3]+p2)    ) + mu*(q1[3]-q2[3]);
  res1[3] += f;
  res2[3] -= f;
}

#ifdef VECTORIZE
//user function -- modified for vectorisation
void res_calc_vec( const double x1[*][SIMD_VEC], const double x2[*][SIMD_VEC], const double q1[*][SIMD_VEC], const double q2[*][SIMD_VEC], const double adt1[*][SIMD_VEC], const double adt2[*][SIMD_VEC], double res1[*][SIMD_VEC], double res2[*][SIMD_VEC], int idx ) {
  double dx,dy,mu, ri, p1,vol1, p2,vol2, f;

  dx = x1[0][idx] - x2[0][idx];
  dy = x1[1][idx] - x2[1][idx];

  ri   = 1.0f/q1[0][idx];
  p1   = gm1*(q1[3][idx]-0.5f*ri*(q1[1][idx]*q1[1][idx]+q1[2][idx]*q1[2][idx]));
  vol1 =  ri*(q1[1][idx]*dy - q1[2][idx]*dx);

  ri   = 1.0f/q2[0][idx];
  p2   = gm1*(q2[3][idx]-0.5f*ri*(q2[1][idx]*q2[1][idx]+q2[2][idx]*q2[2][idx]));
  vol2 =  ri*(q2[1][idx]*dy - q2[2][idx]*dx);

  mu = 0.5f*((adt1[0][idx])+(adt2[0][idx]))*eps;

  f = 0.5f*(vol1* q1[0][idx]         + vol2* q2[0][idx]        ) + mu*(q1[0][idx]-q2[0][idx]);
  res1[0][idx] += f;
  res2[0][idx] -= f;
  f = 0.5f*(vol1* q1[1][idx] + p1*dy + vol2* q2[1][idx] + p2*dy) + mu*(q1[1][idx]-q2[1][idx]);
  res1[1][idx] += f;
  res2[1][idx] -= f;
  f = 0.5f*(vol1* q1[2][idx] - p1*dx + vol2* q2[2][idx] - p2*dx) + mu*(q1[2][idx]-q2[2][idx]);
  res1[2][idx] += f;
  res2[2][idx] -= f;
  f = 0.5f*(vol1*(q1[3][idx]+p1)     + vol2*(q2[3][idx]+p2)    ) + mu*(q1[3][idx]-q2[3][idx]);
  res1[3][idx] += f;
  res2[3][idx] -= f;
}
#endif

#include <immintrin.h>
#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <ctime>

// For double precision on AVX, VLEN = 4
#define VLEN 4

// Gather two adjacent 64-bit quantities and transpose them into SoA form.
inline void gather_transpose_2x64(const double* restrict data,
                                  const int idx[VLEN], double v[2][VLEN]){
    // One 128-bit load for each of our 4 vector lanes.
    // Note the interleaving -- because of the shuffle pattern I use, I store [0, 2] [1, 3] instead of [0, 1] [2, 3].
    __m256d in02 = _mm256_castpd128_pd256(_mm_load_pd(&data[idx[0]]));
    __m256d in13 = _mm256_castpd128_pd256(_mm_load_pd(&data[idx[1]]));
    in02 = _mm256_insertf128_pd(in02, _mm_load_pd(&data[idx[2]]), 1);
    in13 = _mm256_insertf128_pd(in13, _mm_load_pd(&data[idx[3]]), 1);

    // Shuffle sequence to transpose.  This will be the same for other instruction sets.
    // Note that the shuffle is identical for each 128-bits.
    __m256d out0 = _mm256_shuffle_pd(in02, in13, 0b0000);
    __m256d out1 = _mm256_shuffle_pd(in02, in13, 0b1111);

    // One vector-length store per struct element.
    _mm256_store_pd(v[0], out0);
    _mm256_store_pd(v[1], out1);
}

// host stub function
void op_par_loop_res_calc(char const *name, op_set set,
  op_arg arg0,
  op_arg arg1,
  op_arg arg2,
  op_arg arg3,
  op_arg arg4,
  op_arg arg5,
  op_arg arg6,
  op_arg arg7){

  int nargs = 8;
  op_arg args[8];

  args[0] = arg0;
  args[1] = arg1;
  args[2] = arg2;
  args[3] = arg3;
  args[4] = arg4;
  args[5] = arg5;
  args[6] = arg6;
  args[7] = arg7;
  //create aligned pointers for dats
  ALIGNED_double const double * __restrict__ ptr0 = (double *) arg0.data;
  __assume_aligned(ptr0,double_ALIGN);
  ALIGNED_double const double * __restrict__ ptr1 = (double *) arg1.data;
  __assume_aligned(ptr1,double_ALIGN);
  ALIGNED_double const double * __restrict__ ptr2 = (double *) arg2.data;
  __assume_aligned(ptr2,double_ALIGN);
  ALIGNED_double const double * __restrict__ ptr3 = (double *) arg3.data;
  __assume_aligned(ptr3,double_ALIGN);
  ALIGNED_double const double * __restrict__ ptr4 = (double *) arg4.data;
  __assume_aligned(ptr4,double_ALIGN);
  ALIGNED_double const double * __restrict__ ptr5 = (double *) arg5.data;
  __assume_aligned(ptr5,double_ALIGN);
  ALIGNED_double       double * __restrict__ ptr6 = (double *) arg6.data;
  __assume_aligned(ptr6,double_ALIGN);
  ALIGNED_double       double * __restrict__ ptr7 = (double *) arg7.data;
  __assume_aligned(ptr7,double_ALIGN);

  // initialise timers
  double cpu_t1, cpu_t2, wall_t1, wall_t2;
  op_timing_realloc(2);
  op_timers_core(&cpu_t1, &wall_t1);

  if (OP_diags>2) {
    printf(" kernel routine with indirection: res_calc\n");
  }

  int exec_size = op_mpi_halo_exchanges(set, nargs, args);

  if (exec_size >0) {

    #ifdef VECTORIZE
    #pragma novector
    for ( int n=0; n<(exec_size/SIMD_VEC)*SIMD_VEC; n+=SIMD_VEC ){
      if (n+SIMD_VEC >= set->core_size) {
        op_mpi_wait_all(nargs, args);
      }

      ALIGNED_double double dat0[2][SIMD_VEC];
      ALIGNED_double double dat1[2][SIMD_VEC];
      ALIGNED_double double dat2[4][SIMD_VEC];
      ALIGNED_double double dat3[4][SIMD_VEC];
      ALIGNED_double double dat4[1][SIMD_VEC];
      ALIGNED_double double dat5[1][SIMD_VEC];
      ALIGNED_double double dat6[4][SIMD_VEC];
      ALIGNED_double double dat7[4][SIMD_VEC];

      // Temporary variables
      __declspec(align(64)) int idx0[SIMD_VEC];
      __declspec(align(64)) int idx1[SIMD_VEC];
      //__declspec(align(64)) double dat0[2][SIMD_VEC];

      /*#pragma simd
      for ( int i=0; i<SIMD_VEC; i++ ){
        idx0[i] = 2 * arg0.map_data[(n+i) * arg0.map->dim + 0];
        idx1[i] = 2 * arg0.map_data[(n+i) * arg0.map->dim + 1];
      }
      gather_transpose_2x64(ptr0, idx0, dat0);
      gather_transpose_2x64(ptr1, idx1, dat1);*/

      #pragma simd
      for ( int i=0; i<SIMD_VEC; i++ ){
        int idx0_2 = 2 * arg0.map_data[(n+i) * arg0.map->dim + 0];
        int idx1_2 = 2 * arg0.map_data[(n+i) * arg0.map->dim + 1];
        int idx2_4 = 4 * arg2.map_data[(n+i) * arg2.map->dim + 0];
        int idx3_4 = 4 * arg2.map_data[(n+i) * arg2.map->dim + 1];
        int idx4_1 = 1 * arg2.map_data[(n+i) * arg2.map->dim + 0];
        int idx5_1 = 1 * arg2.map_data[(n+i) * arg2.map->dim + 1];


        dat0[0][i] = (ptr0)[idx0_2 + 0];
        dat0[1][i] = (ptr0)[idx0_2 + 1];

        dat1[0][i] = (ptr1)[idx1_2 + 0];
        dat1[1][i] = (ptr1)[idx1_2 + 1];

        dat2[0][i] = (ptr2)[idx2_4 + 0];
        dat2[1][i] = (ptr2)[idx2_4 + 1];
        dat2[2][i] = (ptr2)[idx2_4 + 2];
        dat2[3][i] = (ptr2)[idx2_4 + 3];

        dat3[0][i] = (ptr3)[idx3_4 + 0];
        dat3[1][i] = (ptr3)[idx3_4 + 1];
        dat3[2][i] = (ptr3)[idx3_4 + 2];
        dat3[3][i] = (ptr3)[idx3_4 + 3];

        dat4[0][i] = (ptr4)[idx4_1 + 0];

        dat5[0][i] = (ptr5)[idx5_1 + 0];

        dat6[0][i] = 0.0;
        dat6[1][i] = 0.0;
        dat6[2][i] = 0.0;
        dat6[3][i] = 0.0;

        dat7[0][i] = 0.0;
        dat7[1][i] = 0.0;
        dat7[2][i] = 0.0;
        dat7[3][i] = 0.0;

      }
      #pragma simd
      for ( int i=0; i<SIMD_VEC; i++ ){
        res_calc_vec(
          dat0,
          dat1,
          dat2,
          dat3,
          dat4,
          dat5,
          dat6,
          dat7,
          i);
      }
      for ( int i=0; i<SIMD_VEC; i++ ){
        int idx6_4 = 4 * arg2.map_data[(n+i) * arg2.map->dim + 0];
        int idx7_4 = 4 * arg2.map_data[(n+i) * arg2.map->dim + 1];

        (ptr6)[idx6_4 + 0] += dat6[0][i];
        (ptr6)[idx6_4 + 1] += dat6[1][i];
        (ptr6)[idx6_4 + 2] += dat6[2][i];
        (ptr6)[idx6_4 + 3] += dat6[3][i];

        (ptr7)[idx7_4 + 0] += dat7[0][i];
        (ptr7)[idx7_4 + 1] += dat7[1][i];
        (ptr7)[idx7_4 + 2] += dat7[2][i];
        (ptr7)[idx7_4 + 3] += dat7[3][i];

      }
    }

    //remainder
    for ( int n=(exec_size/SIMD_VEC)*SIMD_VEC; n<exec_size; n++ ){
    #else
    for ( int n=0; n<exec_size; n++ ){
    #endif
      if (n==set->core_size) {
        op_mpi_wait_all(nargs, args);
      }
      int map0idx = arg0.map_data[n * arg0.map->dim + 0];
      int map1idx = arg0.map_data[n * arg0.map->dim + 1];
      int map2idx = arg2.map_data[n * arg2.map->dim + 0];
      int map3idx = arg2.map_data[n * arg2.map->dim + 1];

      res_calc(
        &(ptr0)[2 * map0idx],
        &(ptr1)[2 * map1idx],
        &(ptr2)[4 * map2idx],
        &(ptr3)[4 * map3idx],
        &(ptr4)[1 * map2idx],
        &(ptr5)[1 * map3idx],
        &(ptr6)[4 * map2idx],
        &(ptr7)[4 * map3idx]);
    }
  }

  if (exec_size == 0 || exec_size == set->core_size) {
    op_mpi_wait_all(nargs, args);
  }
  // combine reduction data
  op_mpi_set_dirtybit(nargs, args);

  // update kernel record
  op_timers_core(&cpu_t2, &wall_t2);
  OP_kernels[2].name      = name;
  OP_kernels[2].count    += 1;
  OP_kernels[2].time     += wall_t2 - wall_t1;
}
