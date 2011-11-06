/* libc/sys/linux/sys/termios.h - Terminal control definitions */

/* Written 2000 by Werner Almesberger */


#ifndef _SYS_TERMIOS_H
#define _SYS_TERMIOS_H

int tcgetattr(int fd,void *termios_p);
int tcsetattr(int fd,int optional_actions,const void *termios_p);

#endif
