// COMPAT

typedef u32int uint32_t;
typedef u16int uint16_t;


#include "unicode.h"



/*
#ifdef WX_UNICODE_TEST
    #include "wx/wx.h"
    #include "utils/unicode-fd32.h"
#endif
*/

// Mainly for NULL
#include <stdlib.h>

// Simple functions to handle conversion to and from Unicode/UTF8
// and also to do folding (for case insensitive string comparing)
//
// The file fold_tbl.c is needed for case folding.

static unsigned char g_utf8_length[256];
static int g_did_init_length;

void InitUtf8Lengths() {
    // Fill in lengths in array above
    for( int lb=0; lb<256; lb++ ){
        int l = -1;
        if( !(lb&0x80) ) l=1;
        else if( (lb&0xE0)==0xC0 ) l=2;
        else if( (lb&0xF0)==0xE0 ) l=3;
        else if( (lb&0xF8)==0xF0 ) l=4;
        else if( (lb&0xFC)==0xF8 ) l=5;
        else if( (lb&0xFE)==0xFC ) l=6;
        g_utf8_length[lb] = l;
    }
    g_did_init_length = 1;
}

int unicode_utf8_len(int lead_byte){_CFE_;
    if( !(lead_byte&0x80) ) return 1;  // Special case, make faster
    if( !g_did_init_length )
        InitUtf8Lengths();

    return g_utf8_length[(unsigned char)lead_byte];
}

int unicode_utf8_len_first(const char *pc){_CFE_;
    int lb = *(unsigned char*)pc;
    if( !(lb&0x80) ) return 1;  // Special case, make faster
    if( !g_did_init_length )
        InitUtf8Lengths();

    int l = g_utf8_length[(unsigned char)lb];
    if( l>0 ) return l;

    // Invalid UTF8 lead byte. Look for next valid character.
    const char *pc1 = pc+1;
    while( ((*pc1)&0xC0)==0x80 )
        pc1++;
    return pc1 - pc;
}

int unicode_utf8_to_wchar(int *result, const char *string){_CFE_;
    #ifdef WX_UNICODE_TEST
	int res=-1;
    unicode_utf8_to_wchar( (wchar_t*)&res, string, 4 );
    #endif

    // Assume pointers to be OK
    /*unsigned char*/ int ch = *(unsigned char*)string;
    int l = unicode_utf8_len(ch);
    int lres = l;

    if( l<1  ) return -1;
    int wc = l>1 ? (ch&(0x7F>>l)) : ch;
    while( l>1 ){
        wc = (wc<<6) + (*++string & 0x3F);
        l--;
    }
    *result = wc;

    #ifdef WX_UNICODE_TEST
    wxASSERT( res==wc );
    #endif

    return lres;
}

// Checks if output fits in 1/4/6 bytes buffer
int unicode_wchar_to_utf8(char *s, int wc, int size){_CFE_;
    if( size<1 ) return -1;
    if( (unsigned int)wc>=0x80000000 ) return -2;

    // Single byte case
    if( wc<0x80 ){
        *s = (char)wc;
        //*s = (char)wc&0x7F;
        return 1;
    }
    if( size<4 ) return -3;

    // Two or more UTF8 bytes
    int p = 1;  // Index of last UTF8 byte
    if( wc>0x7FF ){ // 11 bits
        // Three or more UTF8 bytes
        p++; // p>=2
        if( wc>0xFFFF ){    // 16 bits
            // Four or more UTF8 bytes
            p++; // p>=3
            if( wc>0x1FFFFF ){  // 21 bits
                // Five or more UTF8 bytes
                if( size<6 ) return -3;
                p++; // p>=4 UTF8 bytes
                if( wc>0x3FFFFFF ){ // 26 bits
                    // Six UTF8 bytes
                    p++; // p>=5
                    if( (unsigned int)wc>(unsigned int)0x7FFFFFF ){  // 31 bits
						// Would need 7 UTF8 bytes. Not supported.
						return -10;
                    }
                    s[p-4] = 0x80 | ((wc>>24)&0x3F);    // Bit 24..29
                }
                s[p-3] = 0x80 | ((wc>>18)&0x3F);    // Bit 18..23
            }
            s[p-2] = 0x80 | ((wc>>12)&0x3F);    // Bit 12..17
        }
        s[p-1] = 0x80 | ((wc>>6)&0x3F);    // Bit 6..11
    }
    s[p] = 0x80 | (wc&0x3F);    // Bit 0..5
    s[0] = (0xFC << (5-p)) | (wc>>(p*6));

    #ifdef WX_UNICODE_TEST
    char buf[8];
    int l = unicode_wchar_to_utf8( buf, wc, 8 );
    wxASSERT( !strncmp(buf,s,p+1) && p+1==l );
    #endif

    return p+1;
}


