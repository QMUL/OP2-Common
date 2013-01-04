//
// auto-generated by op2.m on 29-Oct-2012 09:32:02
//

/*
Open source copyright declaration based on BSD open source template:
http://www.opensource.org/licenses/bsd-license.php

* Copyright (c) 2009-2011, Mike Giles
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*     * The name of Mike Giles may not be used to endorse or promote products
*       derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY Mike Giles ''AS IS'' AND ANY
* EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL Mike Giles BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

//
// standard headers
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

// global constants

float gm1, gm1i, wtg1[2], xi1[2], Ng1[4], Ng1_xi[4], wtg2[4], Ng2[16], Ng2_xi[32], minf, m2, freq, kappa, nmode, mfan;

//
// mpi header file - included by user for user level mpi
//

#include <mpi.h>

//
// OP header file
//

#include "op_lib_cpp.h"
int op2_stride = 1;
#define OP2_STRIDE(arr, idx) arr[op2_stride*(idx)]

//
// op_par_loop declarations
//

void op_par_loop_res_calc(char const *, op_set,
  op_arg,
  op_arg,
  op_arg,
  op_arg );

void op_par_loop_dirichlet(char const *, op_set,
  op_arg );

void op_par_loop_init_cg(char const *, op_set,
  op_arg,
  op_arg,
  op_arg,
  op_arg,
  op_arg );

void op_par_loop_spMV(char const *, op_set,
  op_arg,
  op_arg,
  op_arg );

void op_par_loop_dotPV(char const *, op_set,
  op_arg,
  op_arg,
  op_arg );

void op_par_loop_updateUR(char const *, op_set,
  op_arg,
  op_arg,
  op_arg,
  op_arg,
  op_arg );

void op_par_loop_dotR(char const *, op_set,
  op_arg,
  op_arg );

void op_par_loop_updateP(char const *, op_set,
  op_arg,
  op_arg,
  op_arg );

void op_par_loop_update(char const *, op_set,
  op_arg,
  op_arg,
  op_arg,
  op_arg );

#include "op_lib_mpi.h"

//
// kernel routines for parallel loops
//

#include "dirichlet.h"
#include "dotPV.h"
#include "dotR.h"
#include "res_calc.h"
#include "updateP.h"
#include "updateUR.h"
#include "init_cg.h"
#include "spMV.h"
#include "update.h"

//
//user declared functions
//

static int compute_local_size (int global_size, int mpi_comm_size, int mpi_rank )
{
  int local_size = global_size/mpi_comm_size;
  int remainder = (int)fmod(global_size,mpi_comm_size);

  if (mpi_rank < remainder)
  {
    local_size = local_size + 1;
  }
  return local_size;
}

static void scatter_float_array(float* g_array, float* l_array, int comm_size, int g_size,
  int l_size, int elem_size)
{
  int* sendcnts = (int *) malloc(comm_size*sizeof(int));
  int* displs = (int *) malloc(comm_size*sizeof(int));
  int disp = 0;

  for(int i = 0; i<comm_size; i++)
  {
    sendcnts[i] =   elem_size*compute_local_size (g_size, comm_size, i);
  }
  for(int i = 0; i<comm_size; i++)
  {
    displs[i] =   disp;
    disp = disp + sendcnts[i];
  }

  MPI_Scatterv(g_array, sendcnts, displs, MPI_float, l_array,
      l_size*elem_size, MPI_float, MPI_ROOT,  MPI_COMM_WORLD );

  free(sendcnts);
  free(displs);
}

static void scatter_int_array(int* g_array, int* l_array, int comm_size, int g_size,
  int l_size, int elem_size)
{
  int* sendcnts = (int *) malloc(comm_size*sizeof(int));
  int* displs = (int *) malloc(comm_size*sizeof(int));
  int disp = 0;

  for(int i = 0; i<comm_size; i++)
  {
    sendcnts[i] =   elem_size*compute_local_size (g_size, comm_size, i);
  }
  for(int i = 0; i<comm_size; i++)
  {
    displs[i] =   disp;
    disp = disp + sendcnts[i];
  }

  MPI_Scatterv(g_array, sendcnts, displs, MPI_INT, l_array,
      l_size*elem_size, MPI_INT, MPI_ROOT,  MPI_COMM_WORLD );

  free(sendcnts);
  free(displs);
}

static void check_scan(int items_received, int items_expected)
{
  if(items_received != items_expected) {
    op_printf("error reading from new_grid.dat\n");
    exit(-1);
  }
}

// main program

int main(int argc, char **argv)
{
  // OP initialisation

  op_init(argc,argv,2);

  //MPI for user I/O
  int my_rank;
  int comm_size;
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &comm_size);

  //timer
  double cpu_t1, cpu_t2, wall_t1, wall_t2;

  int    *bnode, *cell, *g_bnode, *g_cell;
  float  *xm, *g_xm;;

  int    nnode,ncell,nbnodes,niter, g_nnode, g_ncell, g_nbnodes;
  float  rms = 1;

  // read in grid

  op_printf("reading in grid \n");

  FILE *fp;
  if ( (fp = fopen("FE_grid.dat","r")) == NULL) {
    op_printf("can't open file FE_grid.dat\n"); exit(-1);
  }

  if (fscanf(fp,"%d %d %d \n",&g_nnode, &g_ncell, &g_nbnodes) != 3) {
    op_printf("error reading from new_grid.dat\n"); exit(-1);
  }

  if (my_rank == MPI_ROOT) {
    g_cell   = (int *) malloc(4*g_ncell*sizeof(int));
    g_bnode   = (int *) malloc(g_nbnodes*sizeof(int));
    g_xm      = (float *) malloc(2*g_nnode*sizeof(float));

    for (int n=0; n<g_nnode; n++) {
      if (fscanf(fp,"%f %f \n",&g_xm[2*n], &g_xm[2*n+1]) != 2) {
        op_printf("error reading from new_grid.dat\n"); exit(-1);
      }
    }

    for (int n=0; n<g_ncell; n++) {
      if (fscanf(fp,"%d %d %d %d \n",&g_cell[4*n  ], &g_cell[4*n+1],
      &g_cell[4*n+2], &g_cell[4*n+3]) != 4) {
        op_printf("error reading from new_grid.dat\n"); exit(-1);
      }
    }

    for (int n=0; n<g_nbnodes; n++) {
      if (fscanf(fp,"%d \n",&g_bnode[n]) != 1) {
        op_printf("error reading from new_grid.dat\n"); exit(-1);
      }
    }
  }
  fclose(fp);

  nnode = compute_local_size (g_nnode, comm_size, my_rank);
  ncell = compute_local_size (g_ncell, comm_size, my_rank);
  nbnodes = compute_local_size (g_nbnodes, comm_size, my_rank);

  cell   = (int *) malloc(4*ncell*sizeof(int));
  bnode   = (int *) malloc(nbnodes*sizeof(int));
  xm      = (float *) malloc(2*nnode*sizeof(float));

  scatter_int_array(g_cell, cell, comm_size, g_ncell,ncell, 4);
  scatter_int_array(g_bnode, bnode, comm_size, g_nbnodes,nbnodes, 1);
  scatter_float_array(g_xm, xm, comm_size, g_nnode,nnode, 2);

  if(my_rank == MPI_ROOT) {
    free(g_cell);
    free(g_xm);
    free(g_bnode);
  }

  // set constants and initialise flow field and residual

  op_printf("initialising flow field \n");

  float gam = 1.4;
  gm1 = gam - 1.0;
  gm1i = 1.0/gm1;

  wtg1[0] = 0.5;
  wtg1[1] = 0.5;
  xi1[0] = 0.211324865405187;
  xi1[1] = 0.788675134594813;
  Ng1[0] = 0.788675134594813;
  Ng1[1] = 0.211324865405187;
  Ng1[2] = 0.211324865405187;
  Ng1[3] = 0.788675134594813;
  Ng1_xi[0] = -1;
  Ng1_xi[1] = -1;
  Ng1_xi[2] = 1;
  Ng1_xi[3] = 1;
  wtg2[0] = 0.25;
  wtg2[1] = 0.25;
  wtg2[2] = 0.25;
  wtg2[3] = 0.25;
  Ng2[0] = 0.622008467928146; Ng2[1] = 0.166666666666667; Ng2[2] = 0.166666666666667; Ng2[3] = 0.044658198738520;
  Ng2[4] = 0.166666666666667; Ng2[5] = 0.622008467928146; Ng2[6] = 0.044658198738520; Ng2[7] = 0.166666666666667;
  Ng2[8] = 0.166666666666667; Ng2[9] = 0.044658198738520; Ng2[10] = 0.622008467928146; Ng2[11] = 0.166666666666667;
  Ng2[12] = 0.044658198738520; Ng2[13] = 0.166666666666667; Ng2[14] = 0.166666666666667; Ng2[15] = 0.622008467928146;
  Ng2_xi[0] = -0.788675134594813;  Ng2_xi[1] = 0.788675134594813;  Ng2_xi[2] = -0.211324865405187;Ng2_xi[3] = 0.211324865405187;
  Ng2_xi[4] = -0.788675134594813;  Ng2_xi[5] = 0.788675134594813;  Ng2_xi[6] = -0.211324865405187; Ng2_xi[7] = 0.211324865405187;
  Ng2_xi[8] = -0.211324865405187;  Ng2_xi[9] = 0.211324865405187;  Ng2_xi[10] = -0.788675134594813; Ng2_xi[11] = 0.788675134594813;
  Ng2_xi[12] = -0.211324865405187;  Ng2_xi[13] = 0.211324865405187;  Ng2_xi[14] = -0.788675134594813; Ng2_xi[15] = 0.788675134594813;
  Ng2_xi[16] = -0.788675134594813;  Ng2_xi[17] = -0.211324865405187;  Ng2_xi[18] = 0.788675134594813; Ng2_xi[19] = 0.211324865405187;
  Ng2_xi[20] = -0.211324865405187;  Ng2_xi[21] = -0.788675134594813;  Ng2_xi[22] = 0.211324865405187; Ng2_xi[23] = 0.788675134594813;
  Ng2_xi[24] = -0.788675134594813;  Ng2_xi[25] = -0.211324865405187;  Ng2_xi[26] = 0.788675134594813; Ng2_xi[27] = 0.211324865405187;
  Ng2_xi[28] = -0.211324865405187;  Ng2_xi[29] = -0.788675134594813;  Ng2_xi[30] = 0.211324865405187; Ng2_xi[31] = 0.788675134594813;

  minf = 0.1;
  m2 = minf*minf;
  freq = 1;
  kappa = 1;
  nmode = 0;

  mfan = 1.0;

  float *phim = (float *)malloc(nnode*sizeof(float));
  memset(phim,0,nnode*sizeof(float));
  for (int i = 0;i<nnode;i++) {
    phim[i] = minf*xm[2*i];
  }

  float *K = (float *)malloc(4*4*ncell*sizeof(float));
  memset(K,0,4*4*ncell*sizeof(float));
  float *resm = (float *)malloc(nnode*sizeof(float));
  memset(resm,0,nnode*sizeof(float));

  float *V = (float *)malloc(nnode*sizeof(float));
  memset(V,0,nnode*sizeof(float));
  float *P = (float *)malloc(nnode*sizeof(float));
  memset(P,0,nnode*sizeof(float));
  float *U = (float *)malloc(nnode*sizeof(float));
  memset(U,0,nnode*sizeof(float));

  // declare sets, pointers, datasets and global constants

  op_set nodes  = op_decl_set(nnode,  "nodes");
  op_set bnodes = op_decl_set(nbnodes, "bedges");
  op_set cells  = op_decl_set(ncell,  "cells");

  op_map pbnodes  = op_decl_map(bnodes,nodes,1,bnode, "pbedge");
  op_map pcell   = op_decl_map(cells, nodes,4,cell,  "pcell");

  op_dat p_xm     = op_decl_dat(nodes ,2,"float",xm    ,"p_x");
  op_dat p_phim  = op_decl_dat(nodes, 1, "float", phim, "p_phim");
  op_dat p_resm  = op_decl_dat(nodes, 1, "float", resm, "p_resm");
  op_dat p_K  = op_decl_dat(cells, 16, "float:soa", K, "p_K");

  op_dat p_V = op_decl_dat(nodes, 1, "float", V, "p_V");
  op_dat p_P = op_decl_dat(nodes, 1, "float", P, "p_P");
  op_dat p_U = op_decl_dat(nodes, 1, "float", U, "p_U");

  op_decl_const2("gam",1,"float",&gam  );
  op_decl_const2("gm1",1,"float",&gm1  );
  op_decl_const2("gm1i",1,"float",&gm1i  );
  op_decl_const2("m2",1,"float",&m2  );
  op_decl_const2("wtg1",2,"float",wtg1  );
  op_decl_const2("xi1",2,"float",xi1  );
  op_decl_const2("Ng1",4,"float",Ng1  );
  op_decl_const2("Ng1_xi",4,"float",Ng1_xi  );
  op_decl_const2("wtg2",4,"float",wtg2  );
  op_decl_const2("Ng2",16,"float",Ng2  );
  op_decl_const2("Ng2_xi",32,"float",Ng2_xi  );
  op_decl_const2("minf",1,"float",&minf  );
  op_decl_const2("freq",1,"float",&freq  );
  op_decl_const2("kappa",1,"float",&kappa  );
  op_decl_const2("nmode",1,"float",&nmode  );
  op_decl_const2("mfan",1,"float",&mfan  );

  op_diagnostic_output();

  op_partition("PTSCOTCH", "KWAY", cells, pcell, NULL);

  // main time-marching loop

  niter = 20;
  //initialise timers for total execution wall time
  op_timers(&cpu_t1, &wall_t1);
  for(int iter=1; iter<=niter; iter++) {

   op_par_loop_res_calc("res_calc",cells,
              op_arg_dat(p_xm,-4,pcell,2,"float",OP_READ),
              op_arg_dat(p_phim,-4,pcell,1,"float",OP_READ),
              op_arg_dat(p_K,-1,OP_ID,16,"float:soa",OP_WRITE),
              op_arg_dat(p_resm,-4,pcell,1,"float",OP_INC));

    op_par_loop_dirichlet("dirichlet",bnodes,
               op_arg_dat(p_resm,0,pbnodes,1,"float",OP_WRITE));

    float c1 = 0;
    float c2 = 0;
    float c3 = 0;
    float alpha = 0;
    float beta = 0;

    //c1 = R'*R;
    op_par_loop_init_cg("init_cg",nodes,
               op_arg_dat(p_resm,-1,OP_ID,1,"float",OP_READ),
               op_arg_gbl(&c1,1,"float",OP_INC),
               op_arg_dat(p_U,-1,OP_ID,1,"float",OP_WRITE),
               op_arg_dat(p_V,-1,OP_ID,1,"float",OP_WRITE),
               op_arg_dat(p_P,-1,OP_ID,1,"float",OP_WRITE));

    //set up stopping conditions
    float res0 = sqrt(c1);
    float res = res0;
    int inner_iter = 0;
    int maxiter = 200;
    while (res > 0.1*res0 && inner_iter < maxiter) {
      //V = Stiffness*P
      op_par_loop_spMV("spMV",cells,
                 op_arg_dat(p_V,-4,pcell,1,"float",OP_INC),
                 op_arg_dat(p_K,-1,OP_ID,16,"float:soa",OP_READ),
                 op_arg_dat(p_P,-4,pcell,1,"float",OP_READ));

      op_par_loop_dirichlet("dirichlet",bnodes,
                 op_arg_dat(p_V,0,pbnodes,1,"float",OP_WRITE));

      c2 = 0;

      //c2 = P'*V;
      op_par_loop_dotPV("dotPV",nodes,
                 op_arg_dat(p_P,-1,OP_ID,1,"float",OP_READ),
                 op_arg_dat(p_V,-1,OP_ID,1,"float",OP_READ),
                 op_arg_gbl(&c2,1,"float",OP_INC));

      alpha = c1/c2;

      //U = U + alpha*P;
      //resm = resm-alpha*V;
      op_par_loop_updateUR("updateUR",nodes,
                 op_arg_dat(p_U,-1,OP_ID,1,"float",OP_INC),
                 op_arg_dat(p_resm,-1,OP_ID,1,"float",OP_INC),
                 op_arg_dat(p_P,-1,OP_ID,1,"float",OP_READ),
                 op_arg_dat(p_V,-1,OP_ID,1,"float",OP_RW),
                 op_arg_gbl(&alpha,1,"float",OP_READ));

      c3 = 0;

      //c3 = resm'*resm;
      op_par_loop_dotR("dotR",nodes,
                 op_arg_dat(p_resm,-1,OP_ID,1,"float",OP_READ),
                 op_arg_gbl(&c3,1,"float",OP_INC));
      beta = c3/c1;
      //P = beta*P+resm;
      op_par_loop_updateP("updateP",nodes,
                 op_arg_dat(p_resm,-1,OP_ID,1,"float",OP_READ),
                 op_arg_dat(p_P,-1,OP_ID,1,"float",OP_RW),
                 op_arg_gbl(&beta,1,"float",OP_READ));
      c1 = c3;
      res = sqrt(c1);
      inner_iter++;
    }
    rms = 0;
    //phim = phim - Stiffness\Load;
    op_par_loop_update("update",nodes,
               op_arg_dat(p_phim,-1,OP_ID,1,"float",OP_RW),
               op_arg_dat(p_resm,-1,OP_ID,1,"float",OP_WRITE),
               op_arg_dat(p_U,-1,OP_ID,1,"float",OP_READ),
               op_arg_gbl(&rms,1,"float",OP_INC));
    op_printf("rms = %10.5e iter: %d\n", sqrt(rms)/sqrt(g_nnode), inner_iter);
  }
  op_timers(&cpu_t2, &wall_t2);
  op_timing_output();
  op_printf("Max total runtime = %f\n",wall_t2-wall_t1);
  op_exit();

  /*free(cell);
  free(bnode);
  free(xm);
  free(phim);
  free(K);
  free(resm);
  free(V);
  free(P);
  free(U);*/
}