/*OS
 	libfat: library to access a fat32 filesystem

	Rev 0.3	aka "Beta version"

	Copyright (C) 2006-2007  Paolo Angelelli <angelell@cs.unibo.it>

	Acknowledgments to Salvatore Isaja, auctor of fat support for freedos32
	since utf8/16 routines are taken from freedos32 unicode module,
	Renzo Davoli for bugfixes, big endian support and more.

	This software is free software; it can be (re)distributed under the terms
	of the GNU GPL General Public License as published by the Free Software
	Foundation; either version 2 of the License, or any later version.

	The Program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
*/


/**********************************************
*
*	LIBFAT.c
*	Current known problems (limitations) :
*	- May have problems if the filesistem
*	  was accessed before with MS-DOS or
*	  something that does not support Long Filenames
*	- to be cont'd	=)
*	TODO: fix function to create time to support
		timezone.
* 	TODO: Refuse to mount if dirty bit is up
***********************************************/

#include "libfat.h"


#ifndef POPULATE_FREELIST_BUFSZ
#define POPULATE_FREELIST_BUFSZ 8192
#endif

off64_t byte_offset(Volume_t *V, DWORD Cluster, DWORD Offset) {
	off64_t clus = Cluster;
	off64_t off	 = Offset;

	if (Cluster == 1)
		if (V->FatType != FAT32) return (V->rootdir16off + off);
	/* else error */

	return  ( ((clus - 2) * V->bpc64) + V->fdb64 + off );
}

int fat_isfree(Volume_t *V,DWORD value) {
    if (V->FatType == FAT32) {
        return FAT32_ISFREE(value);
    } else if (V->FatType == FAT16) {
        return FAT16_ISFREE(value);
    } else {
        return FAT12_ISFREE(value);
    }
    return 0;
}

int fat_isbad(Volume_t *V,DWORD value) {
    if (V->FatType == FAT32) {
        return FAT32_ISBAD(value);
    } else if (V->FatType == FAT16) {
        return FAT16_ISBAD(value);
    } else {
        return FAT12_ISBAD(value);
    }
    return 0;
}

int fat_iseoc(Volume_t *V,DWORD value) {
    if (V->FatType == FAT32) {
        return FAT32_ISEOC(value);
    } else if (V->FatType == FAT16) {
        return FAT16_ISEOC(value);
    } else {
        return FAT12_ISEOC(value);
    }
    return 0;
}

int fat_legalclus(Volume_t *V,DWORD value) {
    if (V->FatType == FAT32) {
        return FAT32_LEGALCLUS(value);
    } else if (V->FatType == FAT16) {
        return FAT16_LEGALCLUS(value);
    } else {
        return FAT12_LEGALCLUS(value);
    }
    return 0;
}

DWORD fat_eocvalue(Volume_t *V) {
    if (V->FatType == FAT32) {
        return FAT32_EOC_VALUE;
    } else if (V->FatType == FAT16) {
        return FAT16_EOC_VALUE;
    } else {
        return FAT12_EOC_VALUE;
    }
    return 0;
}

static ssize_t readn(int fd, void *buf, size_t count) {
	int done = 0;
	int res;

	while (count > 0) {
		res = read(fd, (void *) &(((char *) buf)[done]), count);
		if (res <= 0) {	// 0 indicates EOF, and it should never happen here.
			fprintf(stderr,"read() error. line: %d\n",__LINE__);
			return -1;
		} else {
			done += res;
			count -= res;
		}
	}
	return done;
}

static ssize_t writen(int fd, const char *buf, size_t count) {
	int done = 0;
	int res;

	while (count > 0) {
		res = write(fd, &(buf[done]), count);
		if (res < 0) {
			perror("write() error");
			return -1;
		} else {
			done += res;
			count -= res;
		}
	}
	return done;
}


time_t fat_mktime(int s, int m, int h, int d, int mo, int y) {
	struct tm t;
	memset((char *) &t, 0, sizeof(struct tm));

	t.tm_sec=s;         /* seconds */
    t.tm_min=m;         /* minutes */
    t.tm_hour=h;        /* hours */
    t.tm_mday=d;        /* day of the month */
    t.tm_mon=mo;         /* month */
    t.tm_year=y;        /* year */

 	return mktime(&t);
}

time_t fat_mktime2(DirEntry_t *D) {
	int s=((((BYTE *) &(D->DIR_WrtTime))[0] & 0x1f) * 2);
	int m=((((((BYTE *) &(D->DIR_WrtTime))[1]&0x7) << 3) + (((BYTE *) &(D->DIR_WrtTime))[0] >> 5)));
	int h=(((BYTE *) &(D->DIR_WrtTime))[1] >> 3);
	int d=(((BYTE *) &(D->DIR_WrtDate))[0] & 0x1f);
	int mo=((((((BYTE *) &(D->DIR_WrtDate))[1]&0x1) << 3) + (((BYTE *) &(D->DIR_WrtDate))[0] >> 5)));
	int y=(( ((BYTE *) &(D->DIR_WrtDate))[1] >> 1) + 80);
 	return fat_mktime(s,m,h,d,mo,y);
}

int fat_fill_time(WORD *Date, WORD *Time, time_t t) {	// works ok.
	struct tm time;
	WORD date=0;
	WORD tim=0;
	WORD bmask3=0x07FF;
	WORD bmask2=0x01FF;
	WORD bmask1=0x001F;

	gmtime_r(&t, &time);

	date = (WORD) time.tm_mday;
	date &= bmask1; // to set 0 first 11 bits;
	date |= ((WORD) time.tm_mon) << 5;
	date &= bmask2; // to set 0 first 6 bits;
	date |= (((WORD) ((time.tm_year + 1900) -1980)) << 9);

	tim = (WORD) (time.tm_sec / 2);
	tim &= bmask1;
	tim |= (((WORD) (time.tm_min)) << 5);
	tim &= bmask3;
	tim |= (((WORD) (time.tm_hour)) << 11);

	*Time = EFW(tim);
	*Date = EFW(date);
	return 0;
}
/************************************************************************
*	Functions to access the FAT										 	*
************************************************************************/
/* FAT32 PRIMITIVES */

/* The location of a valid cluster N into the FAT32 FAT       */
/* Returns 0 on success, or a negative error code on failure. */
/* Called by fat32_read_entry and fat32_write_entry.          */
/* Put into Sector the sector number where the entry is, and  */
/* into EntryOffset the offset of the entry in the sector     */
/* FatNum = number of which fat we want to look into. 0 is 1st*/
/* N = the cluster we want sector and offset of. first is   2 */
#if 0	// Useless atm
static int fat32_cluster_entry(Volume_t *V, DWORD N, int FatNum,
                               DWORD *Sector, DWORD *EntryOffset) {
  DWORD FATSz, EntryPerSec32;
  WORD bytspersec = EFW(V->Bpb.BPB_BytsPerSec);
//  DWORD entry;

  if (N > V->DataClusters + 1) return -ENXIO;

  FATSz = EFD(V->Bpb.BPB_FATSz32) * bytspersec;

//  entry = ( FatNum * FATSz ) + ( EFW(V->Bpb.BPB_ResvdSecCnt) * bytspersec ) + (N * 4);

 // printf("---- %u\n",entry);
  EntryPerSec32 = EFW(V->Bpb.BPB_BytsPerSec) / 4; /* fat32 entries are 4 bytes long */

  *Sector      = ( FatNum * FATSz ) + EFW(V->Bpb.BPB_ResvdSecCnt) +
		    ( N / EntryPerSec32 );
  *EntryOffset = ( (N % EntryPerSec32) * 4 );

  return 0;
}
#endif



/* The location of a valid cluster N into the FAT32 FAT       */
/* Returns the offset of the entry on success, or a negative  */
/*									error code on failure.    */
/* FatNum = number of which fat we want to look into. 0 is 1st*/
/* N = the cluster we want sector and offset of. first is   2 */
off64_t fat32_cluster_off(Volume_t *V, DWORD N, int FatNum) {
  off64_t rsvdbc = V->rsvdbytecnt;
  off64_t fatn	= FatNum;
  off64_t fatsz = V->fatsz;
  off64_t entry = N;
  off64_t entrysz = 4;
  return ( rsvdbc + ( fatn * fatsz )  + ( entry * entrysz ));
}


/* Reads the value of the specified cluster entry in a FAT32 FAT.      */
/* Returns 0 on success, or a negative error code on failure.          */
int fat32_read_entry(Volume_t *V, DWORD N, int FatNum, DWORD *Value)
{
  int   Res;
//  DWORD Sector, EntryOffset;
  DWORD Entry;

  off64_t fatoffset;

/*	finding position	*/

  if ((fatoffset = fat32_cluster_off(V, N, FatNum)) <= 0) return (int) fatoffset;
//  if ((Res = fat_readbuf(V, Sector)) < 0) return Res;
/*	lseek(SEEK_SET)	*/


  if ( (Res = lseek64(V->blkDevFd, fatoffset, SEEK_SET)) < 0) {
    fprintf(stderr,"lseek() error in fat32_read_entry(). N: %u, off: %lld\n",N, fatoffset);
    return Res;
  }
/*	read()		*/

  if ( (Res = readn(V->blkDevFd, &Entry, 4)) != 4) {
    fprintf(stderr,"readn() error in fat32_read_entry(). N: %u, off: %lld\n",N, fatoffset);
    return -1;
  }

  *Value = EFD(Entry) & 0x0FFFFFFF;
  return 0;
}

#ifdef FATWRITE
/* Writes the value of the specified cluster entry in a FAT32 FAT.          */
/* Returns 0 on success, or a negative error code on failure.               */

int fat32_write_entry(Volume_t *V, DWORD N, int FatNum, DWORD Value) {
  int   Res;
//  DWORD Sector, EntryOffset;
  DWORD val;
  off64_t off;

  Value &= 0x0FFFFFFF;

  if ((Res = fat32_read_entry(V, N, FatNum, &val)) != 0) {
  	perror("fat32_write_entry error");
  	return -1;
  }

  val &= 0xF0000000;
  Value = Value | val;

/*	finding position	*/

  if ((off = fat32_cluster_off(V, N, FatNum)) <= 0) return (int) off;

/*	lseek(SEEK_SET)	*/

  if ( (Res = lseek64(V->blkDevFd, off, SEEK_SET)) < 0) {
    perror("lseek() error in fat32_read_entry()");
    return -1;
  }

/*	write()		*/

	Value=EFD(Value);
  if ( (Res = writen(V->blkDevFd, (char*) &Value, 4)) != 4) {
    perror("writen() error in fat32_read_entry()");
    return -1;
  }

  return 0;
}

/* Writes the value of the specified cluster entry in all FAT32 FATs of the volume V */
/* Returns 0 on success, or a negative error code on failure.               */

int fat32_writen_entry(Volume_t *V, DWORD N, DWORD Value) {
	int i;
	int numfats = V->numfats;

	for (i=0; i < numfats; i++) {
		if ( fat32_write_entry(V, N, i, Value) != 0) {
			perror("fat32_write_entry error in fat32_writen_entry()");
			return -1;
		}
	}
	return 0;
}

/* Unlinks (marks as free) all the clusters of the chain starting from */
/* cluster Cluster (that must be part of a valid chain) until the EOC. */

static int fat32_unlinkn(Volume_t *V, DWORD Cluster) {
  int   Res;
  DWORD Next;
  int i =0;

  if ((fat_isfree(V,Cluster)) || (FAT32_ISEOC(Cluster))) return 0;
  if (FAT32_ISBAD(Cluster)) return -1;

  do {
    /* Read the next entry in the chain */
    if ( (Res = fat32_read_entry(V, Cluster, 0, &Next)) != 0 ) {
		fprintf(stderr,"unlinkn() error cycle: %d Line: %d",i, __LINE__);
		return -1;
	}

    /* Update every FAT in the volume */
    if ( (Res = fat32_writen_entry(V, Cluster, 0)) != 0 ) {
		fprintf(stderr,"unlinkn() error cycle: %d Line: %d",i,__LINE__);
	  	return -1;
	}

    /* Update FSInfo Sector values	*/
    V->freecnt++;
//    if (Cluster < V->nextfree ) ->nextfree = Cluster;
    Cluster = Next;
 	i++;
 }  while (!(FAT32_ISEOC(Next)));

 fprintf(stderr,"unkinkn: freecnt: %u, i:%d\n", V->freecnt,i);
  return 0;
}

/* FAT16 PRIMITIVES */

static int fat16_read_entry(Volume_t *V, DWORD N, int FatNum, DWORD *Value) {

	*Value = EFW(((WORD*) V->fat)[N]);
	return 0;
}

static int fat16_write_entry(Volume_t *V, DWORD N, int FatNum, DWORD Value) {
	WORD val;
	val = Value;
	val = EFW(val);
	((WORD*) V->fat)[N] = val;
	return 0;
}

static int fat16_unlinkn(Volume_t *V, DWORD Cluster) {
  int   Res;
  DWORD Next;
  int 	i = 0;

  if ((FAT16_ISFREE(Cluster)) || (FAT16_ISEOC(Cluster))) return 0;
  if (FAT16_ISBAD(Cluster)) return -1;

  do {
    /* Read the next entry in the chain */
    if ( (Res = fat16_read_entry(V, Cluster, 0, &Next)) != 0 ) {
		fprintf(stderr,"unlinkn() error cycle: %d Line: %d",i, __LINE__);
		return -1;
	}

    /* Update every FAT in the volume */
    if ( (Res = fat16_write_entry(V, Cluster, 0,  0)) != 0 ) {
		fprintf(stderr,"unlinkn() error cycle: %d Line: %d",i,__LINE__);
	  	return -1;
	}

    /* Update FSInfo Sector values	*/
    V->freecnt++;
//    if (Cluster < V->nextfree ) ->nextfree = Cluster;
    Cluster = Next;
 	i++;
 }  while (!(FAT16_ISEOC(Next)));

  return 0;
}

/* FAT12 PRIMITIVES */

static int fat12_read_entry(Volume_t *V, DWORD N, int FatNum, DWORD *Value) {
	WORD val;
	DWORD out;
					//  lo hi
	if (N%2 == 0) {	//  XX -X
		val = 0;

		((BYTE *) &val)[0] = (BYTE) V->fat[((int) (N * 1.5))];
		((BYTE *) &val)[1] = (BYTE) V->fat[((int) (N * 1.5)) + 1];

		((BYTE *) &val)[1] &= 0x0F;

	} else {
			 /* odd     X- XX
			 	    	 \  \
			            XX -X   */
		val =0;

		((BYTE *) &val)[0] = (BYTE) V->fat[((int) (N * 1.5))];
		((BYTE *) &val)[1] = (BYTE) V->fat[((int) (N * 1.5)) + 1];

		((BYTE *) &val)[0] = (((BYTE *) &val)[0] >> 4);
		((BYTE *) &val)[0] |= (((BYTE *) &val)[1] << 4);
		((BYTE *) &val)[1] = (((BYTE *) &val)[1] >> 4);
	}

	out = val;
	*Value = EFD(out);
	return 0;
}

