/*
 Copyright (C) 2024 A.Buccheri, H. Menke

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2, or (at your option)
 any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 02110-1301, USA.

*/
#ifndef OCTOPUS_ERROR_HANDLING_H
#define OCTOPUS_ERROR_HANDLING_H

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define AT __FILE__ ":" TOSTRING(__LINE__)

#define error_abort(...) do { error_impl(AT ": ", sizeof(AT ": ") - 1, __VA_ARGS__); abort(); } while (0)
#define error_return(code, ...) do { error_impl(AT ": ", sizeof(AT ": ") - 1, __VA_ARGS__); return (code); } while (0)

static void error_impl(const char *loc, const size_t len, const char *format, ...) {
    enum { bufsz = 1024 };
    char buffer[bufsz];
    snprintf(buffer, bufsz, "%s", loc);
    va_list args;
    va_start(args, format);
    vsnprintf(buffer + len, bufsz - len, format, args);
    va_end(args);
    perror(buffer);
}

#endif //OCTOPUS_ERROR_HANDLING_H
