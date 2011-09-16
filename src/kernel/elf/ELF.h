/*
 * ELF executable loading
 * Copyright (c) 2003, Jeffrey K. Hollingsworth <hollings@cs.umd.edu>
 * Copyright (c) 2003, David H. Hovemeyer <daveho@cs.umd.edu>
 * $Revision: 1.14 $
 *
 * This is free software.  You are permitted to use,
 * redistribute, and modify it as specified in the file "COPYING".
 */

#ifndef GEEKOS_ELF_H
#define GEEKOS_ELF_H
#include <util/cpp.h>
#include <io/FileObject.h>

#define PT_NULL 0
#define PT_LOAD 1
#define PT_DYNAMIC 2
#define PT_INTERP 3
#define PT_NOTE 4
#define PT_SHLIB 5
#define PT_PHDR 6
#define PT_LOPROC 0x70000000
#define PT_HIPROC 0x7fffffff


/*
 * ELF header at the beginning of the executable.
 */
typedef struct {
    unsigned  char	ident[16];
    unsigned  short	type;
    unsigned  short	machine;
    unsigned  int	version;
    unsigned  int	entry;
    unsigned  int	phoff;
    unsigned  int	sphoff;
    unsigned  int	flags;
    unsigned  short	ehsize;
    unsigned  short	phentsize;
    unsigned  short	phnum;
    unsigned  short	shentsize;
    unsigned  short	shnum;
    unsigned  short	shstrndx;
} elfHeader;

/*
 * An entry in the ELF program header table.
 * This describes a single segment of the executable.
 */
typedef struct {
    unsigned  int   type;
    unsigned  int   offset;
    unsigned  int   vaddr;
    unsigned  int   paddr;
    unsigned  int   fileSize;
    unsigned  int   memSize;
    unsigned  int   flags;
    unsigned  int   alignment;
} programHeader;

/*
 * Bits in flags field of programHeader.
 * These describe memory permissions required by the segment.
 */
#define PF_R	0x4	 /* Pages of segment are readable. */
#define PF_W	0x2	 /* Pages of segment are writable. */
#define PF_X	0x1	 /* Pages of segment are executable. */

/*
 * A segment of an executable.
 * It specifies a region of the executable file to be loaded
 * into memory.
 */
struct Exe_Segment {
    u64int offsetInFile;	 /* Offset of segment in executable file */
    u64int lengthInFile;	 /* Length of segment data in executable file */
    u64int startAddress;	 /* Start address of segment in user memory */
    u64int sizeInMemory;	 /* Size of segment in memory */
    int protFlags;		 /* VM protection flags; combination of VM_READ,VM_WRITE,VM_EXEC */
};

/*
 * Maximum number of executable segments we allow.
 * Normally, we only need a code segment and a data segment.
 * Recent versions of gcc (3.2.3) seem to produce 3 segments.
 */
#define EXE_MAX_SEGMENTS 3

/*
 * A struct concisely representing all information needed to
 * load an execute an executable.
 */
struct Exe_Format {
    struct Exe_Segment segmentList[EXE_MAX_SEGMENTS]; /* Definition of segments */
    int numSegments;		/* Number of segments contained in the executable */
    u64int entryAddr;	 	/* Code entry point address */
};


void ELF_exec(u8int* data, int argc, char** argv, FileObject* stdin, FileObject* stdout, FileObject* stderr);
#endif  /* GEEKOS_ELF_H */