static int fat12_write_entry(Volume_t *V, DWORD N, int FatNum, DWORD Value) {
	WORD val;
	val = Value;
	val = EFW(val);	// now val is little endian word

	if (N%2 == 0) { // XX -X : just mix hi byte
		*((BYTE *) (V->fat + ((int) (N * 1.5)))) = ((BYTE *) &val)[0];
		*((BYTE *) (V->fat + ((int) (N * 1.5)) + 1)) &= 0xF0;
		*((BYTE *) (V->fat + ((int) (N * 1.5)) + 1)) |= ( ((BYTE *) &val)[1] & 0x0F );
	} else {
	//		XX -X
	//       /  /
	//		X- XX
		*((BYTE *) (V->fat + ((int) (N * 1.5)))) &= 0x0F;
		*((BYTE *) (V->fat + ((int) (N * 1.5)))) |= ( (((BYTE *) &val)[0] << 4) );

		*((BYTE *) (V->fat + ((int) (N * 1.5)) + 1)) = ( (((BYTE *) &val)[1] << 4) );
		*((BYTE *) (V->fat + ((int) (N * 1.5)) + 1)) |= ( (((BYTE *) &val)[0] >> 4) );
	}
//	fprintf(stderr,"fat12_write_entry: N:%u, value:%u\n",N,Value);
	return 0;
}

static int fat12_unlinkn(Volume_t *V, DWORD Cluster) {
  int   Res;
  DWORD Next;
  int 	i = 0;

  if ((fat_isfree(V,Cluster)) || (fat_iseoc(V,Cluster))) return 0;
  if (fat_isbad(V,Cluster)) return -1;

  do {
    /* Read the next entry in the chain */
    if ( (Res = fat12_read_entry(V, Cluster, 0, &Next)) != 0 ) {
		fprintf(stderr,"unlinkn() error cycle: %d Line: %d",i, __LINE__);
		return -1;
	}

    /* Update every FAT in the volume */
    if ( (Res = fat12_write_entry(V, Cluster, 0, 0)) != 0 ) {
		fprintf(stderr,"unlinkn() error cycle: %d Line: %d",i,__LINE__);
	  	return -1;
	}

    /* Update FSInfo Sector values	*/
    V->freecnt++;
//    if (Cluster < V->nextfree ) ->nextfree = Cluster;
    Cluster = Next;
 	i++;
 }  while (!(fat_iseoc(V,Next)));

  return 0;
}

/* GENERAL WRAPPERS FOR THE ABOVE FUNCTIONS                    */
static int fat_read_entry(Volume_t *V, DWORD N, int FatNum, DWORD *Value) {
    if (V->FatType == FAT32) {
        return fat32_read_entry(V, N, FatNum, Value);
    } else if (V->FatType == FAT16) {
      return fat16_read_entry(V, N, FatNum, Value);
    } else {
      return fat12_read_entry(V, N, FatNum, Value);
    }
    return 0;
}

#if 0/* Useless atm	*/
static int fat_write_entry(Volume_t *V, DWORD N, int FatNum, DWORD Value) {
    if (V->FatType == FAT32) {
        return fat32_write_entry(V, N, FatNum, Value);
    } else if (V->FatType == FAT16) {
      return fat16_write_entry(V, N, FatNum, Value);
    } else {
      return fat12_write_entry(V, N, FatNum, Value);
    }
    return 0;
}
#endif

static int fat_writen_entry(Volume_t *V, DWORD N, DWORD Value) {
    if (V->FatType == FAT32) {
        return fat32_writen_entry(V, N, Value);
    } else if (V->FatType == FAT16) {
      return fat16_write_entry(V, N, 0 ,Value);
    } else {
      return fat12_write_entry(V, N, 0, Value);
    }
    return 0;
}

static int fat_unlinkn(Volume_t *V, DWORD Cluster) {
    if (V->FatType == FAT32) {
        return fat32_unlinkn(V, Cluster);
    } else if (V->FatType == FAT16) {
      return fat16_unlinkn(V, Cluster);
    } else {
      return fat12_unlinkn(V, Cluster);
    }
    return 0;
}

/* set the volume as dirty, 										*/
/* the routine set the 5th high bit to 0 for dirty flag				*/
/* bitmaks: 0x08000000. 											*/
static int fat_mark_dirty(Volume_t *V) {
		DWORD val;
		int Res;

	if (V->FatType == FAT32)  {
		DWORD dirty32 = 0xF7FFFFFF;

		if ((Res = fat_read_entry(V, 1, 0, &val)) != 0) {
			perror("fat_read_entry error");
			return Res;
		}

		val = val & dirty32;

		if ((Res = fat_writen_entry(V, 1, val)) != 0) {
			perror("fat_write_entry error");
			return -1;
		}
	} else if (V->FatType == FAT16)  {
		WORD  dirty16 = 0x8000;
		WORD val16;

        if ((Res = fat_read_entry(V, 1, 0, &val)) != 0) {
            perror("fat_read_entry error");
            return Res;
        }

		val16 =  val;
        val16 = val16 & dirty16;

        if ((Res = fat_writen_entry(V, 1, (DWORD) val16)) != 0) {
            perror("fat_write_entry error");
            return -1;
        }
	}
	return 0;
}

/* set the volume as clean, 										*/
/* the routine set the 5th high bit to 1 for clean flag				*/
/* bitmaks: 0x08000000. 											*/
static int fat_mark_clean(Volume_t *V) {
        DWORD val;
        int Res;

	if (V->FatType == FAT32) {
		DWORD clean32 = 0x08000000;

		if ((Res = fat_read_entry(V, 1, 0, &val)) != 0) {
			perror("fat32_read_entry error");
			return Res;
		}

		val = val | clean32;

		if ((Res = fat_writen_entry(V, 1, val)) != 0) {
			perror("fat32_write_entry error");
			return -1;
		}
	} else if  (V->FatType == FAT16)  {
		WORD  clean16 = 0x8000;
		WORD  val16;

        if ((Res = fat_read_entry(V, 1, 0, &val)) != 0) {
            perror("fat_read_entry error");
            return Res;
        }

        val16 =  val;
        val16 = val16 | clean16;

        if ((Res = fat_writen_entry(V, 1, (DWORD) val16)) != 0) {
            perror("fat_write_entry error");
            return -1;
        }
	}
	return 0;
}
#endif /* #ifdef FATWRITE */

/************************************************************************
*	Functions to work on Directory entries			 	*
************************************************************************/
static void printVolumeData(Volume_t *V) {
	DWORD sz;
	if (V->FatType == FAT32) {
		sz = EFD(V->Bpb.BPB_FATSz32);
	} else {
		sz = EFW(V->Bpb.BPB_FATSz16);
		fprintf(stderr,"root dir off : %lld \n", byte_offset(V,1,0));
	}

	fprintf(stderr,"dataclusters :%u  \n", V->DataClusters);
	fprintf(stderr,"first data byte : %lld \n", V->fdb64   );
	fprintf(stderr,"1st fat off :  %d \n", V->rsvdbytecnt );
	fprintf(stderr,"2nd fat off :  %d\n", V->rsvdbytecnt+ (sz * V->bps) );
	fprintf(stderr,"fat_eoc_value: %u\n", fat_eocvalue(V));
	fprintf(stderr,"fat_eoc_value is eoc?: %d\n", fat_iseoc(V,fat_eocvalue(V)));
	return ;
}

static int libfat_determine_fattype(Volume_t *V) {
    int res;
    off_t fsi_offset;
    char type[9];

    sprintf(type,"FAT12   ");
    if ((res = memcmp( (char *) &(((char *) &(V->Bpb))[54]), type, 8)) == 0) { V->FatType = FAT12; fprintf(stderr,"fat type: FAT12\n"); return 0; }
    sprintf(type,"FAT16   ");
    if ((res = memcmp( (char *) &(((char *) &(V->Bpb))[54]), type, 8)) == 0) { V->FatType = FAT16; fprintf(stderr,"fat type: FAT16\n");return 0; }
    sprintf(type,"FAT32   ");
    if ((res = memcmp( (char *) V->Bpb.BS_FilSysType, type, 8)) == 0) {

		fprintf(stderr,"fat type: FAT32. Fsi at %u\n",EFW(V->Bpb.BPB_FSInfo));
        V->FatType = FAT32;
        fsi_offset = EFW(V->Bpb.BPB_FSInfo) * EFW(V->Bpb.BPB_BytsPerSec);
        // Only for FAT32
		fprintf(stderr,"Fsioff: %d, size: %d\n",(int) fsi_offset, sizeof(FSInfo_t));
        if ( (res = lseek(V->blkDevFd, fsi_offset, SEEK_SET)) != fsi_offset ) { perror("FSI lseek() error"); return -1; }
        if ( (res = readn(V->blkDevFd, &(V->Fsi), sizeof(FSInfo_t))) != sizeof(FSInfo_t) ) { perror("FSI readn() error"); return -1; }
		fprintf(stderr,"--- nxtfree --- :%u\n",EFD(V->Fsi.FSI_Nxt_Free));
		fprintf(stderr,"--- freecnt --- :%u\n",EFD(V->Fsi.FSI_Free_Count));
		fflush(stderr);
        return 0;
    }
    /* else */
    return -1;

/* Now we can determine  FAT type */
/*
    if (CountofClusters < 4085) {
        sprintf(buf,"FAT12");
    } else if (CountofClusters < 65525) {
        sprintf(buf,"FAT16");
    } else {
        sprintf(buf,"FAT32");
    }
*/
}

/* load the whole fat into memory for fat12/16 volumes  */
static int libfat_initialize_fat(Volume_t *V) {
	int res;
    int fatsz = 0;  // total size of the fat in bytes

	/* Lets determine the total size of the fat in bytes */
    if (V->FatType == FAT16) {
        fatsz = (V->DataClusters + 2) * sizeof(WORD);
    } else if (V->FatType == FAT12) {
    	fatsz = ((V->DataClusters + 2) / 2) * 3;
        if (((V->DataClusters + 2) % 2) != 0) fatsz += 2;
	} else {
		fprintf(stderr,"unknown fat type in initialise_fat\n");
		return -1;
	}

	/* Memory Map or Load/Restore the FAT? Load/restore for the moment	*/

	V->fat = malloc(fatsz); // up to 128k of memory for the biggest fat16 ever.
	memset(V->fat, 0, fatsz);

	if ( (res = lseek64(V->blkDevFd, V->rsvdbytecnt, SEEK_SET)) < 0) {
		fprintf(stderr,"lseek() error in initialize fat(), off:%d\n",V->rsvdbytecnt);
		return res;
	}
	/*  read()      */

	if ( (res = readn(V->blkDevFd, V->fat, fatsz)) != fatsz) {
		fprintf(stderr,"readn() error in initialize fat. size: %d\n", fatsz);
		return -1;
	}

/*
	fprintf(stderr,"\n\n\n\n\n");
	for(i=0; i< 200; i++) {
		fprintf(stderr,"%d: %X ",i,(BYTE) V->fat[i]);
//		if ((i %2) == 0) fprintf(stderr, " ");
	}
	fprintf(stderr,"\n\n\n\n\n");
	getc(stdin);
*/

	/* we have now the whole fat in V->fat. We need to write it down in finalize in all the fat copies in the volume	*/
	return 0;
}

static int libfat_scan_for_free(Volume_t *V) {
    int i,res;  // int is ok, because this routine is only for fat12/16
    int count=0;
    DWORD val;
    for (i=2; i<=(V->DataClusters + 1); i++) {
        if ((res=fat_read_entry(V,i,0,&val)) != 0) { fprintf(stderr,"scan_for_free error\n"); return -1; }
        if ( fat_isfree(V,val) ) count++;
    }
    return count;
}

static int libfat_initialize_freelist(Volume_t *V) {
    int res;

    V->fstfclus=0;
    V->fclusz=0;
    if (V->FatType == FAT32) {
        V->freecnt = EFD(V->Fsi.FSI_Free_Count);
        V->nextfree = EFD(V->Fsi.FSI_Nxt_Free);
        if ( !(FAT32_LEGALCLUS(V->nextfree)) || (V->nextfree > (V->DataClusters + 1) ) ) {  //to fix 1st condition
            fprintf(stderr,"invalid next free field: %u\n",V->nextfree); return -1; }
        if ((res = fat_populate_freelist(V)) <= 0) { perror("populate freelist error"); return -1; }
    } else {    // FAT12 || FAT16
		if ((res = libfat_initialize_fat(V)) < 0 ) { fprintf(stderr,"initialize fat error\n"); return -1; }
        /* we need to scan the fat to find the free cluster count */
        V->freecnt = libfat_scan_for_free(V);
        if (V->freecnt < 0) { fprintf(stderr,"initialize freelist error\n"); return -1; }
        V->nextfree = 2;
    }
    return 0;
}

/*	Initialise	a Volume_t structure with the data  found in the BootSector	*/
/*	NB: the pathname must refer to a valid *FAT32* filesystem and must be 	*/
/*  canonical name															*/
/*	FAT12/16 NOT SUPPORTED right now										*/
int	fat_partition_init(Volume_t *V, char *pathname, int flags) {	// todo: add uid and gid;

    DWORD RootDirSectors = 0;	// Only for FAT12/16
    DWORD FATSz;
    DWORD FirstDataSector;
    DWORD TotSec;
    DWORD CountofClusters;
    DWORD DataSec;
	int fd,res;

/*  Variables we need to read from Boot Sector
    unsigned short int  BPB_RootEntCnt;
    unsigned short int  BPB_BytsPerSec;
    unsigned short int  BPB_FATSz16;
    unsigned int        BPB_FATSz32;
    unsigned short int  BPB_TotSec16;
    unsigned int        BPB_TotSec32;
    unsigned short int  BPB_ResvdSecCnt;
    unsigned char       BPB_NumFATs;
    unsigned short int  BPB_SecPerClus;
  ------------------------------------------  */
    V->uid = 0; //=uid;
    V->gid = 0; //=gid;
    V->fat = NULL;

	if (sizeof(off64_t) < 8) {
		perror("off64_t not supported properly by this system. the driver wont work properly");
		return -1;
	}

	if (sizeof(DWORD) != 4) {
		perror("DWORD not supported properly by this system. the driver wont work properly");
		return -1;
	}

	if (sizeof(long long int) < 8) {
		perror("long long int not supported properly by this system. the driver wont work properly");
		return -1;
	}

#ifdef FATWRITE
	if (flags & FAT_WRITE_ACCESS_FLAG) {
		if ( (fd = open(pathname, O_RDWR)) == -1 )
		perror("open() (RDWR) error");
	} else {
		if ( (fd = open(pathname, O_RDONLY)) == -1 )
			perror("open() (RDONLY) error");
	}
#else
	if ( (fd = open(pathname, O_RDONLY)) == -1 )
		perror("open() (RDONLY) error");
#endif



	V->blkDevFd = fd;

	if ( (res = readn(fd, &(V->Bpb), sizeof(Bpb_t))) != sizeof(Bpb_t))		//we read directly from da disk (NB FAT32 BPB)
		perror("BPB readn() error");



/* 	--------------------------------------------------------------------		*/

    if ( V->Bpb.BPB_FATSz16 != 0) { /* 0 is endianess independent */
        FATSz = EFW(V->Bpb.BPB_FATSz16);
    } else {
        FATSz = EFD(V->Bpb.BPB_FATSz32);
    }

    if ( V->Bpb.BPB_TotSec16 != 0) {
        TotSec = EFW(V->Bpb.BPB_TotSec16);
    } else {
        TotSec = EFD(V->Bpb.BPB_TotSec32);
    }

	RootDirSectors = (EFW(V->Bpb.BPB_RootEntCnt) * 32) / EFW(V->Bpb.BPB_BytsPerSec) ;

    // this calculation sucks. it doesnt keep in account first rootdircluster can be different from 2
	FirstDataSector = EFW(V->Bpb.BPB_ResvdSecCnt) + (V->Bpb.BPB_NumFATs * FATSz) + RootDirSectors;
	DataSec = TotSec - FirstDataSector;
	CountofClusters = (DWORD)  (DataSec / V->Bpb.BPB_SecPerClus);
    V->DataClusters = CountofClusters;          /* The total number of valid data clusters */

    /* determine fat type and eventually fetch fsinfo sector    */
    res = libfat_determine_fattype(V);

    V->FirstDataSector = FirstDataSector;       /* The first sector of the data region     */
    V->FirstRootCluster = EFD(V->Bpb.BPB_RootClus);     /* The first cluster of FAT32 root, usually 2   */

	V->bps64 =	EFW(V->Bpb.BPB_BytsPerSec);
	V->spc64 =	V->Bpb.BPB_SecPerClus;
	V->bpc64 =	V->bps64 * V->spc64;
	V->fds64 =	FirstDataSector;
	V->fdb64 =	V->fds64 * V->bps64;

	V->bps = V->bps64;
	V->spc = V->spc64;
	V->bpc = V->bpc64;

	/* position and size of root dir in fat12/16 */
	V->rootdir16off = (EFW(V->Bpb.BPB_ResvdSecCnt) + (V->Bpb.BPB_NumFATs * FATSz)) * V->bps64;
	V->rootdir16sz = RootDirSectors * V->bps;

	if ( V->FatType == FAT32) {
		V->fatsz = EFD(V->Bpb.BPB_FATSz32) * V->bps;
	} else V->fatsz = EFW(V->Bpb.BPB_FATSz16) * V->bps;

	V->rsvdbytecnt = EFW(V->Bpb.BPB_ResvdSecCnt) * V->bps;

	V->numfats =  V->Bpb.BPB_NumFATs;

/* setting mode for the volume -- TODO: get parameter for the mode and use those	*/
	V->mode =0;
	V->mode |= (S_IRWXU | S_IRWXG);

    libfat_initialize_freelist(V);
    memset(V->zerobuf, 0, ZERO_BFSZ);       //it is our buffer for write0 :)

#ifdef LIBFAT_USE_MUTEX
	if (pthread_mutex_init(&(V->fat_mutex), NULL) != 0) {
		perror("pthread_mutex_init() error in partition_init():");
		return -1;
	}
#endif

	printVolumeData(V);

#ifdef FATWRITE
	if (flags & FAT_WRITE_ACCESS_FLAG)
		fat_mark_dirty(V);
#endif
	return 0;
}