extern const unsigned short *g_uc_fold_pages [];
extern const unsigned int g_uc_fold_page_260 [];

int unicode_simple_fold( int wc ){_CFE_;
    int page = ((unsigned int)wc) >> 8;
    const unsigned short *ps = NULL;
    if( page<256 ){
        ps = g_uc_fold_pages[page];
        return page && ps ? (int)ps[wc&0xFF] : wc;
    }
    else if( page==260 )
        return g_uc_fold_page_260[wc&0xFF];
    else
        return wc;
}


















//This is "libfat.h" - Driver's constants, structures and prototypes

#ifndef __LIBFAT_H
#define __LIBFAT_H

//#define FATWRITE
#define _LARGEFILE64_SOURCE
//#define __USE_LARGEFILE64

#include <util/cpp.h>

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
//This is "libfat.h" - Driver's constants, structures and prototypes

#ifndef __BITS_LIBFAT_H
#define __BITS_LIBFAT_H

#ifndef __INSIDE_LIBFAT_H
#error "Never use <bits/libfat.h> directly; include <libfat.h> instead."
#endif
#if __BYTE_ORDER == __BIG_ENDIAN
#define EFW(X) bswap_16(X)
#define EFD(X) bswap_32(X)
#else
#define EFW(X) (X)
#define EFD(X) (X)
#endif


/*	Maximum number of file opened	*/
#define MAX_OPENED_FILES	4096

 /* largest sector size */
#define MAX_SECTORSIZE		8192

/* FAT directory entry size in bytes*/
#define FAT_DIRSIZE		32

/* largest cluster size (sure?) */
#define MAX_CLUSTERSIZE		8192

 /* largest MSDOS path length */
#define MAX_PATHLENGTH		128

 /* largest directory (in sectors) */
#define MAX_DIRSIZE		64

/* size of free cluster array in Volume_t	*/
#ifndef FCLUS_BUFSZ
#define FCLUS_BUFSZ 8192
#endif

/* Maximum number of bytes per cluster: 32k, according to MS for max compatibility */
#define MAX_BYTES_PER_CLUSTER (32*1024)

#define fat_lock(V)
#define fat_unlock(V)

#ifndef ZERO_BFSZ
#define ZERO_BFSZ 8192
#endif


/* FAT Types */
typedef enum { FAT12, FAT16, FAT32 } FatType_t;

/* EOC (End Of Clusterchain) check macros.                               */
/* These expressions are true (nonzero) if the value of a FAT entry is   */
/* an EOC for the FAT type. An EOC indicates the last cluster of a file. */
#define  FAT12_ISEOC(EntryValue)  ((EntryValue) >= 0x0FF8)
#define  FAT16_ISEOC(EntryValue)  ((EntryValue) >= 0xFFF8)
#define  FAT32_ISEOC(EntryValue)  (((EntryValue) & 0x0FFFFFFF) >= 0x0FFFFFF8)	//??????

#define  FAT12_ISFREE(EntryValue) (((EntryValue) & 0x0FFF) == 0x0)
#define  FAT16_ISFREE(EntryValue) (((EntryValue) & 0xFFFF) == 0x0)
#define  FAT32_ISFREE(EntryValue) (((EntryValue) & 0x0FFFFFFF) == 0x00000000)

#define	 FAT12_EOC_VALUE	0x0FFF
#define  FAT16_EOC_VALUE	0xFFFF
#define	 FAT32_EOC_VALUE	0x0FFFFFF8	//it was 0x0FFFFFFF but linux driver set it to F8



