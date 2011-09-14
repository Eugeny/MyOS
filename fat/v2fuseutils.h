
/*   This is part of um-ViewOS
 *   The user-mode implementation of OSVIEW -- A Process with a View
 *
 *   v2fuseutils utility functions for fuse
 *   
 *   Copyright 2007 Renzo Davoli University of Bologna - Italy
 *   
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License along
 *   with this program; if not, write to the Free Software Foundation, Inc.,
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */

#ifndef __V2FUSEUTILS_H
#define __V2FUSEUTILS_H
#include <config.h>
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "v2fuseutils.h"

#define FLRO 1
#define FLRW 2
#define FLRWPLUS 4

/* standard usage for v2 fuse modules */
void v2f_usage(char *progname,struct fuse_operations *ops);

/* move source and mountpoint in front */
void v2f_rearrangeargv(int argc, char *argv[]);

/* check for ro, rw or rw+ options*/
int v2f_checkrorwplus(int argc, char *argv[]);

/* arg=output of v2f_checkrorwplus, it prints a warning 
 * when there is rw (or nothing) but not rw+
 * returns 1 when the warning has been printed*/
int v2f_printwarning(int rorwplus);
#endif