/* sync the fats into the volume	*/
int fat_fat_sync(Volume_t *V) {
    int fsioff;
    int res;
    if (V->FatType == FAT32) {
        V->Fsi.FSI_Free_Count = EFD(V->freecnt);
        V->Fsi.FSI_Nxt_Free = EFD(V->nextfree);

    // lets write down FSInfo Sector

        fsioff = (int) ((int) EFW(V->Bpb.BPB_FSInfo) * V->bps);
        res = lseek(V->blkDevFd,  fsioff , SEEK_SET);
        if (res != fsioff ) {
           perror("lseek() error in partition finalize");
          return -1;
        }
        res = writen(V->blkDevFd, (char *)  &(V->Fsi), sizeof(FSInfo_t));
        if (res < 0) {
            fprintf(stderr,"readn() error, line:%d\n",__LINE__);
            return -1;
        }
    } else if ((V->FatType == FAT12) || (V->FatType == FAT16)) {
		int i,size;
		off_t off;

		if (V->FatType == FAT16) {
			size = (V->DataClusters + 2) * sizeof(WORD);
		} else {
			size = ((V->DataClusters + 2) / 2) * 3;
			if (((V->DataClusters + 2) % 2) != 0) size += 2;
		}

		/* lets write down the whole fat in all copies */
		for (i=0; i < V->numfats; i++) {
			off=V->rsvdbytecnt + ( i * V->fatsz);

			if ( (res = lseek64(V->blkDevFd, off, SEEK_SET)) < 0) {
				fprintf(stderr,"lseek() error in partition finalize(), off:%d\n",(int) off);
        		return res;
    		}
   		 	/*  write()      */

		    if ( (res = writen(V->blkDevFd, V->fat, size)) != size) {
        		fprintf(stderr,"writen() error in partition finalize. size: %d\n", size);
        		return -1;
    		}
		}
	}

	return 0;
}

/* Finalize the volume writing down new count of free clusters and new hint in FSinfo	*/
int fat_partition_finalize(Volume_t *V) {
#ifdef FATWRITE
    int res;

	if ((res = fat_fat_sync(V)) != 0) return -1;

    // Lets mark volume as clean
//    if (flags & FAT_WRITE_ACCESS_FLAG)	// TO FIX THIS
    	fat_mark_clean(V);
#endif

#ifdef LIBFAT_USE_MUTEX
    if (pthread_mutex_destroy(&(V->fat_mutex)) != 0) { perror("pthread_mutex_destroy() error in partition_finalize():"); }
#endif

	if (V->fat != NULL) free(V->fat);

    // Lets close the fd
    close(V->blkDevFd);
    return 0;
}


/*	populate free cluster array	*/
int fat_populate_freelist(Volume_t *V) {
	int i=POPULATE_FREELIST_BUFSZ;
	DWORD buf[POPULATE_FREELIST_BUFSZ];
	off64_t freeoff;
	off64_t lastoff;
	int count=POPULATE_FREELIST_BUFSZ;						// value to compute amount of data to read.
	int res;
	int flag = 0;
	off64_t seekres;
	DWORD freeclus = V->freecnt;
	DWORD lastcluster = V->DataClusters + 1;	//see specs in fatgen103
	DWORD nextfree = V->nextfree;	// value to put into our free cluster array.

	if (freeclus <= 0) {
		fprintf(stderr,"No free clusters left\n");
		return -1;
	}
	if (nextfree > lastcluster) {		// there are free clusters but not at the end of the volume.
		V->nextfree = nextfree = 3;
	}

	freeoff = (off64_t) V->rsvdbytecnt + ((off64_t) V->nextfree * (off64_t) sizeof(DWORD));
	lastoff = (off64_t) V->rsvdbytecnt + ((off64_t) (lastcluster) * (off64_t) sizeof(DWORD)); // this is a legal cluster offset.

//	if ( ((off64_t) V->freecnt ) < (lastoff -freeoff + (off64_t) 1) ) {	//little problem: there should be more free cluster than what is written in FSInfo Sec.
//	}

	/* ok, there is at least 1 cluster we can check	*/
	V->fclusz = 0;
	V->fstfclus = 0;

	while (freeclus > 0) {
		if (freeoff > lastoff) {
			if (flag != 1) {
				flag = 1;
				nextfree = 2;
				freeoff = (off64_t) V->rsvdbytecnt + ((off64_t) 2 * (off64_t) sizeof(DWORD));
			} else { // error: we already had a whole scan of the fat by already setting freeoff to 2
//				fprintf(stderr,"populate_freelist() error: there are no freeclusters left even if system think so\n");
				return 	V->fclusz;
//				return -1;
			}
		}

		if (i >= count) {	// we need to fetch another piece of fat.
			count = (int) MIN(((off64_t) (POPULATE_FREELIST_BUFSZ)), (((lastoff - freeoff) / (off64_t) sizeof(DWORD)) + (off64_t) 1)); // seems ok.

			seekres = lseek64(V->blkDevFd,(off64_t)  freeoff , SEEK_SET);
			if (seekres != freeoff ) {
				perror("lseek() error in populate_freelist");
				return -1;
			}

			res = readn(V->blkDevFd, buf, (size_t) (count * sizeof(DWORD)));
			if (res < 0) {
				fprintf(stderr,"readn() error, line:%d\n",__LINE__);
				return -1;
			}
			i=0;
		}

		if (V->fclusz < FCLUS_BUFSZ) {
			if (fat_isfree(V,buf[i])) {
				V->freeclus[V->fclusz] =nextfree;
				V->fclusz++;
				freeclus --;
			}
		} else {	// we filled the whole freeclus buffer. let's exit.
			return (int) V->fclusz;
		}
		nextfree++;	// we dont update here V->nextfree.
		i++;
		freeoff += (off64_t) sizeof(DWORD);
	}

	// no free clusters left. let's return what we fetched.
	return V->fclusz;
}

/* provides a free cluster to the caller, updating nextfree and count of free cluster. 	*/
/* once provided, that cluster is considered used.										*/
/* The routine takes care to update fstfclus into V, and if needed to repopulate the free cluster array */
/* PLEASE NOTE: this routine does not recover free clusters and does not work if no 	*/
/* nextfree hint is provided. TOFIX														*/

static DWORD	fat32_getFreeCluster(Volume_t *V) {
	int res;
	DWORD clus;

	while (V->fstfclus >= V->fclusz) {
		res = fat_populate_freelist(V);
		if (res <= 0) {
			fprintf(stderr,"populate freelist error: end of space on the volume\n");
			return (DWORD) 0;
		}
	}

	clus = V->freeclus[V->fstfclus];
	V->freecnt--;
	V->fstfclus++;
	V->nextfree = MAX(clus,V->nextfree);
	if (clus > (V->DataClusters + 1)) {
		fprintf(stderr,"getFreeCluster() error. clus num : %u, max clus: %u\n", clus, (V->DataClusters + 1));
		return 0;
	}
//	fprintf(stderr,"gfc: cluster = %u\n", clus);
	return clus;
}

static DWORD 	fat16_getFreeCluster(Volume_t *V) {
	DWORD c, val;
	int res;
	if (V->freecnt <= 0) { fprintf(stderr,"getFreeCluster: end of free clusters in the volume\n"); return 0; }

	do {
		val = V->nextfree;
		if ((res = fat_read_entry(V, V->nextfree++, 0, &c)) < 0) { fprintf(stderr,"getFreeCluster16 error\n"); return 0; }
		if ( V->nextfree > (V->DataClusters + 1))  V->nextfree = 2;
		if (fat_isfree(V,c)) { V->freecnt--; return val; }
	} while (V->freecnt > 0);

	fprintf(stderr,"getFreeCluster: end of free clusters in the volume\n");
	return 0;
}

DWORD	fat_getFreeCluster(Volume_t *V) {
	DWORD val;
    if (V->FatType == FAT32) {
        val= fat32_getFreeCluster(V);
    } else {
		val= fat16_getFreeCluster(V);
    }

	fprintf(stderr,"- - fat_getFreeCluster: clus: %u; freecnt: %u\n",val,V->freecnt);
	return val;
}

/* Calculate the 8-bit checksum for the long file name from its */
/* corresponding short name.                                    */
/* 					                        */

BYTE lfn_checksum(BYTE *name) {
  int Sum = 0, i;
  for (i = 0; i < 11; i++) {
    Sum = (((Sum & 1) << 7) | ((Sum & 0xFE) >> 1)) + name[i];
  }
  return Sum;
}

/* Given a 32byte dirent entry, determines what it is . If lfn entry (and
   if the last one) or sfn entry or free entry				*/

int analyze_dirent(LfnEntry_t *D) {
    BYTE c;

    /*	Empty entry	*/
    if ( DIRENT_ISLAST(D->LDIR_Ord)) return LIBFAT_DIRENT_LASTFREE;
    if ( DIRENT_ISFREE(D->LDIR_Ord)) return LIBFAT_DIRENT_FREE;

	c = D->LDIR_Attr;

#if 0
	/* these are bits, more flags can be set at the same time */
    if ( (c != ATTR_READ_ONLY) && (c != ATTR_HIDDEN) && (c != ATTR_SYSTEM) && (c != ATTR_VOLUME_ID) &&
			    (c != ATTR_DIRECTORY)  && (c != ATTR_ARCHIVE) && (c != ATTR_LONG_NAME) ) {
		fprintf(stderr,"not a valid LDIR_Attr: %u",c);
		return -1;
	}
#endif

    if ( (D->LDIR_Attr == ATTR_LONG_NAME) ) {
		if (LFN_ISLAST(D->LDIR_Ord)) { 	//Lfn Entry
//			fprintf(stderr,"LFN LAsT\n");
			return	LIBFAT_DIRENT_LFN_LAST;
		} else {
//			fprintf(stderr,"LFN\n");
			return	LIBFAT_DIRENT_LFN;
		}
	} else {
//		fprintf(stderr,"SFN\n");
		return	LIBFAT_DIRENT_SFN;	//Sfn entry
    }
}

/* Check the cluster boundaries. if we reached the end of the cluster, the
function goes and get next cluster in the clusterchain. if we reached
the end of the file, it returns -1					*/
//TODO: range check that we are not over last cluster of the volume.

int  check_cluster_bound(Volume_t *V, DWORD *Cluster, DWORD *Offset) {
    WORD ClusterSz;

	int res;
    ClusterSz = V->bpc ;

	if (*Offset == 0) {	// we are in the beginning
		return 0;
	} else if (*Cluster == 1) {
		if ( ( *Offset % V->rootdir16sz ) == 0 ) {
			fprintf(stderr,"No space left on rootdirectory\n");
			return -1;
		} else return 0;
	} else  if ( ( *Offset % ClusterSz ) == 0) {	// we need to go in the next cluster

		DWORD value;

		if ((res = fat_read_entry(V, *Cluster, 0, &value)) != 0) {
			perror("check_cluster_bound() error");
		    return -1;
		}
//		printf("Cluster:%d\nOffset:%d\n,res: %u\n", *Cluster, *Offset, value);
		if (fat_iseoc(V,value)) {
//		    printf("Cluster:%d\nOffset:%d\n", *Cluster, *Offset);
			*Cluster = fat_eocvalue(V);
			return -1;
		} else {
		    *Cluster =  value;
		    *Offset = 0;
		    return 0;
		}
    } else {		// we havent reached the end of the cluster yet
		return 0;
    }
}


/* Fetches a 32byte dirent in a LfnEntry_t structure			    */
/* from the specified volume at the specified cluster and offset            */
/* returns 0 if successful, or -1 if error				    */
/* it moves forward the offset				                    */
/* Called by fat_readdir and find.                                          */
int fetch_entry(Volume_t *V, DWORD *Cluster, DWORD *Offset, LfnEntry_t *D) {
    int res;
	off64_t off;

	off = byte_offset(V, *Cluster, *Offset);

    if ( (res = lseek64(V->blkDevFd, off, SEEK_SET)) < 0 ) {
		perror("lseek() error in fetch_lfn():");
		return -1;
    }

    /*	Reading the dirent	*/
    if ( (res = readn(V->blkDevFd, D, 32)) != 32) {
		fprintf(stderr,"readn() error in fetch_entry() at %d",__LINE__);
		return -1;
    }

    /*	pushing forward the offset	*/
    *Offset += 32;
    return 0;
}

/* Fetches a whole directory entry chain for a file, to the specified Buffer*/
/* Returns the number of directory entries used, 0 if nothing was read, or  */
/* or -1 if an error occurred. Please note that 1 means only SFN entry is   */
/* present. It ensures also that the directory entries chain is correct     */
/* but not that the single entries are correct. (it make sure only that we  */
/* have LFNLAST,LFN,....,LFN,SFN or SFN					    */

