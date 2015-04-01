!
! auto-generated by op2.py on 2015-01-26 18:29
!

MODULE GETQ_CHRISTIENSEN_LIMITER_MODULE
USE OP2_FORTRAN_DECLARATIONS
USE OP2_FORTRAN_RT_SUPPORT
USE ISO_C_BINDING
USE kinds_mod,    ONLY: ink,rlk
USE parameters_mod,ONLY: LI


CONTAINS

!DEC$ ATTRIBUTES FORCEINLINE :: getq_christiensen_limiter
SUBROUTINE getq_christiensen_limiter(du,dv,qx,qy,csqrd,rho,scratch,cq1,cq2,iside)
    USE kinds_mod,ONLY: rlk
    USE parameters_mod,ONLY: N_SHAPE

    implicit none

    REAL(KIND=rlk), DIMENSION(N_SHAPE), INTENT(IN) :: du,dv,scratch
    REAL(KIND=rlk), INTENT(IN) :: csqrd,rho,cq1,cq2
    REAL(KIND=rlk), DIMENSION(N_SHAPE), INTENT(OUT) :: qx,qy
    INTEGER(KIND=ink), INTENT(IN) :: iside
    INTEGER(KIND=ink) :: is1,is2
    REAL(KIND=rlk) :: w1,w2,w3,w4

    is1=MOD(iside+2_ink,N_SHAPE)+1_ink
    is2=iside+1_ink

    w1=cq1*SQRT(csqrd)
    w2=scratch(1)
    w3=scratch(2)
    w2=MIN(0.5_rlk*(w2+w3),2.0_rlk*w2,2.0_rlk*w3,1.0_rlk)
    w2=MAX(0.0_rlk,w2)
    w3=du(is1)
    w4=dv(is1)
    w3=SQRT(w3*w3+w4*w4)
    w3=(1.0_rlk-w2)*rho*(w1+cq2*w3)
    qx(is1)=w3
    qy(is1)=w3
    w2=scratch(3)
    w3=scratch(4)
    w2=MIN(0.5_rlk*(w2+w3),2.0_rlk*w2,2.0_rlk*w3,1.0_rlk)
    w2=MAX(0.0_rlk,w2)
    w3=du(is2)
    w4=dv(is2)
    w3=SQRT(w3*w3+w4*w4)
    w3=(1.0_rlk-w2)*rho*(w1+cq2*w3)
    qx(is2)=w3
    qy(is2)=w3

  END SUBROUTINE getq_christiensen_limiter


SUBROUTINE op_wrap_getq_christiensen_limiter( &
  & opDat1Local, &
  & opDat2Local, &
  & opDat3Local, &
  & opDat4Local, &
  & opDat5Local, &
  & opDat6Local, &
  & opDat7Local, &
  & opDat8Local, &
  & opDat9Local, &
  & opDat10Local, &
  & bottom,top)
  real(8) opDat1Local(4,*)
  real(8) opDat2Local(4,*)
  real(8) opDat3Local(4,*)
  real(8) opDat4Local(4,*)
  real(8) opDat5Local(1,*)
  real(8) opDat6Local(1,*)
  real(8) opDat7Local(4,*)
  real(8) opDat8Local(1)
  real(8) opDat9Local(1)
  integer(4) opDat10Local(1)
  INTEGER(kind=4) bottom,top,i1

  DO i1 = bottom, top-1, 1
! kernel call
  CALL getq_christiensen_limiter( &
    & opDat1Local(1,i1+1), &
    & opDat2Local(1,i1+1), &
    & opDat3Local(1,i1+1), &
    & opDat4Local(1,i1+1), &
    & opDat5Local(1,i1+1), &
    & opDat6Local(1,i1+1), &
    & opDat7Local(1,i1+1), &
    & opDat8Local(1), &
    & opDat9Local(1), &
    & opDat10Local(1) &
    & )
  END DO
