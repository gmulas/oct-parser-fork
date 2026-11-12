! Copyright (C) 2026 Giacomo Mulas
!
! This program is free software; you can redistribute it and/or modify
! it under the terms of the GNU General Public License as published by
! the Free Software Foundation; either version 2, or (at your option)
! any later version.
!
! This program is distributed in the hope that it will be useful,
! but WITHOUT ANY WARRANTY; without even the implied warranty of
! MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
! GNU General Public License for more details.

!! Fortran 2008 interface to liboct_parser.
!!
!! This module is not part of Octopus: it is a standalone replacement for the
!! Octopus-specific Fortran layer (src/basic/parser.F90 + liboct_parser/parser_f.c),
!! written only with standard ISO_C_BINDING, so it needs no name-mangling
!! configuration and no Octopus modules.
!!
!! Conventions:
!!  - Block lines and columns are 0-based, as in the C API and in Octopus.
!!  - Strings are returned as deferred-length allocatable characters.
!!  - Logical values are read as integers (the parser predefines yes/no,
!!    true/false): any non-zero value is .true.
!!  - Procedures that can fail take an optional ierr argument. If ierr is
!!    present it receives 0 on success; if it is absent, an error stops the
!!    program with a message.
!!
!! Distributed under the GNU GPL, version 2 or later.
module oct_parser
  use, intrinsic :: iso_c_binding
  implicit none
  private

  !> Handle to an input block (an opaque pointer to the C sym_block).
  type, public :: oct_block
    private
    type(c_ptr) :: p = c_null_ptr
    character(len=:), allocatable :: name
  end type oct_block

  public :: oct_parser_init, oct_parser_end
  public :: oct_parser_input_file, oct_parser_input_string, oct_parser_environment
  public :: oct_parser_isdef, oct_parse, oct_parser_putsym
  public :: oct_parser_eval, oct_parser_output_table
  public :: oct_block_open, oct_block_close, oct_block_n, oct_block_cols, oct_block_get
  public :: oct_block_is_open

  !> Read a scalar variable, with a default used when it is not in the input.
  interface oct_parse
    module procedure parse_i4, parse_i8, parse_r8, parse_c8, parse_l, parse_str
  end interface oct_parse

  !> Define (or redefine) a variable in the symbol table from the program.
  interface oct_parser_putsym
    module procedure putsym_i4, putsym_r8, putsym_c8
  end interface oct_parser_putsym

  !> Read element (line, column) of a block; both indices are 0-based.
  interface oct_block_get
    module procedure block_i4, block_i8, block_r8, block_c8, block_l, block_str
  end interface oct_block_get

  ! ------------------------------------------------------------------
  ! Bindings to the C API (liboct_parser.h and oct_parser_fbind.c)
  ! ------------------------------------------------------------------
  interface
    integer(c_int) function c_parse_init(file_out, mpiv_node) bind(c, name="parse_init")
      import :: c_int, c_char
      character(kind=c_char), intent(in) :: file_out(*)
      integer(c_int), intent(in) :: mpiv_node
    end function c_parse_init

    integer(c_int) function c_parse_input(file_in, set_used) bind(c, name="parse_input")
      import :: c_int, c_char
      character(kind=c_char), intent(in) :: file_in(*)
      integer(c_int), value :: set_used
    end function c_parse_input

    integer(c_int) function c_parse_input_string(str, set_used) bind(c, name="parse_input_string")
      import :: c_int, c_char
      character(kind=c_char), intent(in) :: str(*)
      integer(c_int), value :: set_used
    end function c_parse_input_string

    subroutine c_parse_environment(prefix) bind(c, name="parse_environment")
      import :: c_char
      character(kind=c_char), intent(in) :: prefix(*)
    end subroutine c_parse_environment

    subroutine c_parse_end() bind(c, name="parse_end")
    end subroutine c_parse_end

    integer(c_int) function c_parse_isdef(name) bind(c, name="parse_isdef")
      import :: c_int, c_char
      character(kind=c_char), intent(in) :: name(*)
    end function c_parse_isdef

    integer(c_int64_t) function c_parse_int(name, def) bind(c, name="parse_int")
      import :: c_int64_t, c_char
      character(kind=c_char), intent(in) :: name(*)
      integer(c_int64_t), value :: def
    end function c_parse_int

    real(c_double) function c_parse_double(name, def) bind(c, name="parse_double")
      import :: c_double, c_char
      character(kind=c_char), intent(in) :: name(*)
      real(c_double), value :: def
    end function c_parse_double

    subroutine c_parse_complex(name, def, res) bind(c, name="octp_f_parse_complex")
      import :: c_double_complex, c_char
      character(kind=c_char), intent(in) :: name(*)
      complex(c_double_complex), intent(in) :: def
      complex(c_double_complex), intent(out) :: res
    end subroutine c_parse_complex

    type(c_ptr) function c_parse_string(name, def) bind(c, name="parse_string")
      import :: c_ptr, c_char
      character(kind=c_char), intent(in) :: name(*), def(*)
    end function c_parse_string

    subroutine c_putsym_int(name, i) bind(c, name="parse_putsym_int")
      import :: c_int, c_char
      character(kind=c_char), intent(in) :: name(*)
      integer(c_int), value :: i
    end subroutine c_putsym_int

    subroutine c_putsym_double(name, d) bind(c, name="parse_putsym_double")
      import :: c_double, c_char
      character(kind=c_char), intent(in) :: name(*)
      real(c_double), value :: d
    end subroutine c_putsym_double

    subroutine c_putsym_complex(name, z) bind(c, name="octp_f_putsym_complex")
      import :: c_double_complex, c_char
      character(kind=c_char), intent(in) :: name(*)
      complex(c_double_complex), intent(in) :: z
    end subroutine c_putsym_complex

    integer(c_int) function c_eval(expr, res) bind(c, name="octp_f_eval")
      import :: c_int, c_double_complex, c_char
      character(kind=c_char), intent(in) :: expr(*)
      complex(c_double_complex), intent(out) :: res
    end function c_eval

    integer(c_int) function c_rmsym(name) bind(c, name="octp_f_rmsym")
      import :: c_int, c_char
      character(kind=c_char), intent(in) :: name(*)
    end function c_rmsym

    subroutine c_free(p) bind(c, name="octp_f_free")
      import :: c_ptr
      type(c_ptr), value :: p
    end subroutine c_free

    subroutine c_sym_output_table(only_unused, mpiv_node) bind(c, name="sym_output_table")
      import :: c_int
      integer(c_int), value :: only_unused, mpiv_node
    end subroutine c_sym_output_table

    integer(c_int) function c_parse_block(name, blk) bind(c, name="parse_block")
      import :: c_int, c_char, c_ptr
      character(kind=c_char), intent(in) :: name(*)
      type(c_ptr), intent(out) :: blk
    end function c_parse_block

    integer(c_int) function c_parse_block_end(blk) bind(c, name="parse_block_end")
      import :: c_int, c_ptr
      type(c_ptr), intent(inout) :: blk
    end function c_parse_block_end

    integer(c_int) function c_parse_block_n(blk) bind(c, name="parse_block_n")
      import :: c_int, c_ptr
      type(c_ptr), value :: blk
    end function c_parse_block_n

    integer(c_int) function c_parse_block_cols(blk, l) bind(c, name="parse_block_cols")
      import :: c_int, c_ptr
      type(c_ptr), value :: blk
      integer(c_int), value :: l
    end function c_parse_block_cols

    integer(c_int) function c_parse_block_int(blk, l, col, r) bind(c, name="parse_block_int")
      import :: c_int, c_ptr
      type(c_ptr), value :: blk
      integer(c_int), value :: l, col
      integer(c_int), intent(out) :: r
    end function c_parse_block_int

    integer(c_int) function c_parse_block_int8(blk, l, col, r) bind(c, name="parse_block_int8")
      import :: c_int, c_int64_t, c_ptr
      type(c_ptr), value :: blk
      integer(c_int), value :: l, col
      integer(c_int64_t), intent(out) :: r
    end function c_parse_block_int8

    integer(c_int) function c_parse_block_double(blk, l, col, r) bind(c, name="parse_block_double")
      import :: c_int, c_double, c_ptr
      type(c_ptr), value :: blk
      integer(c_int), value :: l, col
      real(c_double), intent(out) :: r
    end function c_parse_block_double

    integer(c_int) function c_parse_block_complex(blk, l, col, r) bind(c, name="parse_block_complex")
      import :: c_int, c_double_complex, c_ptr
      type(c_ptr), value :: blk
      integer(c_int), value :: l, col
      complex(c_double_complex), intent(out) :: r
    end function c_parse_block_complex

    integer(c_int) function c_parse_block_string(blk, l, col, r) bind(c, name="parse_block_string")
      import :: c_int, c_ptr
      type(c_ptr), value :: blk
      integer(c_int), value :: l, col
      type(c_ptr), intent(out) :: r
    end function c_parse_block_string

    integer(c_size_t) function c_strlen(s) bind(c, name="strlen")
      import :: c_size_t, c_ptr
      type(c_ptr), value :: s
    end function c_strlen
  end interface

