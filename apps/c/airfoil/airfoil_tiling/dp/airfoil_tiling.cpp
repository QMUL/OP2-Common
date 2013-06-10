/*
 * Open source copyright declaration based on BSD open source template:
 * http://www.opensource.org/licenses/bsd-license.php
 *
 * This file is part of the OP2 distribution.
 *
 * Copyright (c) 2011, Mike Giles and others. Please see the AUTHORS file in
 * the main source directory for a full list of copyright holders.
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
//     Nonlinear airfoil lift calculation
//
//     Written by Mike Giles, 2010-2011, based on FORTRAN code
//     by Devendra Ghate and Mike Giles, 2005
//

//
// standard headers
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

// global constants

double gam, gm1, cfl, eps, mach, alpha, qinf[4];

//
// OP header file
//

#include "op_seq.h"

//
// Tiling header file
//

#include "executor.h"
#include "inspector.h"
#include "invert.h"
#include "test.h"
#include "plotmesh.h"

//
// kernel routines for parallel loops
//

#include "save_soln.h"
#include "adt_calc.h"
#include "res_calc.h"
#include "bres_calc.h"
#include "update.h"

// main program

int main(int argc, char **argv)
{
  // OP initialisation
  op_init(argc,argv,2);

  int    *becell, *ecell,  *bound, *bedge, *edge, *cell;
  double  *x, *q, *qold, *adt, *res;

  int    nnode,ncell,nedge,nbedge,niter;
  double  rms;

  //timer
  double cpu_t1, cpu_t2, wall_t1, wall_t2;

  // read in grid

  op_printf("reading in grid \n");

  FILE *fp;
  if ( (fp = fopen("./new_grid.dat","r")) == NULL) {
    op_printf("can't open file new_grid.dat\n"); exit(-1);
  }

  if (fscanf(fp,"%d %d %d %d \n",&nnode, &ncell, &nedge, &nbedge) != 4) {
    op_printf("error reading from new_grid.dat\n"); exit(-1);
  }

  cell   = (int *) malloc(4*ncell*sizeof(int));
  edge   = (int *) malloc(2*nedge*sizeof(int));
  ecell  = (int *) malloc(2*nedge*sizeof(int));
  bedge  = (int *) malloc(2*nbedge*sizeof(int));
  becell = (int *) malloc(  nbedge*sizeof(int));
  bound  = (int *) malloc(  nbedge*sizeof(int));

  x      = (double *) malloc(2*nnode*sizeof(double));
  q      = (double *) malloc(4*ncell*sizeof(double));
  qold   = (double *) malloc(4*ncell*sizeof(double));
  res    = (double *) malloc(4*ncell*sizeof(double));
  adt    = (double *) malloc(  ncell*sizeof(double));

  for (int n=0; n<nnode; n++) {
    if (fscanf(fp,"%lf %lf \n",&x[2*n], &x[2*n+1]) != 2) {
      op_printf("error reading from new_grid.dat\n"); exit(-1);
    }
  }

  for (int n=0; n<ncell; n++) {
    if (fscanf(fp,"%d %d %d %d \n",&cell[4*n  ], &cell[4*n+1],
               &cell[4*n+2], &cell[4*n+3]) != 4) {
      op_printf("error reading from new_grid.dat\n"); exit(-1);
    }
  }

  for (int n=0; n<nedge; n++) {
    if (fscanf(fp,"%d %d %d %d \n",&edge[2*n], &edge[2*n+1],
               &ecell[2*n],&ecell[2*n+1]) != 4) {
      op_printf("error reading from new_grid.dat\n"); exit(-1);
    }
  }

  for (int n=0; n<nbedge; n++) {
    if (fscanf(fp,"%d %d %d %d \n",&bedge[2*n],&bedge[2*n+1],
               &becell[n], &bound[n]) != 4) {
      op_printf("error reading from new_grid.dat\n"); exit(-1);
    }
  }

  fclose(fp);

  // set constants and initialise flow field and residual

  op_printf("initialising flow field \n");

  gam = 1.4f;
  gm1 = gam - 1.0f;
  cfl = 0.9f;
  eps = 0.05f;

  double mach  = 0.4f;
  double alpha = 3.0f*atan(1.0f)/45.0f;
  double p     = 1.0f;
  double r     = 1.0f;
  double u     = sqrt(gam*p/r)*mach;
  double e     = p/(r*gm1) + 0.5f*u*u;

  qinf[0] = r;
  qinf[1] = r*u;
  qinf[2] = 0.0f;
  qinf[3] = r*e;

  for (int n=0; n<ncell; n++) {
    for (int m=0; m<4; m++) {
      q[4*n+m] = qinf[m];
      res[4*n+m] = 0.0f;
    }
  }

  // declare sets, pointers, datasets and global constants

  op_set nodes  = op_decl_set(nnode,  "nodes");
  op_set edges  = op_decl_set(nedge,  "edges");
  op_set bedges = op_decl_set(nbedge, "bedges");
  op_set cells  = op_decl_set(ncell,  "cells");

  op_map pedge   = op_decl_map(edges, nodes,2,edge,  "pedge");
  //op_map pecell  = op_decl_map(edges, cells,2,ecell, "pecell");
  op_map pbedge  = op_decl_map(bedges,nodes,2,bedge, "pbedge");
  //op_map pbecell = op_decl_map(bedges,cells,1,becell,"pbecell");
  op_map pcell   = op_decl_map(cells, nodes,4,cell,  "pcell");

  /* TODO: still lack real integration with OP2
  op_dat p_bound = op_decl_dat(bedges,1,"int"  ,bound,"p_bound");
  op_dat p_x     = op_decl_dat(nodes ,2,"double",x    ,"p_x");
  op_dat p_adt   = op_decl_dat(cells ,1,"double",adt  ,"p_adt");
  op_dat p_res   = op_decl_dat(cells ,4,"double",res  ,"p_res");
  */
  op_dat p_q     = op_decl_dat(cells ,4,"double",q    ,"p_q");
  op_dat p_qold  = op_decl_dat(cells ,4,"double",qold ,"p_qold");


  op_decl_const(1,"double",&gam  );
  op_decl_const(1,"double",&gm1  );
  op_decl_const(1,"double",&cfl  );
  op_decl_const(1,"double",&eps  );
  op_decl_const(1,"double",&mach );
  op_decl_const(1,"double",&alpha);
  op_decl_const(4,"double",qinf  );

  op_diagnostic_output();


  // initialising and running the inspector

  int nvertices = 1000; // TODO

  op_printf ("running inspector\n");

  int* all_edge  = (int *) malloc (2*(nbedge+nedge)*sizeof(int));
  memcpy (all_edge, edge, sizeof(int)*2*nedge);
  memcpy (all_edge + 2*nedge, bedge, sizeof(int)*2*nbedge);

  inspector_t* insp = initInspector (nnode, nvertices, 4);
  //partitionAndColor (insp, nnode, pedge->map, nedge*2); // TODO: breaking abstraction
  partitionAndColor (insp, nnode, all_edge, (nedge+nbedge)*2); // TODO: breaking

  addParLoop (insp, "cells1", ncell, pcell->map, ncell * 4, OP_DIRECT);
  addParLoop (insp, "edges1", nedge, pedge->map, nedge * 2, OP_INDIRECT);
  addParLoop (insp, "bedges1", nbedge, pbedge->map, nbedge * 2, OP_INDIRECT);
  addParLoop (insp, "cells2", ncell, pcell->map, ncell * 4, OP_DIRECT);

  op_printf ("added parallel loops\n");

  if (runInspector (insp, 0) == INSPOP_WRONGCOLOR)
    op_printf ("%s\n", insp->debug);
  else
    op_printf ("coloring went fine\n");

  // print the mesh

  vtu_mesh_t* mesh = createVtuMesh (nnode, nedge, ncell, x, pedge->map, pcell->map, D2);
  printVtuFile (insp, mesh);
  freeVtuMesh (mesh);

  //inspectorDiagnostic (insp);

  // build the new data array with values in proper positions
  int x_size = 2*nnode;
  double* new_x = (double *) malloc(x_size*sizeof(double));
  for (int i = 0; i < nnode; i++)
  {
    new_x[2*insp->v2v[i]] = x[2*i];
    new_x[2*insp->v2v[i] + 1] = x[2*i + 1];
  }

  printf("running executor\n");
  executor_t* exec = initExecutor (insp);
  int ncolors = exec->ncolors;

  //initialise timers for total execution wall time
  op_timers(&cpu_t1, &wall_t1);

  // tiled execution of the first two loops
  int* renum_pcell  = insp->loops[0]->indMap;
  int* renum_pedge  = insp->loops[1]->indMap;
  int* renum_pbedge = insp->loops[2]->indMap;
  //int* renum2_pcell  = insp->loops[3]->indMap;

  // main time-marching loop

  niter = 1000;

  for(int iter=1; iter<=niter; iter++) {

    // save old flow solution

    op_par_loop(save_soln,"save_soln", cells,
                op_arg_dat(p_q,   -1,OP_ID, 4,"double",OP_READ ),
                op_arg_dat(p_qold,-1,OP_ID, 4,"double",OP_WRITE));

    // predictor/corrector update loop

    for(int k=0; k<2; k++) {

      rms = 0.0;

      //for each colour
      for (int i = 0; i < ncolors; i++)
      {
        // for all tiles of this color
        int tile_size;
        int first_tile = exec->offset[i];
        int last_tile = exec->offset[i + 1];

        for (int j = first_tile; j < last_tile; j++)
        {
          // execute the tile
          tile_t* tile = exec->tiles[exec->c2p[j]];

          // loop adt_calc (calculate area/timstep)
          tile_size = tile->curSize[0];
          for (int k = 0; k < tile_size; k++)
          {
            int cell = tile->element[0][k];

            adt_calc (new_x + renum_pcell[cell*4 + 0]*2,
                      new_x + renum_pcell[cell*4 + 1]*2,
                      new_x + renum_pcell[cell*4 + 2]*2,
                      new_x + renum_pcell[cell*4 + 3]*2,
                      q     + cell*4,
                      adt   + cell);
          }

          // loop res_calc
          tile_size = tile->curSize[1];
          for (int k = 0; k < tile_size; k++)
          {
            int edge = tile->element[1][k];

            res_calc (new_x + renum_pedge[edge*2 + 0]*2,
                      new_x + renum_pedge[edge*2 + 1]*2,
                      q     + ecell[edge*2 + 0]*4,
                      q     + ecell[edge*2 + 1]*4,
                      adt   + ecell[edge*2 + 0]*1,
                      adt   + ecell[edge*2 + 1]*1,
                      res   + ecell[edge*2 + 0]*4,
                      res   + ecell[edge*2 + 1]*4);
          }

          // loop bres_calc
          tile_size = tile->curSize[2];
          for (int k = 0; k < tile_size; k++)
          {
            int edge = tile->element[2][k];

            bres_calc (new_x + renum_pbedge[edge*2 + 0]*2,
                       new_x + renum_pbedge[edge*2 + 1]*2,
                       q     + becell[edge + 0]*4,
                       adt   + becell[edge + 0]*1,
                       res   + becell[edge + 0]*4,
                       bound + edge);
          }

          // loop update
          tile_size = tile->curSize[3];
          for (int k = 0; k < tile_size; k++)
          {
            int cell = tile->element[3][k];

            update    (qold  + cell*4,
                       q     + cell*4,
                       res   + cell*4,
                       adt   + cell,
                       &rms);
          }

        }
      }
    }

    // print iteration history
    rms = sqrt(rms/(double) op_get_size(cells));
    //if (iter%100 == 0)
    if (iter%10 == 0)
      op_printf(" %d  %10.5e \n",iter,rms);
  }

  op_timers(&cpu_t2, &wall_t2);

  //output the result dat array to files
  op_print_dat_to_txtfile(p_q, "out_grid_tile_seq.dat"); //ASCI
  op_print_dat_to_binfile(p_q, "out_grid_tile_seq.bin"); //Binary

  op_timing_output();
  op_printf("Max total runtime = \n%f\n",wall_t2-wall_t1);

  op_exit();


  freeInspector (insp);
  op_printf ("inspector destroyed\n");
  freeExecutor (exec);
  op_printf ("executor destroyed\n");

  free(all_edge);
  free(cell);
  free(edge);
  free(ecell);
  free(bedge);
  free(becell);
  free(bound);
  free(x);
  free(new_x);
  free(q);
  free(qold);
  free(res);
  free(adt);
}