/* Bad cluster marks.                                                    */
/* Set a FAT entry to the FATxx_BAD value to mark the cluster as bad.    */
/*                                                                       */
/* The FAT file system specification says that to avoid confusion, no    */
/* FAT32 volume should ever be configured such that 0x0FFFFFF7 is an     */
/* allocatable cluster number. In fact an entry that would point to the  */
/* cluster 0x0FFFFFF7 would be recognised as Bad instead. Since values   */
/* greater or equal than 0x0FFFFFF8 are interpreted as EOC, I think we   */
/* can assume that the max cluster for a FAT32 volume is 0x0FFFFFF6.     */
/* That problem doesn't exist on FAT12 and FAT16 volumes, in fact:       */
/* 0x0FF7 = 4087 is greater than 4086 (max cluster for a FAT12 volume)   */
/* 0xFFF7 = 65527 is greater than 65526 (max cluster for a FAT16 volume) */
#define FAT12_BAD_VALUE 0x0FF7
#define FAT16_BAD_VALUE 0xFFF7
#define FAT32_BAD_VALUE 0x0FFFFFF7


#define  FAT12_ISBAD(EntryValue)  (EntryValue == 0x0FF7)
#define  FAT16_ISBAD(EntryValue)  (EntryValue == 0xFFF7)
#define  FAT32_ISBAD(EntryValue)  (EntryValue == 0x0FFFFFF7)

#define  FAT12_LEGALCLUS(EntryValue)  (!( (FAT12_ISEOC(EntryValue)) || (FAT12_ISFREE(EntryValue)) || (FAT12_ISBAD(EntryValue))))
#define  FAT16_LEGALCLUS(EntryValue)  (!( (FAT16_ISEOC(EntryValue)) || (FAT16_ISFREE(EntryValue)) || (FAT16_ISBAD(EntryValue))))
#define  FAT32_LEGALCLUS(EntryValue)  (!( (FAT32_ISEOC(EntryValue)) || (FAT32_ISFREE(EntryValue)) || (FAT32_ISBAD(EntryValue))))

/* FAT Date Encoding */
/*
 *      hi byte     |    low byte
 *  |7|6|5|4|3|2|1|0|7|6|5|4|3|2|1|0|
 *  | | | | | | | | | | | | | | | | |
 *  \   7 bits    /\4 bits/\ 5 bits /
 *     year +80     month     day
*/
#define FILE_YEAR(dir) (( ((BYTE *) &((dir)->DIR_WrtDate))[1] >> 1) + 1980)
#define FILE_MONTH(dir) ((((((BYTE *) &((dir)->DIR_WrtDate))[1]&0x1) << 3) + (((BYTE *) &((dir)->DIR_WrtDate))[0] >> 5)))
#define FILE_DAY(dir) (((BYTE *) &((dir)->DIR_WrtDate))[0] & 0x1f)

/* FAT Time Encoding */
/*
 *      hi byte     |    low byte
 *  |7|6|5|4|3|2|1|0|7|6|5|4|3|2|1|0|
 *  | | | | | | | | | | | | | | | | |
 *  \  5 bits /\  6 bits  /\ 5 bits /
 *     hour      minutes     sec*2
*/
#define FILE_HOUR(dir) (((BYTE *) &((dir)->DIR_WrtTime))[1] >> 3)
#define FILE_MINUTE(dir) ((((((BYTE *) &((dir)->DIR_WrtTime))[1]&0x7) << 3) + (((BYTE *) &((dir)->DIR_WrtTime))[0] >> 5)))
#define FILE_SEC(dir) ((((BYTE *) &((dir)->DIR_WrtTime))[0] & 0x1f) * 2)

/* FAT32 Boot Sector and BIOS Parameter Block                         */
/* This structure is used in all driver functions, even if the volume */
/* is FAT12 or FAT16. In fact some FAT32 fields are checked anyway    */
/* like BPB_FATSz32). So we load the boot sector from the disk, we    */
/* detect the FAT type and if it's a FAT32, we fill the following     */
/* struct as is, while if the volume is FAT12/FAT16 we copy in the    */
/* right position the appropriate (common) fields.                    */

