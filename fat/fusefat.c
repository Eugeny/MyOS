/*
    FUSEFAT: fat32 filesystem implementation for FUSE
	
	FUSEFAT: Copyright (C) 2006-2007  Paolo Angelelli <angelell@cs.unibo.it>
		    Various acknowledgments to Renzo Davoli <renzo@cs.unibo.it>,
					       Ludovico Gardenghi <gardengl@cs.unibo.it>
		
	FUSE:    Copyright (C) 2001-2006  Miklos Szeredi <miklos@szeredi.hu>
	
    This program can be distributed under the terms of the GNU GPL.
*/

#include <config.h>

#include "libfat.h"
#include <fuse.h>
#include "v2fuseutils.h"

#define fusefat_getvolume(V)     struct fuse_context *mycontext = fuse_get_context(); V = (Volume_t *) mycontext->private_data;
	

static int fusefat_getattr(const char *path, struct stat *stbuf) {
    int res;
	File_t F;
	Volume_t *V;
	fusefat_getvolume(V);
	fat_lock(V);
	if ((res = fat_open(path, &F, V, O_RDONLY)) != 0) { fat_unlock(V); fprintf(stderr,"-- %d",__LINE__); return -ENOENT; }
	if ((res = fat_stat(&F, stbuf)) != 0) { fat_unlock(V); fprintf(stderr,"-- %d",__LINE__);return -1; }
	fat_unlock(V);
	fprintf(stderr,"getattr(%s)\n",path);
    return 0;
}

static int fusefat_open(const char *path, struct fuse_file_info *fi) {
    int res;
	File_t *F;
	Volume_t *V;
	fusefat_getvolume(V);
	F = malloc(sizeof(File_t));
	fat_lock(V);
	if ((res = fat_open(path, F, V, O_RDWR)) != 0) { fat_unlock(V); fprintf(stderr,"-- %d",__LINE__); free(F); return -ENOENT; }
	fat_unlock(V);
	fi->fh = (long)F;
	fprintf(stderr,"open(%s)\n",path);
    return 0;
}

static int fusefat_access(const char *path, int mask) {
	return 0;
//    return fusefat_open(path, NULL);
}

static int fusefat_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
    struct dirent de;
    (void) offset;
    (void) fi;	
    int res;
	File_t F;
	Volume_t *V;
	fusefat_getvolume(V);
	fprintf(stderr,"readdir(%s)\n",path);
	fat_lock(V);
    if ((res =  fat_open(path, &F, V, O_RDONLY)) != 0) { fat_unlock(V); fprintf(stderr,"-- %d",__LINE__); return -ENOENT; }

    while ((res = fat_readdir(&F, &de)) == 0) {
        struct stat st;
        memset(&st, 0, sizeof(st));
        st.st_ino = de.d_ino;
		if (de.d_type == DT_DIR) { 
			st.st_mode = S_IFDIR;
		} else st.st_mode = S_IFREG;


//		st.st_mode = de.d_type << 12;
		
        if (filler(buf, de.d_name, &st, 0))
            break;
    }
	fat_unlock(V);
    return 0;
}

static int fusefat_release(const char *path, struct fuse_file_info *fi)
{
    /* Just a stub.  This method is optional and can safely be left
       unimplemented */

//    (void) path;
//    (void) fi;
	free((File_t *) ((long)(fi->fh)));
    return 0;
}


static int fusefat_mknod(const char *path, mode_t mode, dev_t rdev) {
    int res;
	char dirname[4096];
  char filename[1024];
	File_t Parent;
	Volume_t *V;
	fusefat_getvolume(V);
/*    if (!(S_ISREG(mode))) {
		return -1;
	} */

	fat_dirname(path, dirname);
  fat_filename(path, filename);
	fat_lock(V);
	fprintf(stderr,"dirname: %s, filename: %s\n", dirname, filename);
	if ((res =  fat_open(dirname, &Parent, V, O_RDWR)) != 0) { fat_unlock(V); fprintf(stderr,"-- %d",__LINE__); return -ENOENT; }
	if ((res =  fat_create(V, &Parent, filename , NULL, S_IFREG, 0)) != 0) { fat_unlock(V); fprintf(stderr,"-- %d",__LINE__); return -1; }
	fat_unlock(V);
    return 0;
}

static int fusefat_mkdir(const char *path, mode_t mode) {
    int res;
    char dirname[4096];
    char filename[1024];
    File_t Parent;
	Volume_t *V;
	fusefat_getvolume(V);
    fat_dirname(path, dirname);
    fat_filename(path, filename);

	fat_lock(V);
    if ((res =  fat_open(dirname, &Parent, V, O_RDWR)) != 0) { fat_unlock(V); fprintf(stderr,"-- %d",__LINE__); return -ENOENT; }
    if ((res =  fat_mkdir(V, &Parent, filename , NULL, S_IFDIR)) != 0) { fat_unlock(V); fprintf(stderr,"-- %d",__LINE__); return -1; }

	fat_unlock(V);
    return 0;
}