int fetch_next_direntry(Volume_t *V, DirEnt_t *D, DWORD *Cluster, DWORD *Offset) {

	DWORD lastclus;
    int lfnlast=0;  // flag againist malformed entries
    int i=0;	//index for Buffer[i]
    int res=LIBFAT_DIRENT_FREE;
	int count = 0;

	if ((fat_iseoc(V,*Cluster)) || (fat_isfree(V,*Cluster))) return -1;

    /*	Skipping free entries	*/
    while ( LIBFAT_DIRENT_ISFREE(res) ) {
	    /*	Checking cluster boundaries to see if we reached end of the cluster	*/
 	   if ( (res = check_cluster_bound(V, Cluster, Offset)) != 0) {
			fprintf(stderr,"fetch_next_direntry: nothing left to read\n");
			return -1;	// Nothing left to read
		}

		lastclus = *Cluster;
		D->direntoff = D->off1 = byte_offset(V, *Cluster, *Offset);
		D->clus = *Cluster;
		D->off  = *Offset;

		if ( fetch_entry(V, Cluster, Offset, &((D->entry)[i])) < 0 ) {
			return -1;	// error fetching direntry
		}
		if ( ( res = analyze_dirent(&((D->entry)[i])) ) < 0) {
			return -1;			// malformed 32byte block
		}
		if ( LIBFAT_DIRENT_ISLASTFREE(res) ) {			// Nothing else after this 32byte direntry
		    return 0;
		}
    }

    /*	fetching the LFN entry chain	*/
    while (LIBFAT_DIRENT_ISLFN(res)) {
		if (LIBFAT_DIRENT_ISLFNLAST(res)) {
		    if (lfnlast > 0) {
				return -1;		// We already encountered an LFNLAST entry without and no Sfn entry.
		    } else {
				lfnlast++;
		    }

	/*	LFNLAST must be the first entry in the lfn chain	*/
			if ( lfnlast !=	1 ) {
				return -1;	// malformed lfn chain
	    	}
		}
		i++;
		count++;

		/*	Checking cluster boundaries to see if we reached end of the cluster	*/
 	   	if ( (res = check_cluster_bound(V, Cluster, Offset)) != 0) {
			perror("fetch_next_direntry(): nothing left to read");
			return -1;	// Nothing left to read
		}

		if (lastclus != *Cluster) {
			D->off2 = byte_offset(V, *Cluster, *Offset);
			D->len1 = i;
		}

		D->direntoff = byte_offset(V, *Cluster, *Offset);
		if ( fetch_entry(V, Cluster, Offset, &((D->entry)[i])) < 0) {
			return -1;
		}
		if ( (res = analyze_dirent(&((D->entry)[i]))) < 0) {
			return -1;
		}
    }

    /* We are out of the 2 whiles. So we got an SFN. Or an error :) */
	if ( ! (LIBFAT_DIRENT_ISSFN(res)) ) {
		return -1;	// we got the error, whatever this is
    } else {
		i++;		//	Please note (note for me haha) that i increasings are correct.
		D->len = i;
		D->len2 = i - D->len1;

		/* Now we have to check beyond the sfn entry to see if anything else may be present or not. */

		/*	Checking cluster boundaries to see if we reached end of the cluster	*/
 	   	if ( (res = check_cluster_bound(V, Cluster, Offset)) != 0) {
			D->last = 1;
		} else {
			off64_t off;
			char buf;

			off = byte_offset(V, *Cluster, *Offset);

			if ( (res = lseek64(V->blkDevFd, off, SEEK_SET)) < 0 ) {
				perror("lseek() error in fetch_lfn():");
				return -1;
    		}

		    /*	Reading the byte after	*/
			if ( (res = readn(V->blkDevFd, &buf, 1)) != 1) {
				fprintf(stderr,"readn() error in fetch_next_direntry() at %d",__LINE__);
				return -1;
    		}

			if (buf != 0) {
				D->last = 0;
			} else {
				D->last = 1;
			}
		}
		return i;
    }
	return 0;
}

/*	Check if all the lfn entries in the chain are in the correct order		*/
int check_lfn_order(LfnEntry_t *Buffer, int bufsize) {
 BYTE andmask = 0x3F;	// 00111111, to and with the lastlfnentry, who is or'ed with 01000000
			//					or 0x40
 int i = 1;
 int limit = bufsize - 1;

 for (i=1; (limit - i) >=0; i++) {	// we check only lfn entries.
    if ( ( Buffer[(limit - i)].LDIR_Ord & andmask ) != i ) return -1;  // entries not in order
 }

 return 0;
}

/*	check if all the lfn entries are related to the sfn in the buffer		*/
int check_lfn_checksum(LfnEntry_t *Buffer, int bufsize) {
    int i;
    BYTE checksum;

    checksum = lfn_checksum(((DirEntry_t *) &Buffer[bufsize -1])->DIR_Name );

    for (i=(bufsize -2); i>=0; i--) {
		if ( Buffer[i].LDIR_Chksum != checksum ) return -1;
    }
    /*	Ok, all the lfn entries are related to the sfn entry in the buffer
							    according to the checksum	*/
    return 0;
}

/*	Return the i-th unicode character from the lfn entry. (i from 0 to 12)		*/
WORD fetch_lfn_char(LfnEntry_t *D, int n) {
    int i = (n % 13) ;	// only 13 char in the lfn entries

    if ( i <= 4  ) return D->LDIR_Name1[i];
    if ( i <= 10 ) return D->LDIR_Name2[i-5];
    return D->LDIR_Name3[i-11];
}

/*	Return the i-th unicode character from the lfn entry. (i from 0 to 12)		*/
static int set_lfn_char(LfnEntry_t *D, WORD c , int n) {
    int i = (n % 13) ;	// only 13 char in the lfn entries

	if (D==NULL) return -1;

    if ( i <= 4  )  {
		D->LDIR_Name1[i] = c;
	} else if ( i <= 10 ) {
		D->LDIR_Name2[i-5] = c;
	} else D->LDIR_Name3[i-11] = c;

	return 0;
}


/*	Return the length of a long file name *including the terminator* in WORDs		*/
int find_lfn_length( LfnEntry_t *D, int bufsize) {
    WORD buf;
    int size = (bufsize - 2) * 13;
    int i=0;

    if (bufsize <= 1) return -1; // only sfn present

    // it checks only the last lfn entry!

    for (i=0; i<13; i++) {
		buf = fetch_lfn_char(&(D[0]),i);
		if ( LFN_ISNULL(buf) ) return ( ( i + 1 ) + size);
    }

    /* i should be 13 now	*/
    return ((i + 1) + size);	//yes, we need space for the terminator as well
}


/*	Extract the long file name from a lfnentry chain to a buffer of WORDS (UTF-16)	*/
int extract_lfn_name( LfnEntry_t *Buffer, int bufsize, WORD *dest, int length) {
    int i;
	int entry;

    for (i=0; i < (length - 1); i++) {
		entry = (bufsize - 2) - (i / 13);	// bufsize -1 is sfn entry
		/*	if we got a terminator before the end --> Error		*/
		dest[i] = fetch_lfn_char(&(Buffer[entry]) ,i);
    }

    // we add the terminator by ourselves
    dest[length - 1] = LFN_NULL;
    return 0;
}

/*	Return the length of a short file name including the terminator in chars		*/
int find_sfn_length( DirEntry_t *D, int bufsize) { // to fix: strip trailing spaces from filename and extension.
    int i;
    int count=0;

    if (  D[bufsize - 1].DIR_Name[0] == 0x20 ) return -1; // illegal first char

    for (i=0; i <8; i++) if (D[bufsize - 1].DIR_Name[i] != 0x20 ) count++;

    // there must be no lead 0 in the sfn extension.

    count++;
    if ( D[bufsize - 1].DIR_Name[8] == 0x20 ) return count;

    for (i=8; i <11; i++) if ( D[bufsize - 1].DIR_Name[i] != 0x20 ) count++;

    return ++count;
}

/*	Extract the short file name from a sfn entry in a direntry block (in some kind of codepage)*/
/*	return name length. including null terminator									   */
int extract_sfn_name(DirEntry_t *D, int bufsize, char *name) {
    int i;
    int count = 0;

	if (  D[bufsize - 1].DIR_Name[0] == 0x20 ) return -1; // illegal first char

    for (i=0; i <8; i++) {
        if (  D[bufsize - 1].DIR_Name[i] != 0x20 ) {
			name[count] = D[bufsize - 1].DIR_Name[i];
			count++;
		}
    }

    // there must be no lead 0 in the sfn extension.

    if ( D[bufsize - 1].DIR_Name[8] == 0x20 ) {
		name[count] = 0;
		return count;
    }

    name[count] = '.';
	count ++;

    for (i=8; i <11; i++) {
        if ( D[bufsize - 1].DIR_Name[i] != 0x20 ) {
			name[count] = D[bufsize - 1].DIR_Name[i];
			count++;
		}
    }

    name[count] = 0;
//	fprintf(stderr, "-- - - -- - -extract_sfn: name extracted: %s\n", name);
    return count;
}


/*	Generates a dirent structure from fat directory entries.		*/
int fatentry_to_dirent(Volume_t *V, DirEnt_t *D, struct dirent *dirp) {
    int res;

	LfnEntry_t *Buffer;
	int bufsize;
    WORD utf16buf[261];
    char utf8buf[521];
    int namelen;

	Buffer= D->entry;
	bufsize = D->len;

    memset(dirp, 0, sizeof(struct dirent));	// bzeroing the fields.
    memset(utf8buf,0,521);

    if ( bufsize < 2 ) {	// we have only sfn
		namelen = find_sfn_length( (DirEntry_t *) Buffer, bufsize);
//		fprintf(stderr," fatentry to dirent: bufsize: %s\n\n", ((DirEntry_t *) Buffer)[0].DIR_Name  );
		if ( (namelen = res = extract_sfn_name( (DirEntry_t *) Buffer, bufsize, utf8buf)) <= 0) return res;
		memcpy(dirp->d_name, utf8buf, namelen);
    } else {			// lfn
		namelen = find_lfn_length( Buffer, bufsize);
		if ( (res = extract_lfn_name( Buffer, bufsize, utf16buf, namelen)) != 0) return res;
		if ( (res = utf16to8(utf16buf, utf8buf)) != 0) return res;
		memcpy(dirp->d_name, utf8buf, 255);	// copying at most 255 utf8 bytes. the last is already 0
    }

	dirp->d_ino = get_fstclus(V, (DirEntry_t *) &(Buffer[bufsize -1]) );

    if (ATTR_ISDIR(((DirEntry_t *) Buffer)[bufsize-1].DIR_Attr)) {	// the direntry refers to a directory
		dirp->d_type = DT_DIR;
    } else {	// the direntry refers to a file
		dirp->d_type = DT_REG;
    }
    dirp->d_reclen = sizeof(struct dirent);
    return 0;
}


/* find the direntry related to the given name in the given directory container specified by *cluster* and offset	*/
/* it overwrites cluster and offset fields with the values of the direntry related to name in the current directory */
/* container (the one before). On fail, it returns -1 and it doesnt overwrite anything. 0 on success		*/
int find_direntry(Volume_t *V, char *name, DWORD *Cluster, DWORD *Offset) {
    int res;
    DWORD bkclu, bkoff;
	DirEnt_t D;
    LfnEntry_t *buffer = D.entry;

	//fprintf(stderr,"find_direntry. filename: %s\n",name);

	for (;;) {
		bkclu = *Cluster;
		bkoff = *Offset;
		res = fetch_next_direntry(V, &D, Cluster, Offset);
		if (res <= 0)  return -1;	// this is the condition to exit : nothing left to fetch or something strange found by fetch_next_direntry.
		/*	checking if this is what we are lookin for		*/
		if (res == 1 ) {	// only sfn name is present.
		    int i;
		    char *entryname;
		    i = find_sfn_length( (DirEntry_t *)buffer, res);
		    entryname = alloca(i);
		    extract_sfn_name((DirEntry_t *)buffer, res, entryname);
		    res = utf8_stricmp(name, entryname);	// possible because Ascii are utf8
		    if (res == 0) {	// we won! good.
				*Cluster = bkclu;
				*Offset = bkoff;
				return 0;
		    }
		} else {		// we use lfn name.
		    int i;
		    WORD *entryname;
			char *utf8name;
		    i = find_lfn_length( buffer, res);
		    entryname = alloca(i * sizeof(WORD));	//is it Possible to use alloca()?
			utf8name = alloca(i * sizeof(WORD));
			memset(utf8name, 0, i * sizeof(WORD));
			memset(entryname, 0, i * sizeof(WORD));

		    extract_lfn_name(buffer, res, entryname, i);	// res = size of buffer
			utf16to8(entryname, utf8name);
			//fprintf(stderr,"found 1 lfn direntry. name:%s\n",utf8name);
		    res = utf8_stricmp(name, utf8name);
		    if (res == 0) {	// we won! good.
				*Cluster = bkclu;
				*Offset = bkoff;
				return 0;
		    }
		}
	}
    return -2;
}


/* It takes a path of directories (it must exist in the volume V) and the Cluster of		*/
/* the root dir. it return the cluster of the last dir in the path.				*/
/*	It returns 0 on success, -1 if the path does not exist.					*/
/*	Offset probably not needed								*/


int traverse_path(Volume_t *V, gchar **parts, guint parts_len, DWORD *Cluster) {
    DWORD Offset = 0;
    DWORD Clus;
//    char nextdir[255];
    int i,res;
	DirEnt_t D;
    LfnEntry_t *buffer = D.entry;

	if (V->FatType == FAT32) {
		Clus = EFD(V->Bpb.BPB_RootClus);
    } else {
		Clus = 1;
	}

    for (i=1; i < (int) (parts_len - 1); i++) {

		if ( (res = find_direntry(V, (char *) parts[i] , &Clus, &Offset)) != 0 ) { //looking for the directory
		    return -1;	//error: part of the path not found.
		} else { // parth of the path found: we have to fetch the related direntry
			DirEntry_t *sfnentry;

		    if ( ( res = fetch_next_direntry(V, &D, &Clus, &Offset)) <= 0) {
				return -1 ;	//fetching the dirent
			}

			sfnentry = (DirEntry_t *) &(buffer[res - 1]);
			if ( ! (ATTR_ISDIR(sfnentry->DIR_Attr))) {
				return -1; //name found but not directory
			}
			Offset = 0;
			Clus = get_fstclus(V,sfnentry);
		}
	}
	*Cluster = Clus;
	return 0;
}



/* find the file (direntry) related to the given path. N.B. The path must exist exactly in the volume V.*/
/* it sets adequately Cluster and Offset. the caller has to fetch the direntry of the file of interest	*/
int find_file(Volume_t *V, const char *path, File_t *F, DWORD *Cluster, DWORD *Offset) {
    char *filename;
    int res;

    /*	splitting the path	*/
    gchar **parts = g_strsplit(path, "/", -1);
    guint parts_len = g_strv_length(parts);

    filename=(char *) parts[parts_len - 1];

    /*	Initialising Cluster and Offset with root directory values. Passed by the caller	*/
	/*	not needed! done in traverse_path()													*/

    /*  walking through the directory tree until we find the parent directory of our file/directory	*/
    /*	FIrst DIR = Rootdir				*/
    /*  We take one by one directories in the path, and look for them in the current dir. If it		*/
    /*	exists, we put DIR=that dir. If not throws an exception						*/

	if ( (res=traverse_path(V, parts, parts_len, Cluster)) != 0) {
		g_strfreev(parts);
		return -1;
	}	//error

	if (F!=NULL) {
		F->ParentFstClus = *Cluster;
		F->ParentOffset = 0;	// not used atm.
	}
	*Offset = 0;

	res = find_direntry(V, filename , Cluster, Offset); //looking for the directory
	g_strfreev(parts);

	if  (res != 0 ) {
		return -1;  //error: part of the path not found.
	} else { // file found. Cluster and offset are related to its direntry.
		F->DirEntryClus = *Cluster;
		F->DirEntryOffset = *Offset;
		return 0;
	}
}

