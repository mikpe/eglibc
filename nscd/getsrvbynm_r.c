/* Copyright (C) 1996, 1997, 2002, 2007 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1996.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2 as
   published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

#include <netdb.h>


#define LOOKUP_TYPE		struct servent
#define FUNCTION_NAME		getservbyname
#define DATABASE_NAME		services
#define ADD_PARAMS		const char *name, const char *proto
#define ADD_VARIABLES		name, proto

#include "../nss/getXXbyYY_r.c"