END SUBROUTINE
SUBROUTINE getq_christiensen_limiter_host( userSubroutine, set, &
  & opArg1, &
  & opArg2, &
  & opArg3, &
  & opArg4, &
  & opArg5, &
  & opArg6, &
  & opArg7, &
  & opArg8, &
  & opArg9, &
  & opArg10 )

  IMPLICIT NONE
  character(kind=c_char,len=*), INTENT(IN) :: userSubroutine
  type ( op_set ) , INTENT(IN) :: set

  type ( op_arg ) , INTENT(IN) :: opArg1
  type ( op_arg ) , INTENT(IN) :: opArg2
  type ( op_arg ) , INTENT(IN) :: opArg3
  type ( op_arg ) , INTENT(IN) :: opArg4
  type ( op_arg ) , INTENT(IN) :: opArg5
  type ( op_arg ) , INTENT(IN) :: opArg6
  type ( op_arg ) , INTENT(IN) :: opArg7
  type ( op_arg ) , INTENT(IN) :: opArg8
  type ( op_arg ) , INTENT(IN) :: opArg9
  type ( op_arg ) , INTENT(IN) :: opArg10

  type ( op_arg ) , DIMENSION(10) :: opArgArray
  INTEGER(kind=4) :: numberOfOpDats
  INTEGER(kind=4), DIMENSION(1:8) :: timeArrayStart
  INTEGER(kind=4), DIMENSION(1:8) :: timeArrayEnd
  REAL(kind=8) :: startTime
  REAL(kind=8) :: endTime
  INTEGER(kind=4) :: returnSetKernelTiming
  INTEGER(kind=4) :: n_upper
  type ( op_set_core ) , POINTER :: opSetCore

  real(8), POINTER, DIMENSION(:) :: opDat1Local
  INTEGER(kind=4) :: opDat1Cardinality

  real(8), POINTER, DIMENSION(:) :: opDat2Local
  INTEGER(kind=4) :: opDat2Cardinality

  real(8), POINTER, DIMENSION(:) :: opDat3Local
  INTEGER(kind=4) :: opDat3Cardinality

  real(8), POINTER, DIMENSION(:) :: opDat4Local
  INTEGER(kind=4) :: opDat4Cardinality

  real(8), POINTER, DIMENSION(:) :: opDat5Local
  INTEGER(kind=4) :: opDat5Cardinality

  real(8), POINTER, DIMENSION(:) :: opDat6Local
  INTEGER(kind=4) :: opDat6Cardinality

  real(8), POINTER, DIMENSION(:) :: opDat7Local
  INTEGER(kind=4) :: opDat7Cardinality

  real(8), POINTER, DIMENSION(:) :: opDat8Local
  real(8), POINTER, DIMENSION(:) :: opDat9Local
  integer(4), POINTER, DIMENSION(:) :: opDat10Local

  INTEGER(kind=4) :: i1

  numberOfOpDats = 10

  opArgArray(1) = opArg1
  opArgArray(2) = opArg2
  opArgArray(3) = opArg3
  opArgArray(4) = opArg4
  opArgArray(5) = opArg5
  opArgArray(6) = opArg6
  opArgArray(7) = opArg7
  opArgArray(8) = opArg8
  opArgArray(9) = opArg9
  opArgArray(10) = opArg10

  returnSetKernelTiming = setKernelTime(23 , userSubroutine//C_NULL_CHAR, &
  & 0.d0, 0.00000,0.00000, 0)
  call op_timers_core(startTime)

  n_upper = op_mpi_halo_exchanges(set%setCPtr,numberOfOpDats,opArgArray)

  opSetCore => set%setPtr

  opDat1Cardinality = opArg1%dim * getSetSizeFromOpArg(opArg1)
  opDat2Cardinality = opArg2%dim * getSetSizeFromOpArg(opArg2)
  opDat3Cardinality = opArg3%dim * getSetSizeFromOpArg(opArg3)
  opDat4Cardinality = opArg4%dim * getSetSizeFromOpArg(opArg4)
  opDat5Cardinality = opArg5%dim * getSetSizeFromOpArg(opArg5)
  opDat6Cardinality = opArg6%dim * getSetSizeFromOpArg(opArg6)
  opDat7Cardinality = opArg7%dim * getSetSizeFromOpArg(opArg7)
  CALL c_f_pointer(opArg1%data,opDat1Local,(/opDat1Cardinality/))
  CALL c_f_pointer(opArg2%data,opDat2Local,(/opDat2Cardinality/))
  CALL c_f_pointer(opArg3%data,opDat3Local,(/opDat3Cardinality/))
  CALL c_f_pointer(opArg4%data,opDat4Local,(/opDat4Cardinality/))
  CALL c_f_pointer(opArg5%data,opDat5Local,(/opDat5Cardinality/))
  CALL c_f_pointer(opArg6%data,opDat6Local,(/opDat6Cardinality/))
  CALL c_f_pointer(opArg7%data,opDat7Local,(/opDat7Cardinality/))
  CALL c_f_pointer(opArg8%data,opDat8Local, (/opArg8%dim/))
  CALL c_f_pointer(opArg9%data,opDat9Local, (/opArg9%dim/))
  CALL c_f_pointer(opArg10%data,opDat10Local, (/opArg10%dim/))


  CALL op_mpi_wait_all(numberOfOpDats,opArgArray)
  CALL op_wrap_getq_christiensen_limiter( &
  & opDat1Local, &
  & opDat2Local, &
  & opDat3Local, &
  & opDat4Local, &
  & opDat5Local, &
  & opDat6Local, &
  & opDat7Local, &
  & opDat8Local, &
  & opDat9Local, &
  & opDat10Local, &
  & 0, n_upper)

  CALL op_mpi_set_dirtybit(numberOfOpDats,opArgArray)

  call op_timers_core(endTime)

  returnSetKernelTiming = setKernelTime(23 , userSubroutine//C_NULL_CHAR, &
  & endTime-startTime,0.00000,0.00000, 1)
END SUBROUTINE
END MODULE