static int fusefat_unlink(const char *path) {
    int res;
	File_t F;
	Volume_t *V;
	fusefat_getvolume(V);
	fat_lock(V);
	if ((res =  fat_open(path, &F, V, O_RDWR)) != 0) { fat_unlock(V); fprintf(stderr,"-- %d",__LINE__); return -ENOENT; }
	if ((res =  fat_delete(&F, 0)) != 0) { fat_unlock(V); fprintf(stderr,"-- %d",__LINE__); return -1; }
	fat_unlock(V);
    return 0;
}

static int fusefat_rmdir(const char *path) {
    int res;
	File_t F;
	Volume_t *V;
	fusefat_getvolume(V);
	fat_lock(V);
	if ((res =  fat_open(path, &F, V, O_RDWR)) != 0) { fat_unlock(V); fprintf(stderr,"-- %d",__LINE__); return -ENOENT; }
	if ((res =  fat_rmdir(&F)) != 0) { fat_unlock(V); fprintf(stderr,"-- %d",__LINE__); return -1; }
	fat_unlock(V);
    return 0;
}

// rename in libfat has bugs
static int fusefat_rename(const char *from, const char *to) {
    int res;
	Volume_t *V;
	fusefat_getvolume(V);

	fprintf(stderr,"from: %s, to: %s\n",from,to);
	fat_lock(V);

	if ((res =  fat_rename(V,from,to)) != 0) { fat_unlock(V); fprintf(stderr,"-- %d",__LINE__); return res; }

	fat_unlock(V);
    return 0;
}

static int fusefat_truncate(const char *path, off_t size) {
    int res;
	File_t F;
	Volume_t *V;
	fusefat_getvolume(V);
	fprintf(stderr,"truncate(%s, %d)\n",path,(int) size);
	fat_lock(V);
    if ((res =  fat_open(path, &F, V, O_RDWR)) != 0) { fat_unlock(V); fprintf(stderr,"-- %d",__LINE__); return -ENOENT; }
    if ((res =  fat_truncate(&F, size)) != 0) { fat_unlock(V); fprintf(stderr,"-- %d",__LINE__); return -1; }
	fat_unlock(V);
    return 0;
}

static int fusefat_utime(const char *path, struct utimbuf *buf) {
    int res;
	File_t F;
	Volume_t *V;
	fusefat_getvolume(V);
	fat_lock(V);
    if ((res =  fat_open(path, &F, V, O_RDONLY)) != 0) { fat_unlock(V); fprintf(stderr,"-- %d",__LINE__); return -ENOENT; }
    if ((res =  fat_utime(&F, buf)) != 0) { fat_unlock(V); fprintf(stderr,"-- %d",__LINE__); return -1; }
	fat_unlock(V);
    return 0;
}

static int fusefat_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    int res, mode;
	DWORD fsize;
	File_t *F;
	Volume_t *V;
	fusefat_getvolume(V);
	F = (File_t *) fi->fh;
	fat_lock(V);
	
	fsize = EFD(F->DirEntry->DIR_FileSize);
	if ((size + (int) offset) > fsize) size = (fsize - offset);
	
	mode = F->Mode;
	F->Mode = O_RDONLY;
	
//    if ((res =  fat_open(path, &F, V, O_RDONLY)) != 0) { fat_unlock(V); fprintf(stderr,"-- %d",__LINE__); return -ENOENT; }
    if ((res =  fat_seek(F, offset, SEEK_SET)) != offset) { fat_unlock(V); fprintf(stderr,"-- %d",__LINE__); return -1; }
	if ((fat_iseoc(V, F->CurClus)) || fat_isfree(V,F->CurClus)) { fat_unlock(V); fprintf(stderr,"-- %d",__LINE__); return -1; }
	
    if ((res =  fat_read_data(V, &(F->CurClus), &(F->CurOff), buf, size )) <= 0) { fat_unlock(V); fprintf(stderr,"-- %d",__LINE__); return -1; }
	F->CurAbsOff += res;
	F->Mode = mode;
	fat_unlock(V);
    return res;
}

static int fusefat_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    int res;
	File_t *F;
	Volume_t *V;
	fusefat_getvolume(V);
	F=(File_t *)fi->fh;
	//if ((fi->flags & O_RDONLY) == O_RDONLY) { fprintf(stderr,"fusefat_write(): file opened in read only mode\n");; return -1; }
	
	fat_lock(V);