/* Read into the buffer data belonging to the file in which is the cluster Cluster. This is low level function.	*/
/* it returns number of bytes read or -1 on error. It does not make any change to Cluster and Offset	*/
int fat_read_data(Volume_t *V, DWORD *Cluster, DWORD *Offset, char *buf, size_t count ) {

	off64_t off = 0;
	int datasize = count;
	int dataread = 0;
	int clustersz= EFW(V->Bpb.BPB_BytsPerSec) * V->Bpb.BPB_SecPerClus;
	int byteleftperclus = clustersz - *Offset;
	int res;
	off64_t seekres;
	int i=0;

//	fprintf(stderr,"off: %u, bytleft %d, count: %d\n",*Offset, byteleftperclus,count);

	if (*Offset > clustersz) {
		fprintf(stderr,"Offset too big\n");
		return -1;
	} else if (*Offset == clustersz) {
		DWORD c =  *Cluster;
		*Offset = 0;

		fat_read_entry(V, c, 0, Cluster); // if this is the last cluster in the chain, the function will allocate a new cluster.

		if (fat_isfree(V,*Cluster)) { // must be eoc or  a valid cluster number
			fprintf(stderr,"fat_write_data wrote on an unlinked cluster\n");
			return -1;
		/* workaround to avoid to return an invalid *Cluster	*/
		// lets check if wehave reached EOC. in this case this function allocate a free cluster to the file even if it does not need it.
		} else if (fat_iseoc(V,*Cluster)) {	//we reached EOC. so we have to get a free cluster and link it to the file
			fprintf(stderr,"read_data error: EOC reached.\n");
			return -1;
		} else {
			// Cluster must be a valid cluster number set up by fat_read_entry.
			fprintf(stderr,"offset >= clustersz, but next cluster exist.\n");
		}

		*Offset = 0;
	}

	while (count > 0) {
		int numbytes;
		int newoffset;

		numbytes = MIN(byteleftperclus, count);

		newoffset = *Offset + numbytes;
		off = byte_offset(V, *Cluster, *Offset);

		fprintf(stderr,"Cluster: %u, Offset: %u, off: %lld, numbyts:%d\n",*Cluster, *Offset, off, numbytes);
		seekres = lseek64(V->blkDevFd,(off64_t)  off , SEEK_SET);
		if (seekres != off ) {
			perror("lseek() error in read_data");
			return -1;
		}

		res = readn(V->blkDevFd, &(buf[dataread]), (size_t) numbytes);
		if (res < 0) {
			fprintf(stderr,"readn() error, line:%d\n",__LINE__);
			return -1;
		}

		dataread += res;
		count -= res;
		i++;

		// NB: readn implies it has to read numbytes bytes or fail, so
		// if count is now > 0, then we automatically have to jump to next cluster.

		//update cluster and offset -- if we reach end of clusterchain, count must have been too large.
		if (count > 0) {	// lets fetch next cluster.
			DWORD c =  *Cluster;
			fat_read_entry(V, c, 0, Cluster);
			// lets check if wehave reached the end of clusterchain
			if (fat_iseoc(V,*Cluster)) {	//we reached EOC so count is too big. Caller have to check *Cluster by himself
				fprintf(stderr,"read_data() error: EOC reached\n");
				return(datasize -count);
			}
			*Offset = 0;
			//lets reset byteleftperclus
			byteleftperclus = clustersz;
		} else {	// count <=0. We are done. lets set *Offset properly
			*Offset = newoffset;
			if ( *Offset >= clustersz ) {	//it cant actually be greater, just equal
				DWORD c =  *Cluster;
				fat_read_entry(V, c, 0, Cluster); // if this is the last cluster in the chain, the function will return EOC/0 pair for clus/off
				*Offset = 0;
			}
			return datasize;
		}
	}
	return -2;	// we shouldnt reach here
}

/* Write from the buffer data to the file who own the cluster Cluster, starting at 						*/
/*  the offset Offset in the cluster Cluster. 								This is low level function. */
/* it returns number of bytes written or -1 on error. It does not make any change to Cluster and Offset */
/* NB: F can be NULL if we are writing on a directory file												*/
/* Please NOTE: the function does not make any check on Cluster.										 */
int fat_write_data(Volume_t *V, File_t *F, DWORD *Cluster, DWORD *Offset, char *buf, int count ) {
#ifdef FATWRITE
	int cnt = count;
	off64_t off = 0;
	int datasize = count;
	int datawritten = 0;
	int clustersz= V->bpc;
	int byteleftperclus;
	int res;
	off64_t seekres;
	int i=0;
	DWORD orig_filesz = 0;

	if (F != NULL) {
		orig_filesz = EFD(F->DirEntry->DIR_FileSize);
	}



	if (*Offset > clustersz) {
		fprintf(stderr,"Offset too big\n");
		return -1;
	} else if (*Offset == clustersz) {
		DWORD c =  *Cluster;
		*Offset = 0;

		fat_read_entry(V, c, 0, Cluster); // if this is the last cluster in the chain, the function will allocate a new cluster.

		if (fat_isfree(V,*Cluster)) { // must be eoc or  a valid cluster number
			fprintf(stderr,"fat_write_data wrote on an unlinked cluster\n");
			return -1;
		/* workaround to avoid to return an invalid *Cluster	*/
		// lets check if wehave reached EOC. in this case this function allocate a free cluster to the file even if it does not need it.
		} else if (fat_iseoc(V,*Cluster)) {	//we reached EOC. so we have to get a free cluster and link it to the file
			DWORD newclus;
			int r;

			newclus = fat_getFreeCluster(V);
			if (newclus == (DWORD) 0) {	// no newclusters available
				fprintf(stderr,"getFreeCluster() error. line %d\n",__LINE__); return -1; }

			/* lets write newclus into *Cluster in the fat.	*/	//tofix: writen_entry instead of write_entry???
			if ((r = fat_writen_entry(V, c, newclus)) != 0) return -1;

			/* lets write EOC into newclus in the fat.		*/
			if ((r = fat_writen_entry(V, newclus,  fat_eocvalue(V))) != 0) return -1;

			/* lets set *Cluster to newclus					*/
			*Cluster = newclus;
		} else {
			// Cluster must be a valid cluster number set up by fat_read_entry.
			fprintf(stderr,"offset >= clustersz, but next cluster exist.\n");
		}
		*Offset = 0;
	}

	byteleftperclus = clustersz - *Offset;
	fprintf(stderr,"off: %u, bytleft %d, cnt: %d\n",*Offset, byteleftperclus,cnt);

	while (cnt > 0) {
		int numbytes;
		int newoffset;

		numbytes = MIN(byteleftperclus, cnt);

		newoffset = *Offset + numbytes;
		off = byte_offset(V, *Cluster, *Offset);

		fprintf(stderr,"Cluster: %u, Offset: %u, off: %lld, numbyts:%d, i:%d\n",*Cluster, *Offset, off, numbytes,i);
		seekres = lseek64(V->blkDevFd,(off64_t)  off , SEEK_SET);
		if (seekres != off ) {
			fprintf(stderr,"lseek() error in read_data\n");
			return -1;
		}

		res = writen(V->blkDevFd, &(buf[datawritten]), (size_t) numbytes);
		if (res != numbytes) { fprintf(stderr,"writen() error in write data\n"); return -1;	}

		if (F != NULL) F->CurAbsOff += res;
		datawritten += res;
		cnt -= res;
		i++;

		// NB: readn implies it has to read numbytes bytes or fail, so
		// if cnt is now > 0, then we automatically have to jump to next cluster.

		//update cluster and offset -- if we reach end of clusterchain, cnt must have been too large.
//		fprintf(stderr,"value of cnt: %d\n",cnt);
		if (cnt > 0) {	// lets fetch next cluster.
			DWORD c =  *Cluster;

			fat_read_entry(V, c, 0, Cluster);
			fprintf(stderr,"reading value of cluster %u:  %u\n",c, *Cluster);
			if (fat_isfree(V,*Cluster)) {
				fprintf(stderr,"fat_write_data wrote on an unlinked cluster\n");
				return -1;
			}
			// lets check if wehave reached the end of clusterchain
			if (fat_iseoc(V,*Cluster)) {	//we reached EOC. so we have to get a free cluster and link it to the file
				DWORD newclus;
				int r;

				newclus = fat_getFreeCluster(V);
				if (newclus == 0) {
					if ((F != NULL) && (EFD(F->DirEntry->DIR_FileSize) < F->CurAbsOff)) F->DirEntry->DIR_FileSize = EFD(F->CurAbsOff);
					fprintf(stderr,"getFreeCluster() error. line:%d\n",__LINE__); return -1;
				}

				/* lets write newclus into *Cluster in the fat.	*/	//tofix: writen_entry instead of write_entry???
				if ((r = fat_writen_entry(V, c, newclus)) != 0) return -1;

				/* lets write EOC into newclus in the fat.		*/
				if ((r = fat_writen_entry(V, newclus,  fat_eocvalue(V))) != 0) return -1;

				/* lets set *Cluster to newclus					*/
				*Cluster = newclus;
			} else {
				// Cluster must be a valid cluster number set up by fat_read_entry.
			}

			*Offset = 0;
			//lets reset byteleftperclus
			byteleftperclus = clustersz;

		} else {	// cnt <=0. We are done. lets set *Offset properly
			DWORD c =  *Cluster;
			*Offset = newoffset;

			if ( *Offset >= clustersz ) {	//it cant actually be greater, just equal
				fat_read_entry(V, c, 0, Cluster); // if this is the last cluster in the chain, the function will allocate a new cluster.

				if (fat_isfree(V,*Cluster)) { // must be eoc or  a valid cluster number
					fprintf(stderr,"fat_write_data wrote on an unlinked cluster\n"); return -1; }

				/* workaround to avoid to return an invalid *Cluster	*/
				// lets check if wehave reached EOC. in this case this function allocate a free cluster to the file even if it does not need it.
				if (fat_iseoc(V,*Cluster)) {	//we reached EOC. so we have to get a free cluster and link it to the file
					// reset cluster value and just left offset value to clustersize
					*Cluster = c;
				} else {
					// Cluster must be a valid cluster number set up by fat_read_entry.
					fprintf(stderr,"offset >= clustersz, but next cluster exist.\n");
					*Offset = 0;
				}
			} else { //offset < clustersize
					fprintf(stderr,"offset !>= clustersz, so everything is fine and we dont have to allocate a new cluster\n");
			}
			if ((F != NULL) && (EFD(F->DirEntry->DIR_FileSize) < F->CurAbsOff)) F->DirEntry->DIR_FileSize = EFD(F->CurAbsOff);
			return datasize;
		}
	}
#endif /* #ifdef FATWRITE */
	return -2;	// we shouldnt reach here
}

static ssize_t fat_write0data(Volume_t *V, File_t *F, DWORD *Cluster, DWORD *Offset, size_t count) {
	int res;
	int retval = count;
	char *buf = V->zerobuf;

	if ((count <= 0)) { fprintf(stderr,"write0 error: count <= 0\n"); return -1; }

	while (count > 0) {
		int size;
		size = MIN(ZERO_BFSZ, count);
		count -= size;
		if ((res = 	fat_write_data(V, F, Cluster, Offset, buf, size )) != size) {
			fprintf(stderr,"write0data error. size: %d, res: %d\n",size, res); return -1; }
	}
	return retval;
}

/* 	Update the DirEntry_t of the file F into the volume V			*/
int fat_update_file(File_t *F) {
#ifdef FATWRITE
	int res;
	off64_t seekres;

	if (F == NULL) return 0;

	seekres = lseek64(F->V->blkDevFd,(off64_t)  F->D.direntoff , SEEK_SET);
	if (seekres != F->D.direntoff ) {
		perror("lseek() error in update file");
		return -1;
	}

	res = writen(F->V->blkDevFd, (char *) F->DirEntry, (size_t) sizeof(DirEntry_t));
	if (res < 0) {
		perror("writen() error in update file");
		return -1;
	}
#endif /* #ifdef FATWRITE */
	return 0;
}

/* set first cluster */
int set_fstclus(Volume_t *V, DirEntry_t *D, DWORD c) {
	if (D==NULL) return -1;
	c=EFD(c); // c is now little endian

	char *src = (char *) &c;
	char *dst = (char *) &(D->DIR_FstClusLO);

	dst[0] = src[0];
	dst[1] = src[1];

	if (V->FatType == FAT32) {
		dst = (char *) &(D->DIR_FstClusHI);
		dst[0] = src[2];
		dst[1] = src[3];
	}

	return 0;
}

/* get first cluster */
DWORD get_fstclus(Volume_t *V, DirEntry_t *D) {
	DWORD val = 0;

	char *dst = (char *) &val;
	char *src = (char *) &(D->DIR_FstClusLO);

	dst[0] = src[0];
	dst[1] = src[1];

	if (V->FatType == FAT32) {
		src = (char *) &(D->DIR_FstClusHI);
		dst[2] = src[0];
		dst[3] = src[1];
	}

	val=EFD(val);
	return val;
}

static int erase_dirent(DirEnt_t *D) {
	BYTE b;
	int i;

	if (D->last == 0) {
		b = FREEENT;
	} else {
		b = ENDOFDIR;
	}

	for (i=0; i < D->len; i++) D->entry[i].LDIR_Ord = b;
	return 0;
}

/* find a direntry by sfn name. require a buffer of 11 char containing the sfn name */
static int find_direntry_bysfn(Volume_t *V, BYTE *sfnname, DWORD *Cluster, DWORD *Offset) {
    int res;
    DWORD bkclu, bkoff;
	DirEnt_t D;
    LfnEntry_t *buffer = D.entry;

	for (;;) {
		bkclu = *Cluster;
		bkoff = *Offset;
		res = fetch_next_direntry(V, &D, Cluster, Offset);
		if (res <= 0)  return -1;	// this is the condition to exit : nothing left to fetch.
		/*	checking if this is what we are lookin for		*/

		res= utf8_strncmp((char *) sfnname, (char *) ((DirEntry_t *) &(buffer[res -1]))->DIR_Name, 11); // possible because Ascii are utf8

		if (res == 0) {	// we won! good.
			*Cluster = bkclu;
			*Offset = bkoff;
			return 0;
		}
	}
    return -2;
}

/* static routine to generate an lfn chain. utf8name must be null terminated */
static int generate_lfn_chain(char *utf8name, LfnEntry_t *buf, BYTE checksum) {
	int res;
	int len;
	int i;
	int size;
	WORD lfn_name[256];

	memset(lfn_name, 0, 256*sizeof(WORD));
	len = res = utf8_strlen(utf8name);
	fprintf(stderr,"filename: %s, len:%d\n", utf8name, len);
	if ((res <= 0) || (res >255)) {
		perror("generate lfn chain() error: illegal filename length");
		return -1;
	}

	size = ((len -1) /13) +1;	// it is correct

	res = utf8to16(utf8name, lfn_name);
	if (res < 0)  {
		perror("utf8to16() error: illegal filename length %d");
		return -1;
	}

	// lets memset the buffer to FFFF (padding for lf name
	memset(buf,0xFF, (21 * sizeof(LfnEntry_t)));

	// lets begin writing the name, properly setting checksum and other fields to 0;
	for(i=0; i <len; i++) {
		res = set_lfn_char((LfnEntry_t *) &(buf[(size - 1 - (i / 13))]), lfn_name[i] , i);
		if (res != 0) return -1;
	}
	if ( (len % 13) != 0 ) { //if we dont have to go next entry
		res = set_lfn_char((LfnEntry_t *) &(buf[0]), 0x0000 , (len % 13));
		if (res != 0) return -1;
	}

	for(i=1; i <=size; i++) {
		buf[size -i].LDIR_Ord = i;
		buf[size -i].LDIR_Attr = ATTR_LONG_NAME;
		buf[size -i].LDIR_Chksum = checksum;
		buf[size -i].LDIR_Type = 0;
		buf[size -i].LDIR_FstClusLO = 0;
	}

	buf[0].LDIR_Ord |= LFN_LASTENTRY;

	//lets return number of used entries
	return ( size );		// i suppose ((len -1) /13) +1 is wrong. im wrong
}