typedef struct
{
  BYTE  BS_jmpBoot[3];		/* 0  Jump to boot code.                BS_jmpBoot */
  BYTE  BS_OEMName[8];		/* 3  OEM name & version.               BS_OEMName */
  WORD  BPB_BytsPerSec;		/* 11 Bytes per sector hopefully 512.   BPB_BytsPerSec */
  BYTE  BPB_SecPerClus;		/* 13 Cluster size in sectors.          BPB_SecPerClus */
  WORD  BPB_ResvdSecCnt;	/* 14 Number of reserved (boot) sectors. BPB_RsvdSecCount */
  BYTE  BPB_NumFATs;		/* 16 Number of FAT tables hopefully 2. BPB_NumFATs */
  WORD  BPB_RootEntCnt;		/* 17 Number of directory slots.        BPB_RootEntCnt */
  WORD  BPB_TotSec16;		/* 19 Total sectors on disk.            BPB_TotSec16 */
  BYTE  BPB_Media;		/* 21 Media descriptor=first byte of FAT. BPB_Media */
				/*    0xF8 is the standard value for fixed media */
  WORD  BPB_FATSz16;		/* 22 Sectors in FAT.                   BPB_FATSz16 */
  WORD  BPB_SecPerTrk;		/* 24 Sectors/track.                    BPB_SecPerTrk */
  WORD  BPB_NumHeads;		/* 26 Heads.                            BPB_NumHeads */
  DWORD BPB_HiddSec;		/* 28 number of hidden sectors.         BPB_HiddSec */
  DWORD BPB_TotSec32;		/* 32 big total sectors.                BPB_TotSec32 */

  /* Here start the FAT32 specific fields (offset 36) */

  DWORD BPB_FATSz32;		/* 36 32bit count of sectors occupied by each FAT. BPB_FATSz16 must be 0    */
  WORD  BPB_ExtFlags;		/* 40 extension flags. Usually 0.
                                 * Bits 0-3: Zero-based number of active Fat. Valid if mirroring is disabled
                                 * Bits 4-6: Reserved
                                 * Bit 7: 0 = FAT mirrored into all FATs at runtime. 1 = only 1 FAT active
                                 * Bits 8-15: Reserved */

  WORD  BPB_FSVer;		/* 42 Version number of the FAT32 volume. For future extension. Must be 0:0 */
  DWORD BPB_RootClus;		/* 44 start cluster of root dir. Usually 2 */
  WORD  BPB_FSInfo;		/* 48 changeable global info */
  WORD  BPB_BkBootSec;		/* 50 back up boot sector. Recomended 6 */
  BYTE  BPB_Reserved[12];	/* 52 reserved for future expansion */

  /* The following fields are present also in a FAT12/FAT16 BPB, (labelblk_t)   */
  /* but at offset 36. In a FAT32 BPB they are at offset 64 instead. */

  BYTE BS_DrvNum;		/* 64 physical drive ?		*/
  BYTE BS_Reserved1;		/* 65 reserved			*/
  BYTE BS_BootSig;		/* 66 dos > 4.0 diskette. signature.	*/
  DWORD BS_VolID;		/* 67 serial number 		*/
  BYTE BS_VolLab[11];		/* 71 disk label 		*/
  BYTE BS_FilSysType[8];	/* 82 FAT type 			*/
}
__attribute__ ((packed)) Bpb_t;

/* FAT12 and FAT16 Structure starting at Offset 36 of sector 0 */

typedef struct
{
        BYTE BS_DrvNum;		/* 36 physical drive ?		*/
        BYTE BS_Reserved1;	/* 37 reserved			*/
        BYTE BS_BootSig;	/* 38 dos > 4.0 diskette. signature.	*/
        DWORD BS_VolID;		/* 39 serial number 		*/
        BYTE BS_VolLab[11];	/* 43 disk label 		*/
        BYTE BS_FilSysType[8];	/* 54 FAT type 			*/
}
__attribute__ ((packed)) labelblk_t;

