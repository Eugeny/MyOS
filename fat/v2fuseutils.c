
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

#include <config.h>
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "v2fuseutils.h"

void v2f_usage(char *progname,struct fuse_operations *ops)
{
	  int argc = 2;
		static char *argv[] = {"...", "-h"};
		fprintf(stderr,
				"Usage: %s imagefile mountpoint [options]\n"
				"or at your choice:\n"
				"       %s [options] imagefile mountpoint\n", progname,progname);
		fprintf(stderr,"valid fuse options follow:\n");
#if FUSE_USE_VERSION < 26
		fuse_main( argc, argv, ops);
#else
		fuse_main( argc, argv, ops, NULL);
#endif
		exit(1);
}

void v2f_rearrangeargv(int argc, char *argv[])
{
	  int i,sourcearg,dasho;
		  for (i=1,dasho=sourcearg=0;i<argc && sourcearg==0;i++) {
				    if (*argv[i] != '-' && !dasho)
							      sourcearg=i;
						    dasho=(strcmp(argv[i],"-o")==0);
								  }
			  if (sourcearg > 1 && sourcearg < argc-1) {
					    char *sourcepath=argv[sourcearg];
							    char *mountpoint=argv[sourcearg+1];
									    for (i=sourcearg; i>1; i--)
												      argv[i+1]=argv[i-1];
											    argv[1]=sourcepath;
													    argv[2]=mountpoint;
															  }
}

int v2f_checkrorwplus(int argc, char *argv[])
{
	int i,result=0;
	for (i=0;i<argc;i++)
		if (strcmp(argv[i],"-o")==0) {
			char *s=argv[i+1];
			char *t=s;
			int status=0;
			while (*s) {
				//printf("%c %d\n",*s,status);
				switch(status) {
					case 0: /* beginning of token */
						if (*s=='r')
							status=1;
						else
							status=2;
						*(t++)=*(s++);
						break;
					case 1: /* leading r */
						if (*s=='o')
							status=3;
						else if (*s=='w')
							status=4;
						else if (*s==',')
							status=0;
						else
							status=2;
						*(t++)=*(s++);
						break;
					case 2: /* no match */
						if (*s==',')
							status=0;
						*(t++)=*(s++);
						break;
					case 3: /* ro */
						if (*s==',') {
							result |= FLRO;
							status=0;
						} else
							status = 2;
						*(t++)=*(s++);
						break;
					case 4:
						*t=*s;
						if (*s=='+') 
							status=5;
						else if (*s==',') {
							result |= FLRW;
							*t='o';
							status = 0;
						} else
							status = 2;
						t++;s++;
						break;
					case 5:
						if (*s==',') {
							result |= FLRWPLUS;
							status = 0;
							t--;
						} else 
							status = 2;
						*(t++)=*(s++);
						break;
				}
			}
			switch (status) {
				case 3: result |= FLRO; break;
				case 4: result |= FLRW; break;
				case 5: result |= FLRWPLUS; t--;break;
			}
			*t=0;
		}
	return result;
}

int v2f_printwarning(int rorwplus) {
	if (!rorwplus || (rorwplus & FLRW)) {
		fprintf(stderr,"This is experimental code, opening rw a real file system could be\n"
				"dangerous for your data. Please add \"-o ro\" if you want to open the file\n"
				"system image in read-only mode, or \"-o rw+\" if you accept the risk to test\n"
				"this module\n");
		return 1;
	} else
		return 0;
}