/* find n 32byte slots to store a file entry . it sets properly cluster and offset and returns absolute offset */
static off64_t fat_find_lfnslots(Volume_t *V, File_t *dir, DWORD *Cluster , DWORD *Offset, int n) {
    DWORD oldclus,clus,newclus;
	DWORD off=0;
	int res;
	int found =0;
	char *cluster;
	int rootdirflag = 0;

	if ((dir == NULL) || (dir->rootdir == 1)) rootdirflag =1;

	if ((rootdirflag != 1)){
		if (!ATTR_ISDIR(dir->DirEntry->DIR_Attr)) {
			perror("find lfnslots error: file is not a directory");
			return -1;
		}
		clus = get_fstclus(V, dir->DirEntry);
		dir->CurAbsOff = 0;
	} else {
		if (V->FatType == FAT32) {
			clus = 2;
		} else clus = 1;
	}
	if ( fat_iseoc(V,clus) || fat_isfree(V,clus) ) {	// this case should never happen since every directories cointain . and ..
		//we should fetch a new cluster here..			// when they are create so at least 1 cluster.
		return -1;
	}

	oldclus = newclus = clus;

  if ((V->FatType == FAT32) || (rootdirflag !=1)) {	// FAT32 or not rootdir
	// now time to allocate space to read clusters;
	cluster = alloca(V->bpc);

	while (found < n) {
		if ((off % V->bpc) == 0) { //fetch new cluster
			off = 0;
			if ( fat_iseoc(V,newclus) || fat_isfree(V,newclus) ) {
				//set properly clus and off according with found, then allocate new cluster, link it and update the file then return

				newclus = fat_getFreeCluster(V);
				if (newclus == 0) {
					return -1;
				}

				/* We must zero this new cluster */
				{
				 DWORD tmpclus, tmpoff;
				 tmpclus=newclus; tmpoff = 0;

				 if ((res = fat_write0data(V, NULL, &tmpclus, &tmpoff, (V->bpc -1))) != (V->bpc -1)) {
					fprintf(stderr,"fat_mkdir() error: write0data() failed\n"); return -1; }
				}

				if (found == 0) {
					*Cluster=newclus;
					*Offset=0;
					if (dir != NULL) {
						dir->CurClus=*Cluster;
						dir->CurOff=*Offset;
					}
				} else {
					*Cluster = clus;
					*Offset = V->bpc - (found * 32);
					if (dir != NULL) {
						dir->CurClus=*Cluster;
						dir->CurOff=*Offset;
						dir->CurAbsOff -= (found *32);
					}
				}

				// linking
				fat_writen_entry(V, newclus, fat_eocvalue(V));
				fat_writen_entry(V, clus, newclus);

				return byte_offset(V, *Cluster, *Offset);
			}
			oldclus = clus;
			clus = newclus;
			res = fat_read_data(V, &newclus, &off, cluster, V->bpc );	// it updates clus and set off to 0	because we read a whole clustersize
			if (res < 0) { fprintf(stderr,"find lfn slots error\n"); return -1; }
		}

		if (DIRENT_ISFREE((BYTE) cluster[off])) {
			found++;
		} else {
			found = 0;
		}
		off += 32;
		if (dir != NULL) dir->CurAbsOff += 32;
	}

	// out of the while: we found n adjacent free entries without having to allocate a new cluster. lets compute the cluster
	if (off < (n * 32)) { // sequence began in previous cluster
		*Cluster = oldclus;
		*Offset = V->bpc - ((n * 32) - off);
	} else {	// this cluster
		*Cluster = clus;
		*Offset = off - (n*32);
	}
  } else { // FAT12/16 root
  	off64_t res64;
	cluster = alloca(EFW(V->Bpb.BPB_RootEntCnt) * 32 ); // size of the root directory in the volume

	if ((res64 = lseek64(V->blkDevFd, V->rootdir16off ,SEEK_SET)) != V->rootdir16off) {
		fprintf(stderr,"find lfn slots error : lseek()\n"); return -1;}
	if ((res = readn(V->blkDevFd,cluster, (EFW(V->Bpb.BPB_RootEntCnt) * 32) )) != (EFW(V->Bpb.BPB_RootEntCnt) * 32)) {
		fprintf(stderr,"find lfn slots error : readn()\n"); return -1;}

 	while (found < n) {
		if (off >= (EFW(V->Bpb.BPB_RootEntCnt) * 32)) { fprintf(stderr,"find_lfn_slots: Not enough free space\n"); return -1; }

		if (DIRENT_ISFREE((BYTE) cluster[off])) {
			found++;
		} else {
			found = 0;
		}
		off += 32;
		if (dir != NULL) dir->CurAbsOff += 32;
	}

  	*Cluster = clus;
	*Offset = off - (n*32);
  }

  return byte_offset(V, *Cluster, *Offset);
}

/* create() function for	libfat											 */
/* dirflag specifies if the file is a directory								 */
/* if parent is NULL, then it is supposed to be rootdir					     */
/* reminder for mkdir: each dir has to cointain . and .. in the beginning!!! */
/* remnider 2: directories have filesize = 0!!!!!!!!!!!	O_O					 */
/* Another reminder: when we create a file or a directory, the only date that changes*/
/* is in the direntry in the parent. not in the . or the various .. of subdirectories*/

int fat_create(Volume_t *V, File_t *parent, char *filename , DirEntry_t *sfn, mode_t mode, int dirflag) {
	int res;
	int i=1;
	int slotnum;
	LfnEntry_t  entry[21];
	DWORD clus;
	DWORD off=0;
	DWORD bkclus;
	BYTE sfnname[12];	// 12 because sprintf put null terminator
	char lfnname[12];
	BYTE checksum;
	DirEntry_t *dirent;
	time_t tim;
	off64_t res64;
	int rootdirflag = 0;


	if ((parent == NULL) || (parent->rootdir == 1)) rootdirflag =1;

	res = utf8_stricmp(filename,"");
	res |= utf8_stricmp(filename,".");
	res |= utf8_stricmp(filename,"..");

	//lets check for . and ..
	if (res == 0) {
		fprintf(stderr,"fat_create(): cannot create \".\" or \"..\" or empty file name. filename: %s\n",filename);
		return -1;
	}

	/* we dont have to check for / because filename was tokenized with it. but we should check for \ */
	res = utf8_strchk(filename);	// please note here 1 means string ok, 0 string bad
	if (res == 0) {
		fprintf(stderr,"fat_create(): illegal file name\n");
		return -1;
	}

	//let's look for filename in parent.
	if (rootdirflag != 1) {
		clus = get_fstclus(V,parent->DirEntry);
	} else {
		if (V->FatType == FAT32) {
			clus = 2;
		} else clus = 1;
	}
	bkclus=clus;

	//fprintf(stderr,"cluster where we start lookin for the file: %u. offset: %lld\n",clus, byte_offset(V,clus,off));

	res = find_direntry(V, filename , &clus, &off); //looking for the directory
	if (res == 0) { // file already exist in the parent
		//todo : check for O_EXCL
		fprintf(stderr,"fat_create() error: file exist\n");
		return -1;
	}


	sprintf((char *) sfnname,"~%-10d",i);
	if (sfn != NULL) {
		memcpy(sfnname, sfn->DIR_Name, 11); // we use sfname from sfn
		extract_sfn_name(sfn, 1, lfnname);
	}

	/* lets generate a suitable sfn name that is not present in the parent.	*/
	do {
		clus =bkclus;
		off=0;
		res = find_direntry_bysfn(V, sfnname, &clus, &off);
		if (res == 0) {
			//lets change sfn name.
			sprintf((char *) sfnname,"~%-10d",++i);
		} else {
			clus =bkclus;
        	off=0;
				// let's take care that even a such lfn does not exist
			res = find_direntry(V, lfnname, &clus, &off);
			if (res == 0) {
				//lets change sfn name.
				sprintf((char *) sfnname,"~%-10d",++i);
				sprintf(lfnname,"~%d",i);
			}
		}
	} while( res == 0 );
		fprintf(stderr,"sfn after search: %s.\n",sfnname);


	/* let's generate lfnentry chain	*/
	checksum = lfn_checksum(sfnname);

	res=generate_lfn_chain(filename, entry, checksum);
	if ((res > 20) || (res <= 0)) {
		perror("generate_lfn_chain() error");
		return -1;
	}
	slotnum = res;

	dirent = (DirEntry_t *) &(entry[slotnum]); //because res is the size of only lfn entries in entry[]
	if (sfn == NULL) {
		dirent->DIR_Attr=0;
		if (dirflag == 1) {
			dirent->DIR_Attr = ATTR_DIRECTORY;
		} else {
			dirent->DIR_Attr = ATTR_ARCHIVE;
		}
		dirent->DIR_FstClusLO = dirent->DIR_FstClusHI = 0;
		dirent->DIR_FileSize = 0;
		dirent->DIR_NTRes = dirent->DIR_CrtTimeTenth = 0;
	} else {
		/* disregard dirflag and copy sfn into the chain	*/
		memcpy((char *) dirent ,(char *) sfn, sizeof(DirEntry_t));
	}
	memcpy(&(dirent->DIR_Name), sfnname, 11);

	/* setting creation and write time	*/
	tim=time(NULL);
	fat_fill_time(&(dirent->DIR_CrtDate), &(dirent->DIR_CrtTime), tim);
	fat_fill_time(&(dirent->DIR_WrtDate), &(dirent->DIR_WrtTime), tim);

	dirent->DIR_LstAccDate = dirent->DIR_CrtDate;

	/*lets find a place in the parent to store the chain  or ask for a new cluster	*/

	res64 = fat_find_lfnslots(V, parent, &clus , &off, res + 1); // lfn slots + sfn slot
	if (res64 <= 0) {
		fprintf(stderr,"findlfnslots: no slots found\n");
		return -1;
	} else { fprintf(stderr,"findlfnslots: found %d slots at clus:%u, off:%u\n",(res + 1),clus, off); }

	/*  now we can write down the dirent and update the wrttime in the parent. NULL as second param because we are writing a directory */
	if ((V->FatType == FAT32)  || (rootdirflag != 1)) {
		res = fat_write_data(V, NULL, &clus, &off, (char *) entry, ((slotnum + 1) * sizeof(DirEntry_t)));
		if (res != ((slotnum + 1) * sizeof(DirEntry_t))) {
			fprintf(stderr,"write error in fat_create(). res: %d,line: %d\n",res,__LINE__);
			return -1;
		}
	} else { //FAT12/16 root
		off64_t res64;
		res64 = byte_offset(V,clus, off);	// clus have been set to 1 by find_lfnslots to indicate fat12/16 root dir
		if ((res64 = lseek(V->blkDevFd, res64, SEEK_SET)) != byte_offset(V,clus, off)) {
			fprintf(stderr,"lseek error in fat_create(). res: %d,line: %d\n",res,__LINE__); return -1; }
		if ((res = writen(V->blkDevFd,(char *) entry,((slotnum + 1) * sizeof(DirEntry_t)))) != ((slotnum + 1) * sizeof(DirEntry_t))) {
			fprintf(stderr,"write error in fat_create(). res: %d,line: %d\n",res,__LINE__); return -1; }
	}
	if (rootdirflag != 1) {
		fat_fill_time(&(parent->DirEntry->DIR_WrtDate), &(parent->DirEntry->DIR_WrtTime), tim);
		parent->DirEntry->DIR_LstAccDate = parent->DirEntry->DIR_CrtDate;
		res = fat_update_file(parent);
		if (res != 0) return -1;
	}
	return 0;
}

/* mkdir() routine for libfat */
/* if parent is missing, consider it as root dir	*/
int fat_mkdir(Volume_t *V, File_t *parent, char *filename , DirEntry_t *sfn, mode_t mode) {
	int res;
	DWORD Cluster;
	DWORD Offset=0;
	DirEnt_t D;
	DirEntry_t fst2entry[2];
	DWORD newclus, bknewclus;
	DWORD parentfstclus;
	File_t F;
	time_t tim;


	res = fat_create(V, parent, filename , sfn, mode, 1);
	if (res != 0) {
		fprintf(stderr,"fat_mkdir(): fat_create() error\n");
		return -1;
	}

	if ((parent != NULL) && (parent->rootdir != 1)) {
		parentfstclus = Cluster = get_fstclus(V,parent->DirEntry);
	} else {
		if (V->FatType == FAT32) {
			Cluster = 2;
		} else Cluster = 1;
		parentfstclus = 0; // fsck.vfat complains if it's 2.. reeeally weird
	}

	res = find_direntry(V, filename, &Cluster, &Offset);
	if ( res != 0 ) { //looking for the directory
		fprintf(stderr,"fat_mkdir() error: directory not found in parent\n");
		return -1;	//error: part of the path not found.
	}

	res = fetch_next_direntry(V, &D, &Cluster, &Offset);
	if (res <= 0) {
		fprintf(stderr,"fat_mkdir() error: fetch_next_direntry failed\n");
		return -1 ;	//fetching the dirent
	}

	/* now we have to allocate first cluster to the directory and create . and .. */
	if (sfn == NULL) {
		bknewclus = newclus = fat_getFreeCluster(V);
	} else bknewclus = newclus = get_fstclus(V,sfn);
	if (newclus == 0) {
		fprintf(stderr,"fat_mkdir() error: getfreecluster failed\n");
		return -1;
	}

	/* lets write EOC into newclus in the fat.		*/
	if (sfn == NULL) {
		res = fat_writen_entry(V, newclus,  fat_eocvalue(V));
		if (res != 0) {
			fprintf(stderr,"fat_mkdir() error: fat_writen_entry failed\n");
			return -1;
		}
	}

	memset(fst2entry, 0, 2*sizeof(DirEntry_t));
	memcpy((char *) fst2entry, ".          ", 11);
	memcpy((char *) &(fst2entry[1]), "..         ", 11);
	fst2entry[0].DIR_Attr = fst2entry[1].DIR_Attr = ATTR_DIRECTORY;
	set_fstclus(V,&(fst2entry[0]), bknewclus);	// .
	set_fstclus(V,&(fst2entry[1]), parentfstclus);	// ..

	tim=time(NULL);

	fat_fill_time(&(fst2entry[0].DIR_CrtDate), &(fst2entry[0].DIR_CrtTime), tim);
	fat_fill_time(&(fst2entry[0].DIR_WrtDate), &(fst2entry[0].DIR_WrtTime), tim);
	fat_fill_time(&(fst2entry[1].DIR_CrtDate), &(fst2entry[1].DIR_CrtTime), tim);
	fat_fill_time(&(fst2entry[1].DIR_WrtDate), &(fst2entry[1].DIR_WrtTime), tim);
	fst2entry[0].DIR_LstAccDate = fst2entry[1].DIR_LstAccDate = fst2entry[0].DIR_CrtDate;

	Offset =0;
	/* NB: We have to zero the new cluster allocated BECAUSE otherwise find_file does NOT work properly */
	{
		DWORD tmpclus, tmpoff;
		tmpclus=newclus; tmpoff = 0;

		if ((res = fat_write0data(V, NULL, &tmpclus, &tmpoff, (V->bpc -1))) != (V->bpc -1)) {
			fprintf(stderr,"fat_mkdir() error: write0data() failed\n"); return -1; }
	}

	if ((res = fat_write_data(V, NULL, &newclus, &Offset, (char *)fst2entry, 2 * sizeof(DirEntry_t))) != (2* sizeof(DirEntry_t))) {
		fprintf(stderr,"fat_mkdir() error: write_data() failed\n"); return -1; }

	/* finally we should update the newely created dirent. but update_file() require a File_t and create at the moment does not provide it. so workaround */
	/* tofix */

	F.V = V;
	memcpy((char *) &(F.D), (char *) &D, sizeof(DirEnt_t));
	F.DirEntry = (DirEntry_t *) &(F.D.entry[F.D.len -1]);
	set_fstclus(V,F.DirEntry, bknewclus);
	fprintf(stderr,"newclus = %u,1stclus: %u, len = %d\n", newclus, get_fstclus(V, F.DirEntry), F.D.len);
	res = fat_update_file(&F);
	if (res != 0) {
		fprintf(stderr,"fat_mkdir() error: update_file() failed\n");
		return -1;
	}
	return 0;
}