/* FAT32 FSInfo Sector structure */
typedef struct
{
  DWORD FSI_LeadSig;		/* FSI_LeadSig.   0x41615252        				*/
  BYTE  FSI_Reserved1[480];	/* FSI_Reserved1 (size 480 bytes should be 0)  			*/
  DWORD FSI_StrucSig;		/* FSI_StructSig. 0x61417272    				*/
  DWORD FSI_Free_Count;		/* FSI_Free_Count.                      			*/
  DWORD FSI_Nxt_Free;		/* FSI_Nxt_Free. Just an hint. 0xFFFFFFFF =  no hint available	*/
  BYTE  FSI_Reserved2[12];	/* FSI_Reserved2                        			*/
  DWORD FSI_TrailSig;		/* FSI_TrailSig.  0xAA550000					*/
}
__attribute__ ((packed)) FSInfo_t;

/* FAT 32-byte Directory Entry structure */
typedef struct
{
  BYTE  DIR_Name[11];		/*  0 file name (8+3) */
  BYTE  DIR_Attr;		/* 11 attribute byte */
  BYTE  DIR_NTRes;		/* 12 case of short filename */ /* MS says "reserved for NT */
  BYTE  DIR_CrtTimeTenth;	/* 13 creation time, milliseconds (?) */
  WORD  DIR_CrtTime;		/* 14 creation time */
  WORD  DIR_CrtDate;		/* 16 creation date */
  WORD  DIR_LstAccDate;		/* 18 last access date */
  WORD  DIR_FstClusHI;		/* 20 start cluster, Hi */
  WORD  DIR_WrtTime;		/* 22 time stamp */
  WORD  DIR_WrtDate;		/* 24 date stamp */
  WORD  DIR_FstClusLO;		/* 26 starting cluster number */
  DWORD DIR_FileSize;		/* 28 size of the file */
}
__attribute__ ((packed)) DirEntry_t;

// sfentry == DirEntry_t, value = DWORD
#define FIRST_CLUSTER(sfnentry, value) ((WORD *) &value)[0] = ((DirEntry_t) sfnentry).DIR_FstClusLO; ((WORD *) &value)[1] = ((DirEntry_t) sfnentry).DIR_FstClusHI

/* Special codes for the first byte of a directory entry (DIR_Name[0] */
#define FREEENT  0xE5 /* The directory entry is free             */
#define ENDOFDIR 0x00 /* This and the following entries are free */
#define DIRENT_ISFREE(D)	(((D) == FREEENT) || ((D) == ENDOFDIR))
#define DIRENT_ISLAST(D)	(D == ENDOFDIR)


/*	2bytes trailing signature that occupies the byte 510 and 511 of the boot sector,bkbootsector and FSinfo sector	*/
#define BPB_TRAILSIG	0x55AA

/* Attributes for DIR_Attr	*/
#define ATTR_READ_ONLY 0x1
#define ATTR_HIDDEN 0x2
#define ATTR_SYSTEM 0x4
#define ATTR_VOLUME_ID 0x8
#define ATTR_DIRECTORY 0x10
#define ATTR_ARCHIVE 0x20
#define ATTR_LONG_NAME ( ATTR_READ_ONLY | ATTR_HIDDEN | ATTR_SYSTEM | ATTR_VOLUME_ID )

#define ATTR_ISDIR(D)	( ( (D) & ATTR_DIRECTORY ) ==  ATTR_DIRECTORY )

/* values used by libfat. not correlated with fat file system values */

#define LIBFAT_DIRENT_FREE      0x01  //(0001)
#define LIBFAT_DIRENT_LASTFREE  0x09  //(1001)
#define LIBFAT_DIRENT_SFN       0x02  //(0010)
#define LIBFAT_DIRENT_LFN       0x04  //(0100)
#define LIBFAT_DIRENT_LFN_LAST  0x0C  //(1100)

#define LIBFAT_DIRENT_ISFREE(res)	((res & LIBFAT_DIRENT_FREE) == LIBFAT_DIRENT_FREE)
#define LIBFAT_DIRENT_ISLASTFREE(res)	((res & LIBFAT_DIRENT_LASTFREE) == LIBFAT_DIRENT_LASTFREE)
#define LIBFAT_DIRENT_ISSFN(res)	((res & LIBFAT_DIRENT_SFN) == LIBFAT_DIRENT_SFN)
#define LIBFAT_DIRENT_ISLFN(res)	((res & LIBFAT_DIRENT_LFN) == LIBFAT_DIRENT_LFN)
#define LIBFAT_DIRENT_ISLFNLAST(res)	((res & LIBFAT_DIRENT_LFN_LAST) == LIBFAT_DIRENT_LFN_LAST)

