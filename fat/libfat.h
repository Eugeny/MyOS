//This is "libfat.h" - Driver's constants, structures and prototypes    

#ifndef __LIBFAT_H
#define __LIBFAT_H

//#define FATWRITE
#define _LARGEFILE64_SOURCE
//#define __USE_LARGEFILE64

#include <config.h>
#include <stdint.h>
#include <stdio.h>
#include <dirent.h>
#include <linux/types.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "unicode.h"
//#include <glib.h>
#include <sys/vfs.h>
#include <sys/statvfs.h>
#include <utime.h>

/* All data types are UNSIGNED . types from  <stdint.h> */

#define BYTE 	uint8_t
#define WORD	uint16_t
#define DWORD	uint32_t

#ifndef LIBFAT_USE_MUTEX
#define LIBFAT_USE_MUTEX 1
#endif

struct Volume_t;
struct File_t;
struct DirEntry_t;

#define __INSIDE_LIBFAT_H
#include "bits/libfat.h"
#undef __INSIDE_LIBFAT_H
#define FAT_WRITE_ACCESS_FLAG 1

/*	Prototypes			*/
int fat_partition_init(Volume_t *V, char *pathname, int flags);
int fat_partition_finalize(Volume_t *V);

int fat_read_data(Volume_t *V, DWORD *Cluster, DWORD *Offset, char *buf, size_t count );
int fat_write_data(Volume_t *V, File_t *F, DWORD *Cluster, DWORD *Offset, char *buf, int count );
int fat_update_file(File_t *F);
int fat_create(Volume_t *V, File_t *parent, char *filename , DirEntry_t *sfn, mode_t mode, int dirflag);
int fat_mkdir(Volume_t *V, File_t *parent, char *filename , DirEntry_t *sfn, mode_t mode);
int fat_delete(File_t *F, int dir);
int fat_rmdir(File_t *F);
int fat_truncate(File_t *F, DWORD len);
int fat_rename(Volume_t *V, const char *from, const char *to);
int fat_open(const char *uft8path, File_t *F, Volume_t *V, int flags);
off64_t fat_seek(File_t *F, off64_t offset, int whence);
int fat_stat(File_t *F, struct stat *st);
int set_fstclus(Volume_t *V, DirEntry_t *D, DWORD c);
DWORD get_fstclus(Volume_t *V, DirEntry_t *D);
int fat_utime(File_t *F, struct utimbuf *buf);
int fat_statvfs(Volume_t *V, struct statvfs *buf);
int fat_readdir(File_t *Dir, struct dirent *de);
off64_t byte_offset(Volume_t *V, DWORD Cluster, DWORD Offset);

/*	Functions for string conversions		*/

int utf16to8(const WORD *restrict source, char *restrict dest);
int utf8_strlen(char *s);
int utf8to16(const char *restrict source, WORD *restrict dest);
int utf16toASCII(WORD *restrict source, char *restrict dest, int len);
int utf8_stricmp(const char *s1, const char *s2);
int utf8_strncmp(const char *s1, const char *s2, int n);
int utf8_strchk(char *s);
int fat_dirname(const char *path, char *dest);
int fat_filename(const char *path, char *dest);
#endif /* #ifdef __LIBFAT_H  */