/* delete() routine for libfat */
static int fat_real_delete(File_t *F, int dir, int flag) {
	DWORD fstclus;
	DWORD Cluster, Offset;
	DirEnt_t D;
	int res;

	// check for dir
	if ((ATTR_ISDIR(F->DirEntry->DIR_Attr)) && (dir != 1)) {
		perror("fat_delete(): file is a directory");
		return -1;
	} else if (!(ATTR_ISDIR(F->DirEntry->DIR_Attr)) && (dir != 0)) {
		return -1;
	}

	if (flag == 0) {
		// get first cluster of the file and unlinkn it
		fstclus = get_fstclus(F->V,F->DirEntry);
		res = fat_unlinkn(F->V, fstclus);
		if (res != 0) return -1;
	}

	// clone the dirent and erase it
	memcpy((char *) &D,(char *) &(F->D), sizeof(DirEnt_t));
	erase_dirent(&D);

	// write it down
	Cluster=F->D.clus;
	Offset=F->D.off;
	if (F->V->FatType == FAT32) {
		res = fat_write_data(F->V, NULL, &Cluster, &Offset, (char *) D.entry, (D.len * sizeof(DirEntry_t)) );
		if (res != (D.len * sizeof(DirEntry_t)) ) return -1;
	} else { // FAT12/16
		off64_t res64;
		res64 = byte_offset(F->V,Cluster, Offset);
		if ((res64 = lseek(F->V->blkDevFd, res64, SEEK_SET)) != byte_offset(F->V,Cluster, Offset)) {
			fprintf(stderr,"lseek error in fat_delete(). res: %d,line: %d\n",res,__LINE__); return -1; }
		if ((res = writen(F->V->blkDevFd,(char *) D.entry, (D.len * sizeof(DirEntry_t)))) != (D.len * sizeof(DirEntry_t))) {
			fprintf(stderr,"write error in fat_delete(). res: %d,line: %d\n",res,__LINE__); return -1; }
	}
	return 0;
}

int fat_delete(File_t *F, int dir) {
	return fat_real_delete(F, dir, 0);
}

/* rmdir() routine for libfat */
int fat_rmdir(File_t *F) {
	int res;
	DirEnt_t D;
	DWORD Offset = 64; // let's skip . and .. sfn entries
	DWORD Cluster = get_fstclus(F->V,F->DirEntry);

	res = fetch_next_direntry(F->V, &D, &Cluster, &Offset);
	if (res > 0) {
		perror("fat_rmdir(): directory not empty");
		return -1;
	}

	res = fat_delete(F, 1);
	if (res != 0) return -1;

	return 0;
}

/* truncate() routine for libfat */
int fat_truncate(File_t *F, DWORD len) {
	DWORD fsize = EFD(F->DirEntry->DIR_FileSize);
	DWORD clus;
	int res;
	DWORD Cluster, Next;

	if (len >= fsize) return len;
	if ((F->rootdir ==1)  && (len == 0)) { fprintf(stderr,"cant truncate rootdir to 0\n");return -1; }

	clus = (len / F->V->bpc);
	if (((len % F->V->bpc) != 0) || (len==0))	clus++;

	// follow the cluster chain up clus cluster;
	Cluster = get_fstclus(F->V,F->DirEntry);
	if (!(fat_legalclus(F->V,Cluster))) { fprintf(stderr,"fat_truncate(): line %d\n",__LINE__); return -1; }

//	fprintf(stderr,"fclus: %u, clus: %u\n",Cluster,clus);
	clus--;

	while(clus > 0) {
		res = fat_read_entry(F->V, Cluster, 0, &Next);
		if (res != 0) { fprintf(stderr,"fat_truncate(): line %d\n",__LINE__); return -1; }
		Cluster = Next;
		clus--;
	}

	res = fat_read_entry(F->V, Cluster, 0, &Next);
	if (res != 0) { fprintf(stderr,"fat_truncate() line %d\n",__LINE__); return -1; }

	// set cluster number clus to EOC and start unlink from there
	if (len > 0) {
		res = fat_writen_entry(F->V, Cluster, fat_eocvalue(F->V));
	} else {
		res = fat_writen_entry(F->V, Cluster, 0);
		set_fstclus(F->V,F->DirEntry,0);
		F->V->freecnt++;
	}	//setting fstclus to free

	if (res != 0) { fprintf(stderr,"fat_truncate(): line %d\n",__LINE__); return -1; }

	if (fat_iseoc(F->V,Next)) {
		// nothing to do
	} else {
		res = fat_unlinkn(F->V, Next);
		if (res != 0) { fprintf(stderr,"fat_truncate(): line %d\n",__LINE__); return -1; }
	}

	if ((F->rootdir != 1) && (!(ATTR_ISDIR(F->DirEntry->DIR_Attr)))) {
		// update filesize
		F->DirEntry->DIR_FileSize = EFD(len);

		// update file
		res = fat_update_file(F);
	}
	if (res != 0) { fprintf(stderr,"fat_truncate(): line %d\n",__LINE__); return -1; }

	return 0;
}

/*
readdir() routine for libfat */
int fat_readdir(File_t *Dir, struct dirent *de) {
	int res;
	DirEnt_t D;

//	fprintf(stderr,"fat_readdir: CurClus: %u, CurOff: %u\n",Dir->CurClus, Dir->CurOff);
	if	((res = fetch_next_direntry(Dir->V, &D, &(Dir->CurClus), &(Dir->CurOff))) <=0 ) {
		fprintf(stderr, "readdir: error in fetch_next_direntry\n"); return -1;
	} else {
		// fprintf(stderr, "readdir: res: %d\n",res);
	}

	if	((res = fatentry_to_dirent(Dir->V, &D, de)) < 0 ) return -1;

	return 0;
}

/* stat() routine for libfat */
int fat_stat(File_t *F, struct stat *st) {

	memset((char *) st, 0, sizeof(struct stat));

    st->st_dev = (int) (F->V);     /* ID of device containing file */
    st->st_nlink = 1;   /* number of hard links */
    st->st_uid = F->V->uid;     /* user ID of owner */
    st->st_gid = F->V->gid;     /* group ID of owner */
    st->st_rdev = 0;    /* device ID (if special file) */
    st->st_blksize = F->V->bpc; /* blocksize for filesystem I/O */

  if(F->rootdir==1) { //root dir
  	st->st_ino = 2;
	st->st_mode = S_IFDIR | S_IRWXU;
	st->st_size = 0;
	st->st_blocks=0;
    st->st_ctime = st->st_atime = st->st_mtime = 0;   /* time of last modification */
  } else {				// normal file or dir
	st->st_ino = get_fstclus(F->V,F->DirEntry);     /* inode number */

	if (ATTR_ISDIR(F->DirEntry->DIR_Attr)) {
		st->st_mode = S_IFDIR | S_IRWXU;						/* protection */
	} else {
		st->st_mode = S_IFREG | S_IRWXU;
	}
	st->st_size = EFD(F->DirEntry->DIR_FileSize);    /* total size, in bytes */
	st->st_blocks = (st->st_size / 512) + 1;  /* number of blocks allocated */
    st->st_ctime = st->st_atime = st->st_mtime = fat_mktime2(F->DirEntry);   /* time of last modification. tofix atime */
  }
  return 0;
}


/* utime() routine for libfat	*/
int fat_utime(File_t *F, struct utimbuf *buf) {
	if ((F==NULL)||(buf==NULL)) return -1;
	WORD accdate, acctime, moddate, modtime;

	fat_fill_time(&accdate, &acctime, buf->actime);
	fat_fill_time(&moddate, &modtime, buf->modtime);
	F->DirEntry->DIR_LstAccDate = accdate;
	F->DirEntry->DIR_WrtDate 	= moddate;
	F->DirEntry->DIR_WrtTime 	= modtime;
	return (fat_update_file(F));
}

/* statfs() routine for libfat	*/
int fat_statvfs(Volume_t *V, struct statvfs *buf) {
	if ((V==NULL) || (buf == NULL)) return -1;
buf->f_bsize = 1024;    /* file system block size */
buf->f_frsize = 1024;   /* fragment size */
buf->f_blocks= (V->DataClusters * (V->bpc / 1024));;   /* size of fs in f_frsize units */
buf->f_bfree = (V->freecnt * (V->bpc / 1024));    /* # free blocks */
buf->f_bavail= (V->freecnt * (V->bpc / 1024));;   /* # free blocks for non-root */
buf->f_files= V->DataClusters;    /* # inodes */
buf->f_ffree=V->freecnt;    /* # free inodes */
buf->f_favail=V->freecnt;   /* # free inodes for non-root */
buf->f_fsid=0x4d44;     /* file system ID */
buf->f_flag=0;     /* mount flags */
buf->f_namemax=255;  /* maximum filename length */
	return 0;
}

/*	open() routine for libfat  -- TO COMPLETE	*/
int fat_open(const char *filename, File_t *F, Volume_t *V, int flags) {
	int res;
	DWORD clus, off;

	if (filename == NULL) { fprintf(stderr,"fat_open(): invalid filename string\n"); return -1; }

	res = utf8_stricmp(filename,"");
	res = utf8_stricmp(filename,".");
	res |= utf8_stricmp(filename,"..");

	//lets check for . and ..
	if ((res == 0)) {
		fprintf(stderr,"fat_open(): cannot open \".\" or \"..\" or an empty string. filename: %s\n", filename);
		return -1;
    }

	F->rootdir=0;
	F->V = V;

	res = utf8_stricmp(filename,"/");
	if (res == 0) {// root directory
		if (V->FatType == FAT32) {
			F->CurClus = 2;
		} else { // fat12/16
			F->CurClus = 1;
		}
		F->CurOff = F->CurAbsOff = 0;
		F->rootdir = 1;
		F->DirEntry = NULL;
		F->Mode = flags;
		return 0;
    }

	/* todo: if filename is empty string, consider it as root dir */
    res = find_file(V, filename, F, &clus, &off);
	if (res != 0) {
		fprintf(stderr,"fat_open(): find file error fname: %s\n", filename);
		return -1;
	}

	F->DirEntryClus=clus;
	F->DirEntryOffset=off;

    memset(F->D.entry,0,21 * sizeof(LfnEntry_t));
	res = fetch_next_direntry(V, &(F->D), &clus, &off);
 	if (res <= 0) {
		perror("fat_open():"); return -1;
	} else  {
		F->DirEntry = (DirEntry_t *) &(F->D.entry[F->D.len - 1]);
		F->CurClus = get_fstclus(F->V,F->DirEntry);
		F->CurOff = F->CurAbsOff = 0;
		F->Mode = flags;
	}

	fprintf(stderr,"fat_open(%s): first cluster: %u, begins at %lld. direntry sz: %d:%d\n",filename, F->CurClus, byte_offset(V,F->CurClus, F->CurOff),res, F->D.len);
	return 0;
}

/* rename() routine for libfat */		// TOFIX: if destination is present rename returns error, but has already deleted the file
// TOFIX: behaviour with directories (mv dir1 dir2 puts dir1 into dir2 if dir2 is present)

int fat_rename(Volume_t *V, const char *from, const char *to) {
	int res;
    char dirnameto[4096];
	char filenameto[1024];
	File_t From, To, F, newParent;

	if ((res =  fat_open(from, &From, V, O_RDWR)) != 0) { fprintf(stderr,"fat_rename(): source file or directory doesnt exist"); return -ENOENT; }
	fat_dirname(to, dirnameto);
	fat_filename(to, filenameto);

	if ((res =  fat_open(dirnameto, &newParent, V, O_RDWR)) != 0) {
		fprintf(stderr,"fat_rename(): destination parent does not exist\n");
		return -1;
	}

	memcpy((char *) &F,(char *) &From, sizeof(File_t));
	if (ATTR_ISDIR(From.DirEntry->DIR_Attr)) { //directory
		if ((res =  fat_open(to, &To, V, O_RDWR)) == 0) {
			fprintf(stderr,"fat_rename(): destination file already exist: cant overwrite with a directory\n");
			return -1;
		}
		res = fat_real_delete(&From,1,1);
		if (res != 0) { fprintf(stderr,"delete directory error\n"); return -1; }
		res = fat_mkdir(V, &newParent, filenameto , F.DirEntry, 0);
		if (res != 0) { fprintf(stderr,"create new directory error\n"); return -1; }
	} else {								// regular file
		if ((res =  fat_open(to, &To, V, O_RDWR)) == 0) {
			if (ATTR_ISDIR(To.DirEntry->DIR_Attr)) {
				fprintf(stderr, "fat_rename() cant overwrite a directory with a file!\n");
				return -1;
			}
			if ((res = fat_delete(&To, 0)) != 0) {
				fprintf(stderr,"error while deleting destination file\n");
				return -1;
			}
		}
		res = fat_real_delete(&From,0,1);
		if (res != 0) { fprintf(stderr,"delete file error\n"); return -1; }
		res = fat_create(V, &newParent, filenameto , F.DirEntry, 0, 0);
		if (res != 0) { fprintf(stderr,"create file error\n"); return -1; }
	}
	return 0;
}

/*	This function calculate the cluster/offset pair related to an assolute
offset of a file. It put these informations inside *F.
It returns the offset on success or negative value on error. */
/* It now supports RW mode that will allocate clusters filled with 0es
if seek is beyond filesize*/