/* values used in fat file system	*/

#define LFN_LASTENTRY		0x40	/*	Last entry of a LFN set	*/
#define LFN_PADDING		0xFF	/*	padding for last LFN entry, if needed	*/
#define LFN_NULL		0x00	/*	null terminator for last LFN entry	*/

#define LFN_ISNULL(D)		( D == LFN_NULL )
#define LFN_ISLAST(D)		((D & LFN_LASTENTRY) == LFN_LASTENTRY)


/* FAT 32-byte Long File Name Directory Entry structure */
typedef struct
{
  BYTE LDIR_Ord;	/* Sequence number for slot        */
  WORD LDIR_Name1[5];	/* First 5 Unicode characters      */
  BYTE LDIR_Attr;	/* Attributes, always 0x0F         */
  BYTE LDIR_Type;	/* Reserved, always 0x00           */
  BYTE LDIR_Chksum;	/* Checksum of 8.3 name            */
  WORD LDIR_Name2[6];	/* 6 more Unicode characters       */
  WORD LDIR_FstClusLO;	/* First cluster number, must be 0 */
  WORD LDIR_Name3[2];	/* Last 2 Unicode characters       */
}
__attribute__ ((packed)) LfnEntry_t;

/* Linked likst structure for free clusters				*/

struct llist {
	int n;
	struct llist *next;
};

typedef struct llist FreeClus_t;

/* This structure stores all informations about a FAT volume.       */
/* It is the P structure (private data) for file system volume ops. */
typedef struct
{
  /* Data referring to the hosting block device */
  DWORD      blkDevFd; 			/* File Descriptor of the block device hosting the volume   */
  mode_t	 mode;				/* Permissions for the volume								*/
  uid_t      uid;     			/* user ID of owner */
  gid_t      gid;     			/* group ID of owner */

  /* Some precalculated data */

  DWORD      VolSig;          	/* Must be FAT_VOLSIG for a valid volume   */
  FatType_t  FatType;         	/* Can be FAT12, FAT16 or FAT32            */
  DWORD      DataClusters;    	/* The total number of valid not reserved data clusters (last valid data cluster is dataclusters + 1 */
  DWORD      FirstDataSector; 	/* The first sector of the data region, usually the beginning of rootdir in fat32     */
  DWORD      FirstRootCluster;	/* The first sector of FAT12/FAT16 root cluster, usually 2 (useless at the moment)   */
  DWORD    freecnt;  			/* The count of free clusters              */
  DWORD    nextfree;    		/* The cluster number from which to start  */
                              	/* to search for free clusters, if known   */
  int numfats;

  DWORD		freeclus[FCLUS_BUFSZ];
  int 		fstfclus;
  int 		fclusz;

  int bps;
  int spc;
  int bpc;
  DWORD fatsz;					// It's ok to have an int here, cause a fat can have up to 2^28 DWORDs
  int rsvdbytecnt;				// count of reserved bytes before fat 0

  off64_t bps64; 	// bytes per sector    = V->Bpb.BPB_BytsPerSec;
  off64_t spc64;	// sectors per cluster        d=V->Bpb.BPB_SecPerClus;
  off64_t bpc64;	// bytes per cluster    =
  off64_t fds64;	// first data serctor			        e=V->FirstDataSector;
  off64_t fdb64;		// first data byte

  off64_t rootdir16off;
  int rootdir16sz;

  /* a pthread mutex */
  pthread_mutex_t fat_mutex;

  /* for write0data (in seek) */
  char zerobuf[ZERO_BFSZ];

  /* FAT in fat12/16 volumes */
  char *fat;

  /* The BIOS Parameter Block of the volume (the long entry) */
  Bpb_t       Bpb;
  FSInfo_t	  Fsi;
} Volume_t;

/*	Structure for dirents	*/
typedef struct
{
	DWORD		clus;
	DWORD		off;
	off64_t		off1;
	off64_t		off2;
	off64_t		direntoff;
	int 		len1;
	int			len2;
	int 		len;
	LfnEntry_t 	entry[21];
	int last;
} DirEnt_t;