contains

  ! ------------------------------------------------------------------
  ! Helpers
  ! ------------------------------------------------------------------
  pure function cstr(s) result(c)
    character(len=*), intent(in) :: s
    character(kind=c_char, len=len_trim(s)+1) :: c
    c = trim(s) // c_null_char
  end function cstr

  !> Copy a malloc'ed C string into a Fortran allocatable string, then free it.
  function take_cstring(p) result(s)
    type(c_ptr), intent(in) :: p
    character(len=:), allocatable :: s
    character(kind=c_char), pointer :: chars(:)
    integer :: n, i

    if (.not. c_associated(p)) then
      s = ""
      return
    end if
    n = int(c_strlen(p))
    call c_f_pointer(p, chars, [n])
    allocate(character(len=n) :: s)
    do i = 1, n
      s(i:i) = chars(i)
    end do
    call c_free(p)
  end function take_cstring

  subroutine set_or_stop(ierr, code, msg)
    integer, optional, intent(out) :: ierr
    integer, intent(in) :: code
    character(len=*), intent(in) :: msg
    if (present(ierr)) then
      ierr = code
    else if (code /= 0) then
      write(0, '(a)') "oct_parser: " // msg
      error stop 1
    end if
  end subroutine set_or_stop

  ! ------------------------------------------------------------------
  ! Initialisation and input
  ! ------------------------------------------------------------------

  !> Initialise the symbol table. log_file receives a record of every value
  !> read ("-" means stdout). Only rank 0 writes the log (default rank 0).
  subroutine oct_parser_init(log_file, rank, ierr)
    character(len=*), intent(in) :: log_file
    integer, optional, intent(in) :: rank
    integer, optional, intent(out) :: ierr
    integer(c_int) :: r
    r = 0
    if (present(rank)) r = int(rank, c_int)
    call set_or_stop(ierr, int(c_parse_init(cstr(log_file), r)), &
      "cannot open parser log file '" // trim(log_file) // "'")
  end subroutine oct_parser_init

  !> Read an input file. If set_used is .true., all the variables found are
  !> marked as used (they will not be reported by oct_parser_output_table).
  subroutine oct_parser_input_file(file_name, set_used, ierr)
    character(len=*), intent(in) :: file_name
    logical, optional, intent(in) :: set_used
    integer, optional, intent(out) :: ierr
    integer(c_int) :: su
    su = 0
    if (present(set_used)) then
      if (set_used) su = 1
    end if
    call set_or_stop(ierr, int(c_parse_input(cstr(file_name), su)), &
      "cannot parse input file '" // trim(file_name) // "'")
  end subroutine oct_parser_input_file

  !> Parse input given as a string (lines separated by new_line('a')).
  subroutine oct_parser_input_string(contents, set_used, ierr)
    character(len=*), intent(in) :: contents
    logical, optional, intent(in) :: set_used
    integer, optional, intent(out) :: ierr
    integer(c_int) :: su
    su = 0
    if (present(set_used)) then
      if (set_used) su = 1
    end if
    call set_or_stop(ierr, int(c_parse_input_string(contents // c_null_char, su)), &
      "cannot parse input string")
  end subroutine oct_parser_input_string

  !> If the environment variable <prefix>PARSE_ENV is set, read all the
  !> environment variables starting with prefix, overriding the input file
  !> (e.g. with prefix "OCT_", OCT_Radius=2 sets Radius = 2).
  subroutine oct_parser_environment(prefix)
    character(len=*), intent(in) :: prefix
    call c_parse_environment(cstr(prefix))
  end subroutine oct_parser_environment

  !> Free the symbol table and close the log file.
  subroutine oct_parser_end()
    call c_parse_end()
  end subroutine oct_parser_end

  logical function oct_parser_isdef(name)
    character(len=*), intent(in) :: name
    oct_parser_isdef = c_parse_isdef(cstr(name)) /= 0
  end function oct_parser_isdef

  !> Print the symbol table; with only_unused=.true. print (to stderr) the
  !> assignments that the program never read, which usually are typos.
  subroutine oct_parser_output_table(only_unused, rank)
    logical, intent(in) :: only_unused
    integer, optional, intent(in) :: rank
    integer(c_int) :: u, r
    u = 0
    if (only_unused) u = 1
    r = 0
    if (present(rank)) r = int(rank, c_int)
    call c_sym_output_table(u, r)
  end subroutine oct_parser_output_table

  ! ------------------------------------------------------------------
  ! Scalars
  ! ------------------------------------------------------------------
  subroutine parse_i4(name, def, res)
    character(len=*), intent(in) :: name
    integer(c_int), intent(in) :: def
    integer(c_int), intent(out) :: res
    integer(c_int64_t) :: r
    r = c_parse_int(cstr(name), int(def, c_int64_t))
    if (r > huge(res) .or. r < -huge(res) - 1) then
      write(0, '(a)') "oct_parser: value of '" // trim(name) // "' does not fit a default integer"
      error stop 1
    end if
    res = int(r, c_int)
  end subroutine parse_i4

  subroutine parse_i8(name, def, res)
    character(len=*), intent(in) :: name
    integer(c_int64_t), intent(in) :: def
    integer(c_int64_t), intent(out) :: res
    res = c_parse_int(cstr(name), def)
  end subroutine parse_i8

  subroutine parse_r8(name, def, res)
    character(len=*), intent(in) :: name
    real(c_double), intent(in) :: def
    real(c_double), intent(out) :: res
    res = c_parse_double(cstr(name), def)
  end subroutine parse_r8

  subroutine parse_c8(name, def, res)
    character(len=*), intent(in) :: name
    complex(c_double_complex), intent(in) :: def
    complex(c_double_complex), intent(out) :: res
    call c_parse_complex(cstr(name), def, res)
  end subroutine parse_c8

  subroutine parse_l(name, def, res)
    character(len=*), intent(in) :: name
    logical, intent(in) :: def
    logical, intent(out) :: res
    integer(c_int64_t) :: idef
    idef = 0
    if (def) idef = 1
    res = c_parse_int(cstr(name), idef) /= 0
  end subroutine parse_l

  subroutine parse_str(name, def, res)
    character(len=*), intent(in) :: name
    character(len=*), intent(in) :: def
    character(len=:), allocatable, intent(out) :: res
    res = take_cstring(c_parse_string(cstr(name), def // c_null_char))
  end subroutine parse_str

  subroutine putsym_i4(name, val)
    character(len=*), intent(in) :: name
    integer(c_int), intent(in) :: val
    call c_putsym_int(cstr(name), val)
  end subroutine putsym_i4

  subroutine putsym_r8(name, val)
    character(len=*), intent(in) :: name
    real(c_double), intent(in) :: val
    call c_putsym_double(cstr(name), val)
  end subroutine putsym_r8

  subroutine putsym_c8(name, val)
    character(len=*), intent(in) :: name
    complex(c_double_complex), intent(in) :: val
    call c_putsym_complex(cstr(name), val)
  end subroutine putsym_c8

  ! ------------------------------------------------------------------
  ! Expressions
  ! ------------------------------------------------------------------

  !> Evaluate an expression, e.g. "exp(-r^2/2)*cos(k*x)", with the symbols
  !> of the input plus, optionally, temporary variables names(i)=values(i)
  !> (removed again afterwards, as in Octopus). ierr: 0 ok, 1 syntax error,
  !> 2 the expression is a string, 3 names/values have different sizes.
  function oct_parser_eval(expr, names, values, ierr) result(z)
    character(len=*), intent(in) :: expr
    character(len=*), optional, intent(in) :: names(:)
    real(c_double), optional, intent(in) :: values(:)
    integer, optional, intent(out) :: ierr
    complex(c_double_complex) :: z
    integer :: i, st
    integer(c_int) :: idummy

    z = (0.0_c_double, 0.0_c_double)
    if (present(names) .neqv. present(values)) then
      call set_or_stop(ierr, 3, "oct_parser_eval: names and values must be given together")
      return
    end if
    if (present(names)) then
      if (size(names) /= size(values)) then
        call set_or_stop(ierr, 3, "oct_parser_eval: names and values have different sizes")
        return
      end if
      do i = 1, size(names)
        call c_putsym_double(cstr(names(i)), values(i))
      end do
    end if

    st = int(c_eval(expr // c_null_char, z))

    if (present(names)) then
      do i = 1, size(names)
        idummy = c_rmsym(cstr(names(i)))
      end do
    end if

    select case (st)
    case (1)
      call set_or_stop(ierr, st, "syntax error in expression '" // trim(expr) // "'")
    case (2)
      call set_or_stop(ierr, st, "expression '" // trim(expr) // "' is not a number")
    case default
      call set_or_stop(ierr, 0, "")
    end select
  end function oct_parser_eval

  ! ------------------------------------------------------------------
  ! Blocks
  ! ------------------------------------------------------------------

  !> Open a block. Returns .true. if the block exists in the input.
  logical function oct_block_open(name, blk) result(found)
    character(len=*), intent(in) :: name
    type(oct_block), intent(out) :: blk
    found = c_parse_block(cstr(name), blk%p) == 0
    if (found) then
      blk%name = trim(name)
    else
      blk%p = c_null_ptr
    end if
  end function oct_block_open

  subroutine oct_block_close(blk)
    type(oct_block), intent(inout) :: blk
    integer(c_int) :: idummy
    if (c_associated(blk%p)) idummy = c_parse_block_end(blk%p)
    blk%p = c_null_ptr
    if (allocated(blk%name)) deallocate(blk%name)
  end subroutine oct_block_close

  logical function oct_block_is_open(blk)
    type(oct_block), intent(in) :: blk
    oct_block_is_open = c_associated(blk%p)
  end function oct_block_is_open

  !> Number of lines in the block.
  integer function oct_block_n(blk)
    type(oct_block), intent(in) :: blk
    call check_open(blk)
    oct_block_n = int(c_parse_block_n(blk%p))
  end function oct_block_n

  !> Number of columns in line l (0-based).
  integer function oct_block_cols(blk, l)
    type(oct_block), intent(in) :: blk
    integer, intent(in) :: l
    call check_open(blk)
    if (l < 0 .or. l >= oct_block_n(blk)) then
      write(0, '(a,i0,a)') "oct_parser: line ", l, " out of range in block '" // blk%name // "'"
      error stop 1
    end if
    oct_block_cols = int(c_parse_block_cols(blk%p, int(l, c_int)))
  end function oct_block_cols

  subroutine check_open(blk)
    type(oct_block), intent(in) :: blk
    if (.not. c_associated(blk%p)) then
      write(0, '(a)') "oct_parser: operation on a block that is not open"
      error stop 1
    end if
  end subroutine check_open

  subroutine block_err(blk, l, c, what, code, ierr)
    type(oct_block), intent(in) :: blk
    integer, intent(in) :: l, c, code
    character(len=*), intent(in) :: what
    integer, optional, intent(out) :: ierr
    character(len=32) :: buf
    write(buf, '(i0,",",i0)') l, c
    call set_or_stop(ierr, code, "block '" // blk%name // "' has no " // what // &
      " at (line,col) = (" // trim(buf) // ")")
  end subroutine block_err

  subroutine block_i4(blk, l, c, res, ierr)
    type(oct_block), intent(in) :: blk
    integer, intent(in) :: l, c
    integer(c_int), intent(out) :: res
    integer, optional, intent(out) :: ierr
    call check_open(blk)
    call block_err(blk, l, c, "integer", &
      int(c_parse_block_int(blk%p, int(l, c_int), int(c, c_int), res)), ierr)
  end subroutine block_i4

  subroutine block_i8(blk, l, c, res, ierr)
    type(oct_block), intent(in) :: blk
    integer, intent(in) :: l, c
    integer(c_int64_t), intent(out) :: res
    integer, optional, intent(out) :: ierr
    call check_open(blk)
    call block_err(blk, l, c, "integer", &
      int(c_parse_block_int8(blk%p, int(l, c_int), int(c, c_int), res)), ierr)
  end subroutine block_i8

  subroutine block_r8(blk, l, c, res, ierr)
    type(oct_block), intent(in) :: blk
    integer, intent(in) :: l, c
    real(c_double), intent(out) :: res
    integer, optional, intent(out) :: ierr
    call check_open(blk)
    call block_err(blk, l, c, "real number", &
      int(c_parse_block_double(blk%p, int(l, c_int), int(c, c_int), res)), ierr)
  end subroutine block_r8

  subroutine block_c8(blk, l, c, res, ierr)
    type(oct_block), intent(in) :: blk
    integer, intent(in) :: l, c
    complex(c_double_complex), intent(out) :: res
    integer, optional, intent(out) :: ierr
    call check_open(blk)
    call block_err(blk, l, c, "complex number", &
      int(c_parse_block_complex(blk%p, int(l, c_int), int(c, c_int), res)), ierr)
  end subroutine block_c8

  subroutine block_l(blk, l, c, res, ierr)
    type(oct_block), intent(in) :: blk
    integer, intent(in) :: l, c
    logical, intent(out) :: res
    integer, optional, intent(out) :: ierr
    integer(c_int64_t) :: i
    call block_i8(blk, l, c, i, ierr)
    res = i /= 0
  end subroutine block_l

  subroutine block_str(blk, l, c, res, ierr)
    type(oct_block), intent(in) :: blk
    integer, intent(in) :: l, c
    character(len=:), allocatable, intent(out) :: res
    integer, optional, intent(out) :: ierr
    type(c_ptr) :: p
    integer :: st
    call check_open(blk)
    p = c_null_ptr
    st = int(c_parse_block_string(blk%p, int(l, c_int), int(c, c_int), p))
    if (st == 0) then
      res = take_cstring(p)
    else
      res = ""
    end if
    call block_err(blk, l, c, "string", st, ierr)
  end subroutine block_str

end module oct_parser