//    if ((res =  fat_open(path, &F, V, O_RDWR)) != 0) { fat_unlock(V); fprintf(stderr,"-- %d",__LINE__); return -ENOENT; }
//	fprintf(stderr,"fusefat_write: performing seek at offset %d\n", (int) offset);
    if ((res =  fat_seek(F, offset, SEEK_SET)) < 0) { fat_update_file(F); fat_unlock(V); fprintf(stderr,"-- %d",__LINE__); return -1; }
//	fprintf(stderr,"fusefat_write: performing write_data at clus: %u, off: %u\n", F->CurClus, F->CurOff);
    if ((res =  fat_write_data(V, F,&(F->CurClus), &(F->CurOff), buf, size )) != size) { 
		fat_update_file(F); fat_unlock(V); fprintf(stderr,"-- %d:",__LINE__); fprintf(stderr,"fat_write_data() error\n");   return -1; }
//	fprintf(stderr,"fusefat_write: performing update\n");		
	if ((res =  fat_update_file(F)) != 0) { fat_unlock(V); fprintf(stderr,"fat_update_file() error\n"); fprintf(stderr,"-- %d",__LINE__); return -1; }
	fat_unlock(V);
    return size;
}

static int fusefat_statvfs(const char *path, struct statvfs *stbuf) {
    int res;
	Volume_t *V;
	fusefat_getvolume(V);
	fat_lock(V);
	fat_statvfs(V, stbuf);
	fat_unlock(V);
    return 0;
}

static int fusefat_fsync(const char *path, int isdatasync, struct fuse_file_info *fi) {
    /* Just a stub.  This method is optional and can safely be left
       unimplemented */

    (void) path;
    (void) isdatasync;
    (void) fi;

    return 0;
}

static int fusefat_flush(const char *path, struct fuse_file_info *fi) {

    int res;
	Volume_t *V;
	fusefat_getvolume(V);
	fat_lock(V);
	fat_fat_sync(V);
	fat_unlock(V);
	return 0;
}

#if ( FUSE_MINOR_VERSION <= 5 )
static void *init_data;

void *fusefat_init(void)
{
	return init_data;
}
#else
void *fusefat_init(struct fuse_conn_info *conn)
{
	struct fuse_context *mycontext;
	mycontext=fuse_get_context();
#ifndef DEBUG
	printf("INIT %p\n",mycontext->private_data);
#endif
	return mycontext->private_data;
}   
#endif


static struct fuse_operations fusefat_oper = {
	.init		=	fusefat_init,
	//	.destroy	=	,
	//	.lookup		=	NULL,
	//	.forget		= 	NULL,		// forget an opened fh
	.getattr	= fusefat_getattr,
	.access		= fusefat_access,		// check file access permission
	//    .readlink	= fusefat_readlink,	
	.readdir	= fusefat_readdir,
	.mknod		= fusefat_mknod,
	//	.create = ,		
	.mkdir		= fusefat_mkdir,
	//    .symlink	= fusefat_symlink,
	.unlink		= fusefat_unlink,		// remove a file
	.rmdir		= fusefat_rmdir,
	.rename		= fusefat_rename,
	//    .link	= fusefat_link,
	//    .chmod	= NULL,
	//    .chown	= NULL,
	.truncate	= fusefat_truncate,
	//    .utimens	= fusefat_utimens,
	.utime	= fusefat_utime,
	.open	= fusefat_open,
	.opendir= NULL,
	.read	= fusefat_read,
	.write	= fusefat_write,
	.statfs	= fusefat_statvfs,
	.release	= fusefat_release,	//we should avoid to delete a file if multiple processes are using it.
	.releasedir = NULL,
	.fsync	= fusefat_fsync,	//sync
	.flush  = fusefat_flush
};

static void rearrangeargv(int argc, char *argv[])
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

int main(int argc, char *argv[])
{
	Volume_t fat32_volume;
	Volume_t *V = &fat32_volume;
	int rorwplus;

	struct fuse_chan *fuse_fd;

	char *pathname ;
	char *mountpoint;

	int res;

	if (argc < 3) { 
		v2f_usage(argv[0],&fusefat_oper);
		return -ENODEV;
	}
	v2f_rearrangeargv(argc,argv);
	pathname=argv[1];
	argv[1]=argv[0];

	rorwplus=v2f_checkrorwplus(argc-2,argv+2);
	if (v2f_printwarning(rorwplus))
		return -EINVAL;

	if (rorwplus == FLRWPLUS) fprintf(stderr,"volume mounted in rw mode\n");
	
	if ((res = fat_partition_init(V,pathname,
					(rorwplus==FLRWPLUS)?FAT_WRITE_ACCESS_FLAG:0)) < 0) return -1;		

	//   umask(0);
#if ( FUSE_MINOR_VERSION <= 5 )
	init_data=V;
	res =  fuse_main(--argc, ++argv, &fusefat_oper);
#else
	res =  fuse_main(--argc, ++argv, &fusefat_oper, V);
#endif

	res = fat_partition_finalize(V);
	return res;
}