/* The file structure stores all the informations about an open file. */	// VA CAMBIATA!!!!!!!!!!!!!!!!
typedef struct
{
  Volume_t  *V;               /* Pointer to the volume hosting the file */
  DWORD     ParentFstClus;   /* First cluster of the parent directory  */
  DWORD		ParentOffset;	 /* Offset of the direntry	in parent file */
  DWORD		DirEntryClus;	 /* Cluster containing the direntry			*/
  DWORD     DirEntryOffset;  /* Offset of the dir entry in the cluster  */
//  DWORD     DirEntrySector;  /* Sector containing the directory entry  */
//  DWORD     DirEntrySecOff;  /* Byte offset of the 1st dir entry in sector */
//  off64_t	AbsOffset;		 /* Absolute offset in the fs of the 1st dir entry */
  DirEnt_t 	D;        /* The file's directory entry             */
  DirEntry_t *DirEntry;		 /* Pointer to sfn entry at the end of the chain */
  int	    Mode;            /* File opening mode                      */
  char 		FileName[511];		 /* Utf8 filename							  */
  int		rootdir;			/* 1 if the file refers to rootdir */

  /* The following fields refer to the byte position into the file */
  DWORD		CurClus;		 /* Cluster where the file offset is atm	*/
  DWORD		CurOff;			 /* Offset in the current cluster			*/
  off64_t	CurAbsOff;		 /* Absolute offset related to the beginning of the file	*/
							 /* Useful to know if we have to go ahead or restart from the beginning */
} File_t;

/*	Prototypes			*/

/*	FAT access functions . these should be static so we dont have to declare them here		*/
int fat32_read_entry(Volume_t *V, DWORD N, int FatNum, DWORD *Value);
#ifdef FATWRITE
int fat32_write_entry(Volume_t *V, DWORD N, int FatNum, DWORD Value);
int fat32_writen_entry(Volume_t *V, DWORD N, DWORD Value);
#endif
time_t fat_mktime2(DirEntry_t *D);
int fat_fill_time(WORD *Date, WORD *Time, time_t t);
int fat_isfree(Volume_t *V,DWORD value);
int fat_isbad(Volume_t *V,DWORD value);
int fat_iseoc(Volume_t *V,DWORD value);
int fat_legalclus(Volume_t *V,DWORD value);
DWORD fat_eocvalue(Volume_t *V);

/*	Directory entry functions			*/

int fat_populate_freelist(Volume_t *V);
DWORD   fat_getFreeCluster(Volume_t *V);
BYTE lfn_checksum(BYTE *name);
int analize_dirent(LfnEntry_t *D);
int  check_cluster_bound(Volume_t *V, DWORD *Cluster, DWORD *Offset);
int fetch_entry(Volume_t *V, DWORD *Cluster, DWORD *Offset, LfnEntry_t *D);
int fetch_next_direntry(Volume_t *V, DirEnt_t *D, DWORD *Cluster, DWORD *Offset);
int check_lfn_order(LfnEntry_t *Buffer, int bufsize);
int check_lfn_checksum(LfnEntry_t *Buffer, int bufsize);
WORD fetch_lfn_char(LfnEntry_t *D, int n);
int find_lfn_length( LfnEntry_t *D, int bufsize);
int extract_lfn_name( LfnEntry_t *Buffer, int bufsize, WORD *dest, int length);
int find_sfn_length( DirEntry_t *D, int bufsize);
int extract_sfn_name(DirEntry_t *D, int bufsize, char *name);
int fatentry_to_dirent(Volume_t *V, DirEnt_t *D, struct dirent *dirp);
int find_direntry(Volume_t *V, char *name, DWORD *Cluster, DWORD *Offset);
int traverse_path(Volume_t *V, gchar **parts, guint parts_len, DWORD *Cluster);
int find_file(Volume_t *V, const char *path, File_t *F, DWORD *Cluster, DWORD *Offset);
int fat_fat_sync(Volume_t *V);
#endif /* #ifdef _BITS_LIBFAT_H */#undef __INSIDE_LIBFAT_H
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
