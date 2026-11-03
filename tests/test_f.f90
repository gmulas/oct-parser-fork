! Test of the Fortran module oct_parser (run by "make check").
program test_f
  use, intrinsic :: iso_c_binding, only: c_double, c_double_complex, c_int64_t
  use oct_parser
  implicit none
  integer, parameter :: dp = c_double
  integer :: failures, n, ierr, ival
  integer(c_int64_t) :: i8
  real(dp) :: r
  complex(c_double_complex) :: z
  logical :: l
  character(len=:), allocatable :: s, input
  character(len=512) :: buf
  type(oct_block) :: blk

  failures = 0
  call get_environment_variable("OCTP_INPUT_FILE", buf, status=ierr)
  if (ierr == 0) then
    input = trim(buf)
  else
    input = "test.inp"
  end if

  call oct_parser_init("test_f.log_parser")
  call oct_parser_input_file(input)
  call oct_parser_environment("OCTP_TEST_")

  call oct_parse("CalculationMode", 0, ival);       call check(ival == 2, "CalculationMode")
  call oct_parse("NumberOfSteps", 0_c_int64_t, i8);  call check(i8 == 100, "NumberOfSteps int64")
  call oct_parse("NotInInput", -7, ival);           call check(ival == -7, "default")
  call oct_parse("BoxSize", 0.0_dp, r);             call check(abs(r - 5.0_dp) < 1e-12_dp, "BoxSize")
  call oct_parse("Phase", (0.0_dp, 0.0_dp), z);     call check(abs(z - (0.0_dp, 1.0_dp)) < 1e-12_dp, "Phase")
  call oct_parse("Verbose", .false., l);            call check(l, "Verbose")
  call oct_parse("Debug", .true., l);               call check(.not. l, "Debug")
  call oct_parse("Title", "none", s);               call check(s == "Water molecule", "Title")
  call oct_parse("NoTitle", "a default", s);        call check(s == "a default", "string default")
  call check(oct_parser_isdef("Spacing") .and. .not. oct_parser_isdef("Nope"), "isdef")

  call check(oct_block_open("Species", blk), "open Species")
  call check(oct_block_n(blk) == 2, "block_n")
  call check(oct_block_cols(blk, 1) == 4, "block_cols")
  call oct_block_get(blk, 1, 0, s);   call check(s == "O", "block string")
  call oct_block_get(blk, 1, 1, r);   call check(abs(r - 15.9994_dp) < 1e-12_dp, "block real")
  call oct_block_get(blk, 1, 2, ival); call check(ival == 8, "block int")
  call oct_block_get(blk, 1, 3, z);   call check(abs(z - (2.0_dp, 3.0_dp)) < 1e-12_dp, "block complex")
  call oct_block_close(blk)
  call check(.not. oct_block_is_open(blk), "block closed")
  call check(.not. oct_block_open("NoSuchBlock", blk), "missing block")

  z = oct_parser_eval("x^2 + y", ["x", "y"], [3.0_dp, 1.0_dp])
  call check(abs(z - (10.0_dp, 0.0_dp)) < 1e-12_dp, "eval with variables")
  call check(.not. oct_parser_isdef("x"), "eval variables removed")
  z = oct_parser_eval("1 +* 2", ierr=ierr)
  call check(ierr == 1, "eval syntax error")
  z = oct_parser_eval("2*Spacing")
  call check(abs(real(z) - 0.5_dp) < 1e-12_dp, "eval with input symbols")

  call oct_parser_putsym("Injected", 1.5_dp)
  call oct_parse("Injected", 0.0_dp, r); call check(abs(r - 1.5_dp) < 1e-12_dp, "putsym")

  call oct_parser_input_string("FromString = 3" // new_line('a') // "%B" // new_line('a') // &
       " 1 | 2" // new_line('a') // "%" // new_line('a'))
  call oct_parse("FromString", 0, ival); call check(ival == 3, "input string")
  call check(oct_block_open("B", blk), "block from string")
  n = oct_block_cols(blk, 0); call check(n == 2, "block from string cols")
  call oct_block_close(blk)

  call oct_parser_end()

  if (failures == 0) then
    print '(a)', "test_f: all checks passed"
  else
    error stop 1
  end if

contains
  subroutine check(cond, what)
    logical, intent(in) :: cond
    character(len=*), intent(in) :: what
    if (.not. cond) then
      write(0, '(a)') "FAILED: " // what
      failures = failures + 1
    end if
  end subroutine check
end program test_f
