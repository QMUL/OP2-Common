
!Crown Copyright 2014 AWE.
!
! This file is part of Bookleaf.
!
! Bookleaf is free software: you can redistribute it and/or modify it under
! the terms of the GNU General Public License as published by the
! Free Software Foundation, either version 3 of the License, or (at your option)
! any later version.
!
! Bookleaf is distributed in the hope that it will be useful, but
! WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
! FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
! details.
!
! You should have received a copy of the GNU General Public License along with
! Bookleaf. If not, see http://www.gnu.org/licenses/.

MODULE lagstep_mod

  IMPLICIT NONE

  PUBLIC :: lagstep

CONTAINS

  SUBROUTINE lagstep(dt)

    USE kinds_mod,    ONLY: rlk,ink,lok
    USE integers_mod, ONLY: nel,nnod,nshape,nel1,nnod1
    USE geometry_mod, ONLY: getgeom,getgeom2
    USE pointers_mod, ONLY: rho,elmass,elvol,ielmat,ein,pre,csqrd,      &
&                           ndx,ndy,elx,ely,ndu,ndv,ielnod
    USE getacc_mod,   ONLY: getacc
    USE getq_mod,     ONLY: getq
    USE getpc_mod,    ONLY: getpc
    USE getforce_mod, ONLY: getforce
    USE getein_mod,   ONLY: getein
    USE utilities_mod,ONLY: gather,gather2
    USE scratch_mod,  ONLY: elu=>rscratch21,elv=>rscratch22,            &
&                           elfx=>rscratch23,elfy=>rscratch24,          &
&                           rho05=>rscratch11,ein05=>rscratch12,        &
&                           pre05=>rscratch13,ndxu=>rscratch14,         &
&                           ndyv=>rscratch15,dx=>rscratch25,            &
&                           dy=>rscratch26,scratch=>rscratch27
    USE timing_mod,   ONLY: bookleaf_times
    USE TYPH_util_mod,ONLY: get_time
    USE op2_bookleaf,       d_elu=>d_rscratch21,d_elv=>d_rscratch22,            &
&                           d_elfx=>d_rscratch23,d_elfy=>d_rscratch24,          &
&                           d_rho05=>d_rscratch11,d_ein05=>d_rscratch12,        &
&                           d_pre05=>d_rscratch13,d_ndxu=>d_rscratch14,         &
&                           d_ndyv=>d_rscratch15,d_dx=>d_rscratch25,            &
&                           d_dy=>d_rscratch26,d_scratch=>d_rscratch27

    USE common_kernels, ONLY: a_eq_b_over_c
    USE lagstep_kernels


    ! Argument list
    REAL(KIND=rlk),INTENT(IN) :: dt
    ! Local
    INTEGER(KIND=ink)         :: iel,inod
    REAL(KIND=rlk)            :: dt05, t0, t1

    ! Timer
    t0=get_time()

    ! ##############
    ! Predictor
    ! ##############
    dt05=0.5_rlk*dt

    CALL gather2(s_elements,m_el2node,d_ndu,d_elu)
    CALL gather2(s_elements,m_el2node,d_ndv,d_elv)

    ! Artificial viscosity
    CALL getq (d_elx,d_ely,d_elu,d_elv,d_rho,  &
&             d_pre,d_dx,d_dy,d_elfx,d_elfy,d_scratch)

    ! Force
    CALL getforce(nshape,nel,dt05,elx(1,1),ely(1,1),elu(1,1),elv(1,1),  &
&                 elfx(1,1),elfy(1,1),pre(1),rho(1),.FALSE._lok, &
                  d_elx,d_ely,d_elu,d_elv,d_elfx,d_elfy,d_pre,d_rho)

    ! Half step positions
    call op_par_loop_7(lagstep_half_pos,s_nodes, &
&                      op_arg_dat(d_ndxu,-1,OP_ID,1,'real(8)',OP_WRITE), &
&                      op_arg_dat(d_ndyv,-1,OP_ID,1,'real(8)',OP_WRITE), &
&                      op_arg_dat(d_ndx ,-1,OP_ID,1,'real(8)',OP_READ), &
&                      op_arg_dat(d_ndy ,-1,OP_ID,1,'real(8)',OP_READ), &
&                      op_arg_dat(d_ndu ,-1,OP_ID,1,'real(8)',OP_READ), &
&                      op_arg_dat(d_ndv ,-1,OP_ID,1,'real(8)',OP_READ), &
&                      op_arg_gbl(dt05,1,'real(8)',OP_READ))

    !# Missing code here that can't be merged
    ! Update geometry and iso-parametric terms
    CALL getgeom2(d_ndxu,d_ndyv,d_elx,d_ely)
    !# Missing code here that can't be merged
    ! Half step density
    call op_par_loop_3(a_eq_b_over_c,s_elements, &
&                      op_arg_dat(d_rho05,-1,OP_ID,1,'real(8)',OP_WRITE), &
&                      op_arg_dat(d_elmass,-1,OP_ID,1,'real(8)',OP_READ), &
&                      op_arg_dat(d_elvol,-1,OP_ID,1,'real(8)',OP_READ))

    ! Half step internal energy
    CALL getein(dt05,d_ein05,d_elfx,d_elfy,d_elu,d_elv)
    !# Missing code here that can't be merged
    ! Half step pressure
    CALL getpc(d_rho05,d_ein05,d_pre05,d_csqrd)
    !# Missing code here that can't be merged

    ! ###############
    ! Corrector
    ! ###############
    ! Artificial viscosity
    CALL getq(d_elx,d_ely,d_elu,d_elv,d_rho05,  &
&             d_pre05,d_dx,d_dy,d_elfx,d_elfy,d_scratch)
    ! Force
    CALL getforce(nshape,nel,dt,elx(1,1),ely(1,1),elu(1,1),elv(1,1),    &
&                 elfx(1,1),elfy(1,1),pre05(1),rho05(1),.TRUE._lok, &
                  d_elx,d_ely,d_elu,d_elv,d_elfx,d_elfy,d_pre05,d_rho05)
    ! Acceleration
    CALL getacc(dt05,dt,d_elfx,d_elfy,d_ndxu,d_ndyv,d_elu,d_elv,d_rho05)
    ! Update geometry and iso-parametric terms
    CALL getgeom2(d_ndx,d_ndy,d_elx,d_ely)
    !# Missing code here that can't be merged
    ! Full step density
    call op_par_loop_3(a_eq_b_over_c,s_elements, &
&                      op_arg_dat(d_rho,-1,OP_ID,1,'real(8)',OP_WRITE), &
&                      op_arg_dat(d_elmass,-1,OP_ID,1,'real(8)',OP_READ), &
&                      op_arg_dat(d_elvol,-1,OP_ID,1,'real(8)',OP_READ))
    ! full step internal energy update
    CALL getein(dt,d_ein,d_elfx,d_elfy,d_elu,d_elv)
    !# Missing code here that can't be merged
    ! Full step pressure
    CALL getpc(d_rho,d_ein,d_pre,d_csqrd)
    !# Missing code here that can't be merged

    ! Timing data
    t1=get_time()
    t1=t1-t0
    bookleaf_times%time_in_lag=bookleaf_times%time_in_lag+t1

  END SUBROUTINE lagstep

END MODULE lagstep_mod