off64_t	fat_seek(File_t *F, off64_t offset, int whence) {
	int res;
	off64_t off;
	off64_t curabsoff= F->CurAbsOff;
	off64_t curoff = F->CurOff;
	off64_t clustersz = F->V->bpc64;
	int mode = F->Mode;


	if (F == NULL) { fprintf(stderr,"fat_seek(): NULL File.\n"); return -1; }
	if (offset < 0) { fprintf(stderr,"fat_seek(): invalid offset < 0\n"); return -1; }
	if ((F->rootdir == 1) || (ATTR_ISDIR(F->DirEntry->DIR_Attr))) mode=O_RDONLY;

// TOFIX	if (!fat_legalclus(V,F->CurClus)) { fprintf(stderr,"fat_seek(): illegal current cluster \n"); return -1; }

	/* Let's combine offset with whence	*/
	if (whence == SEEK_SET) {
		off = offset;
	} else if (whence == SEEK_CUR) {
		off = F->CurAbsOff + offset;
	} else if (whence == SEEK_END) {
		off = EFD(F->DirEntry->DIR_FileSize) + offset;
	} else { fprintf(stderr,"fat_seek(): unknown Whence\n"); return -1;	}	// error

	/* Let's test if the file is empty. if so let's allocate its first cluster, set it, update the file.	*/

	if ((mode == O_RDONLY) && (off >= EFD(F->DirEntry->DIR_FileSize))) { fprintf(stderr,"fat_seek(): cant seek beyond EOF in O_RDONLY\n"); return -1; }

	if ((F->rootdir != 1) && (!fat_legalclus(F->V,get_fstclus(F->V,F->DirEntry))) && (F->DirEntry->DIR_FileSize == 0)) { /* 0 is endianess independent */
		if (mode != O_RDONLY) {
		/* Let's allocate the first cluster */
			DWORD freecls, freeoff;

			if ((freecls = fat_getFreeCluster(F->V)) == 0) return -1;

			freeoff=0;

            set_fstclus(F->V,F->DirEntry, freecls);
//			fprintf(stderr,"fat_seek(): empty file in O_RDWR. fclus allocated: %d\n",freecls);
            F->CurClus = freecls;
            F->CurOff  = 0;
			F->CurAbsOff = 0;
			F->DirEntry->DIR_FileSize = 0;
            fat_writen_entry(F->V, freecls,  fat_eocvalue(F->V));
//			res = fat_write0data(F->V, NULL, &freecls, &freeoff, F->V->bpc);	// let's zero the cluster content; better not, since then we allocate 1 free clus.
//          if (res != F->V->bpc) { fprintf(stderr, "write0data() error. res= %d, line: %d\n",res,__LINE__); return -1; }

//            res = fat_update_file(F);
//            if (res != 0) { fprintf(stderr, "update file error. res= %d, line: %d\n",res,__LINE__); return -1; }
		} else {
		/* Empty file in O_RDONLY == Error  */
			fprintf(stderr,"file is empty. Unpossible to perform read only actions.\n"); return -1;
		}
	}

	/* Main IF */
	if (off < curabsoff) {	// Given this condition we can suppose we will encounter only allocated clusters in this branch
		if ( (curabsoff - off) > curoff) {	// the offset is in a previous cluster
			DWORD newclus;					// we start from the beginning and reach the correct cluster/offset pair
			off64_t newoff = off;

			if (F->rootdir != 1) {
				newclus = get_fstclus(F->V,F->DirEntry);
			} else newclus = 2;

			while (newoff >= clustersz) {
				if ( (res = fat_read_entry(F->V, newclus, 0, &newclus)) != 0 ) {
					fprintf(stderr,"fat_seek() error at line %d\n",__LINE__); return -1; }
				if (!fat_legalclus(F->V,newclus)) {	// actually we should be able to seek beyond EOF and fill with 0. TO FIX but not here
					fprintf(stderr,"fat_seek(): end of clusterchain reached while offset < current offset\n"); return -1; }

				newoff -= clustersz;
			}

			// let's set cluster/offset pair into F
			F->CurClus = newclus;
			F->CurOff = newoff;
			F->CurAbsOff = off;
			return off;
		} else {								// the offset is in this cluster
			F->CurOff = curabsoff - off;
			F->CurAbsOff = off;
			return off;
		}
	} else if (off > curabsoff) {
		if ( (off - curabsoff) >= (clustersz - curoff) ) {			// the offset is in a following cluster
			DWORD newclus;
			off64_t	newoff = off - curabsoff - (clustersz - curoff);	// we start from this cluster and reach the correct cluster/offset pair

			if ( (res = fat_read_entry(F->V, F->CurClus, 0, &newclus)) != 0 ) { perror("fat_seek() error"); return -1; }

			if (fat_isbad(F->V,newclus)) { fprintf(stderr,"bad cluster in the chain\n"); return -1; }
			if (fat_isfree(F->V,newclus)) { fprintf(stderr,"free cluster in the chain, and no EOC\n"); return -1; }

			if (fat_iseoc(F->V,newclus)) {	// We must be beyond the filesize
				if (mode != O_RDONLY) {
				/* Lets allocate the cluster and fill with 0	*/
					DWORD freecls,cclus,coff;

					cclus = F->CurClus;
					coff =  (EFD(F->DirEntry->DIR_FileSize)) % F->V->bpc;
		//            fprintf(stderr,"fat_seek(): empty file in O_RDWR\n");
		            if ((freecls = fat_getFreeCluster(F->V)) == 0) return -1;
            		fat_writen_entry(F->V, F->CurClus , freecls);
					fat_writen_entry(F->V, freecls,  fat_eocvalue(F->V));
            		newclus = F->CurClus = freecls;
					F->CurOff  = 0;

					if (((EFD(F->DirEntry->DIR_FileSize) ) % F->V->bpc ) != 0) {
						F->DirEntry->DIR_FileSize = EFD(((EFD(F->DirEntry->DIR_FileSize) / F->V->bpc) * F->V->bpc) + F->V->bpc) ;

						res = fat_write0data(F->V, NULL, &cclus, &coff, (F->V->bpc - coff));	// let's zero the end of the cluster
						if (res != (F->V->bpc - coff)) { fprintf(stderr, "write0data() error. res= %d, line: %d\n",res,__LINE__); return -1; }
					}
    				cclus = freecls;
					coff = 0;
					res = fat_write0data(F->V, NULL, &cclus, &coff, F->V->bpc);	// let's zero the new cluster
					if (res != F->V->bpc) { fprintf(stderr, "write0data() error. res= %d, line: %d\n",res,__LINE__); return -1; }
				} else {
					perror("fat_seek(): end of clusterchain reached"); return -1;
				}
			}

			while (newoff >= clustersz) {
				if ( (res = fat_read_entry(F->V, newclus, 0, &newclus)) != 0 ) { perror("fat_seek() error"); return -1; }

				if (fat_isbad(F->V,newclus)) { fprintf(stderr,"bad cluster in the chain\n"); return -1; }
				if (fat_isfree(F->V,newclus)) { fprintf(stderr,"free cluster in the chain, and no EOC\n"); return -1; }

				if (fat_iseoc(F->V,newclus)) {	// We must be beyond the filesize
					if (mode != O_RDONLY) {
					/* Lets allocate the cluster and fill with 0	*/
						DWORD freecls,cclus,coff;

			            if ((freecls = fat_getFreeCluster(F->V)) == 0) return -1;
	            		fat_writen_entry(F->V, F->CurClus , freecls);
						fat_writen_entry(F->V, freecls,  fat_eocvalue(F->V));
	            		newclus = F->CurClus = freecls;
						F->CurOff  = 0;

						F->DirEntry->DIR_FileSize += EFD(F->V->bpc) ;

	    				cclus = freecls;
						coff = 0;
						res = fat_write0data(F->V, NULL, &cclus, &coff, F->V->bpc);	// let's zero the new cluster
						if (res != F->V->bpc) { fprintf(stderr, "write0data() error. res= %d, line: %d\n",res,__LINE__); return -1; }
					} else {
						perror("fat_seek(): end of clusterchain reached"); return -1;
					}
				}
				newoff -= clustersz;
			}

			// let's set cluster/offset pair into F
			F->CurClus = newclus;
			F->CurOff = newoff;
			F->CurAbsOff = off;
//			fprintf(stderr, "clus: %u, curclus: %u, off: %lld, curoff: %u\n",newclus, F->CurClus, newoff, F->CurOff);
//			if (mode != O_RDONLY) { if ((res = fat_update_file(F)) != 0) {
//				fprintf(stderr, "update file error. res= %d, line: %d\n",res,__LINE__); return -1; }
//			}
			return off;

		} else {								// the offset is in this cluster
			off64_t newoff = (off - curabsoff) + curoff;

			if (off >= EFD(F->DirEntry->DIR_FileSize)) {	// Offset beyond the filesize. we must zero a piece of the cluster
				if (mode != O_RDONLY) {
					DWORD cclus = F->CurClus;
					DWORD coff  = (EFD(F->DirEntry->DIR_FileSize) % F->V->bpc);
					res = fat_write0data(F->V, NULL, &cclus, &coff, (off - EFD(F->DirEntry->DIR_FileSize)));	// let's zero a piece of the clus
					if (res != (off - EFD(F->DirEntry->DIR_FileSize))) {
						fprintf(stderr, "write0data() error. res= %d, line: %d\n",res,__LINE__); return -1;
					}
				} else {
					fprintf(stderr,"fat_seek(): cant seek beyond filesize in O_RDONLY\n"); return -1;
				}
			}

			F->CurOff = newoff;
			F->CurAbsOff = off;
//			fprintf(stderr, "off: %lld, curoff: %u, clus:%u\n", newoff, F->CurOff,F->CurClus);
			return off;
		}
	} else { // offset == filesize
		// check if offset is >= filesize..
		// check if curoff is > clustersz
		if (curoff == clustersz) {
			// do nothing. write_data already handle this.
		}
		return offset;
	}
	return -2; // we should not reach here
}

/************************************************************************
*	Functions for character representation conversion				 	*
************************************************************************/


int utf16to8(const WORD *restrict source, char *restrict dest) // Seems to work.
{
    wchar_t wc;
    int res;
    while (*source) {
		res = unicode_utf16le_to_wchar(&wc, source, 2);
        if (res < 0) return res;
        source += res;
        res = unicode_wchar_to_utf8(dest, wc, 6); // why 6...
        if (res < 0) return res;
		dest += res;
    }
    *dest = 0;
    return 0;
}

/* return the length in characters of an utf8 null-terminated string excluding null terminator	*/
int utf8_strlen(char *s) {
    int i = 0;
    int cur = 0;
//    int res;

    for (;;) {
		if (s[cur] == 0) {
		    break;	// null terminator encountered
		} else {
			i++;
			cur += unicode_utf8_len(s[cur]) ;	// incrementing the cursor of character size
		}
    }
    return i;
}

/* check the string s for illegal characters not allowed in file names	*/
int utf8_strchk(char *s) {
    int i = 0;
    int cur = 0;
// char not allowed : < 0x20 || 7F || 5C || "*/:<>?/|

    for (;;) {
		if (s[cur] == 0) {
		    break;	// null terminator encountered
		} else if ((s[cur]<0x20)||(s[cur]==0x7F)||(s[cur]==0x5C)||(s[cur]=='\"')||(s[cur]=='*')||(s[cur]=='/')||(s[cur]=='\\')||
								(s[cur]==':')||(s[cur]=='<')||(s[cur]=='>')||(s[cur]=='?')||(s[cur]=='|')) {
			return 0;
		} else {
			i++;
			cur += unicode_utf8_len(s[cur]) ;	// incrementing the cursor of character size
		}
    }
    return 1;
}

/*	Converts the utf8 string source into utf16le string dest, null terminating it	*/
int utf8to16( const char *restrict source, WORD *restrict dest) {
    wchar_t wc;
    int res;
    while (*source)
    {
        res = unicode_utf8_to_wchar(&wc, source, 6); // chissa` perche` 6...
        if (res < 0) return -1;
        source += res;
        res = unicode_wchar_to_utf16le((uint16_t *) dest, wc, 2);
        if (res < 0) return -1;
		dest += res;
    }
    *dest = 0;
    return 0;
}

#if 0
/*	Convert the utf16le filename to ASCII 8+3 numeric tailed filename	*/
/*	return 0 on success, -1 on error									*/
/*  Used to create direntries.							  				*/
int utf8toSfn(WORD *restrict source, char *restrict dest, int len, int val) {
    char *ext;

	/*	splitting the filename	*/
    gchar **parts = g_strsplit(path, ".", -1);
    guint parts_len = g_strv_length(parts);

	if (parts_len > 1) {
    	ext=(char *) parts[parts_len - 1];
	} else {	//no extension
		ext = NULL;
	}
}
#endif

/*	Convert the utf16le string source to ASCII string dest. Used for printing	*/
/*	return the string length including the null terminator				*/
int utf16toASCII(WORD *restrict source, char *restrict dest, int len) {
    int i;
    int res;
    wchar_t wc;

    if ( len > (NAME_MAX+1) ) len = NAME_MAX+1;

    for (i=0; i < len;) {

        res = unicode_utf16le_to_wchar(&wc, &(source[i]), 2);

        if (wc < 0x000080) {
    	    dest[i] = (char) wc;
		    if ( dest[i] == 0 ) {
				return 0;
   		    }
    	}  else {
	    	dest[i] = 0x5F;
		}
    	i++;


    	if (i == len) {
			dest[len]=0;	//to be sure we have a null terminator
			return len;
    	} else {
    	    return i;
    	}
	}
	return 0;
}

int toupper(int ch) {							// Seems to work.
	unsigned char c = (unsigned char) ch;
	if (((c >= 'a') && (c <= 'z')) || ((c >= 0xE8) && (c <= 0xFE) && (c != 0xF7))) c -= 0x20;
	return (int) c;
}


/* Compares two UTF-8 strings, disregarding case.
 * Returns 0 if the strings match, a positive value if they don't match,
 * or a -EILSEQ if an invalid UTF-8 sequence is found.
 */
int utf8_stricmp(const char *s1, const char *s2) {		// Seems to work.
	wchar_t wc1, wc2;
	int res;
//	int i;
	for (;;) {
//	for (i=0;;i++) {
		if (!(*s2 & 0x80)) {  //    1000 0000
			if (toupper(*s1) != toupper(*s2)) return 1;
			if (*s1 == 0) return 0;	// *s2 == *s1
			s1++;
			s2++;
		} else {
			res = unicode_utf8towc(&wc1, s1, 6);
			if (res < 0) return res;
			s1 += res;
			res = unicode_utf8towc(&wc2, s2, 6);
			if (res < 0) return res;
			s2 += res;
			if (unicode_simple_fold(wc1) != unicode_simple_fold(wc2)) return 1;
		}
	}
	return -1;
}

/* variant which compares only first n chars	*/

int utf8_strncmp(const char *s1, const char *s2, int n) {		// Seems to work.
	wchar_t wc1, wc2;
	int res;
	int i;
//	for (;;) {
	for (i=0;i < n;i++) {
		if (!(*s2 & 0x80)) {  //    1000 0000
			if (toupper(*s1) != toupper(*s2)) return 1;
			if (*s1 == 0) return 0;	// *s2 == *s1
			s1++;
			s2++;
		} else {
			res = unicode_utf8towc(&wc1, s1, 6);
			if (res < 0) return res;
			s1 += res;
			res = unicode_utf8towc(&wc2, s2, 6);
			if (res < 0) return res;
			s2 += res;
			if (unicode_simple_fold(wc1) != unicode_simple_fold(wc2)) return 1;
		}
	}
	return 0;
}

/* modifies string filename such that it cointains the dirname of the file, stripping the filename  */
int fat_dirname(const char *path, char *dest) {
	char *slash;
	strcpy(dest, path);
	slash = strrchr(dest, 0x2F); // 0x2F = "/"
	if (slash == &(dest[0])) { dest[1] = 0; return 0; } // root dir
	*slash  = 0;
	return 0;
}

int fat_filename(const char *path, char *dest) {
	char *slash;
	slash = strrchr(path, 0x2F); // 0x2F = "/"
	slash++;
	strcpy(dest, slash);
	return 0;
}
