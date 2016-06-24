//
// auto-generated by op2.py
//

//user function
inline void adt_calc(const double *x1, const double *x2, const double *x3,
                     const double *x4, const double *q, double *adt) {
  double dx, dy, ri, u, v, c;

  ri = 1.0f / q[0];
  u = ri * q[1];
  v = ri * q[2];
  c = sqrt(gam * gm1 * (ri * q[3] - 0.5f * (u * u + v * v)));

  dx = x2[0] - x1[0];
  dy = x2[1] - x1[1];
  *adt = fabs(u * dy - v * dx) + c * sqrt(dx * dx + dy * dy);

  dx = x3[0] - x2[0];
  dy = x3[1] - x2[1];
  *adt += fabs(u * dy - v * dx) + c * sqrt(dx * dx + dy * dy);

  dx = x4[0] - x3[0];
  dy = x4[1] - x3[1];
  *adt += fabs(u * dy - v * dx) + c * sqrt(dx * dx + dy * dy);

  dx = x1[0] - x4[0];
  dy = x1[1] - x4[1];
  *adt += fabs(u * dy - v * dx) + c * sqrt(dx * dx + dy * dy);

  //*adt = (*adt) / cfl;
  *adt = (*adt) * (1.0f / cfl);
}
#ifdef VECTORIZE
//user function -- modified for vectorisation
void adt_calc_vec( const double x1[*][SIMD_VEC], const double x2[*][SIMD_VEC], const double x3[*][SIMD_VEC], const double x4[*][SIMD_VEC], const double *q, double *adt, int idx ) {
  double dx, dy, ri, u, v, c;

  ri = 1.0f / q[0];
  u = ri * q[1];
  v = ri * q[2];
  c = sqrt(gam * gm1 * (ri * q[3] - 0.5f * (u * u + v * v)));

  dx = x2[0][idx] - x1[0][idx];
  dy = x2[1][idx] - x1[1][idx];
  *adt = fabs(u * dy - v * dx) + c * sqrt(dx * dx + dy * dy);

  dx = x3[0][idx] - x2[0][idx];
  dy = x3[1][idx] - x2[1][idx];
  *adt += fabs(u * dy - v * dx) + c * sqrt(dx * dx + dy * dy);

  dx = x4[0][idx] - x3[0][idx];
  dy = x4[1][idx] - x3[1][idx];
  *adt += fabs(u * dy - v * dx) + c * sqrt(dx * dx + dy * dy);

  dx = x1[0][idx] - x4[0][idx];
  dy = x1[1][idx] - x4[1][idx];
  *adt += fabs(u * dy - v * dx) + c * sqrt(dx * dx + dy * dy);

  *adt = (*adt) * (1.0f / cfl);
}
#endif

// host stub function
void op_par_loop_adt_calc(char const *name, op_set set,
  op_arg arg0,
  op_arg arg1,
  op_arg arg2,
  op_arg arg3,
  op_arg arg4,
  op_arg arg5){

  int nargs = 6;
  op_arg args[6];

  args[0] = arg0;
  args[1] = arg1;
  args[2] = arg2;
  args[3] = arg3;
  args[4] = arg4;
  args[5] = arg5;
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
  ALIGNED_double       double * __restrict__ ptr5 = (double *) arg5.data;
  __assume_aligned(ptr5,double_ALIGN);

  // initialise timers
  double cpu_t1, cpu_t2, wall_t1, wall_t2;
  op_timing_realloc(1);
  op_timers_core(&cpu_t1, &wall_t1);

  if (OP_diags>2) {
    printf(" kernel routine with indirection: adt_calc\n");
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
      ALIGNED_double double dat2[2][SIMD_VEC];
      ALIGNED_double double dat3[2][SIMD_VEC];
      #pragma simd
      for ( int i=0; i<SIMD_VEC; i++ ){
        int idx0_2 = 2 * arg0.map_data[(n+i) * arg0.map->dim + 0];
        int idx1_2 = 2 * arg0.map_data[(n+i) * arg0.map->dim + 1];
        int idx2_2 = 2 * arg0.map_data[(n+i) * arg0.map->dim + 2];
        int idx3_2 = 2 * arg0.map_data[(n+i) * arg0.map->dim + 3];

        dat0[0][i] = (ptr0)[idx0_2 + 0];
        dat0[1][i] = (ptr0)[idx0_2 + 1];

        dat1[0][i] = (ptr1)[idx1_2 + 0];
        dat1[1][i] = (ptr1)[idx1_2 + 1];

        dat2[0][i] = (ptr2)[idx2_2 + 0];
        dat2[1][i] = (ptr2)[idx2_2 + 1];

        dat3[0][i] = (ptr3)[idx3_2 + 0];
        dat3[1][i] = (ptr3)[idx3_2 + 1];

      }
      #pragma simd
      for ( int i=0; i<SIMD_VEC; i++ ){
        adt_calc_vec(
          dat0,
          dat1,
          dat2,
          dat3,
          &(ptr4)[4 * (n+i)],
          &(ptr5)[1 * (n+i)],
          i);
      }
      for ( int i=0; i<SIMD_VEC; i++ ){

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
      int map2idx = arg0.map_data[n * arg0.map->dim + 2];
      int map3idx = arg0.map_data[n * arg0.map->dim + 3];

      adt_calc(
        &(ptr0)[2 * map0idx],
        &(ptr1)[2 * map1idx],
        &(ptr2)[2 * map2idx],
        &(ptr3)[2 * map3idx],
        &(ptr4)[4 * n],
        &(ptr5)[1 * n]);
    }
  }

  if (exec_size == 0 || exec_size == set->core_size) {
    op_mpi_wait_all(nargs, args);
  }
  // combine reduction data
  op_mpi_set_dirtybit(nargs, args);

  // update kernel record
  op_timers_core(&cpu_t2, &wall_t2);
  OP_kernels[1].name      = name;
  OP_kernels[1].count    += 1;
  OP_kernels[1].time     += wall_t2 - wall_t1;
  OP_kernels[1].transfer += (float)set->size * arg0.size;
  OP_kernels[1].transfer += (float)set->size * arg4.size;
  OP_kernels[1].transfer += (float)set->size * arg5.size;
  OP_kernels[1].transfer += (float)set->size * arg0.map->dim * 4.0f;
}
