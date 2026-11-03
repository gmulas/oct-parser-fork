program example_f
  use, intrinsic :: iso_c_binding, only: c_double
  use oct_parser
  implicit none
  integer :: natoms, l
  real(c_double) :: cutoff, x
  character(len=:), allocatable :: title, sym
  character(len=256) :: arg
  type(oct_block) :: blk

  arg = "example.inp"
  if (command_argument_count() > 0) call get_command_argument(1, arg)

  call oct_parser_init("parser_f.log")
  call oct_parser_input_file(trim(arg))

  call oct_parse("NAtoms", 1, natoms)
  call oct_parse("Cutoff", 10.0_c_double, cutoff)
  call oct_parse("Title", "untitled", title)
  print '(a,": ",i0," atoms, cutoff ",f6.2)', title, natoms, cutoff

  if (oct_block_open("Atoms", blk)) then
    do l = 0, oct_block_n(blk) - 1          ! block indices are 0-based
      call oct_block_get(blk, l, 0, sym)
      call oct_block_get(blk, l, 1, x)
      print '(2x,a2,f8.3)', sym, x
    end do
    call oct_block_close(blk)
  end if

  call oct_parser_end()
end program example_f
