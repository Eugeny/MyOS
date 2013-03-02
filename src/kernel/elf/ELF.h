#ifndef ELF_ELF_H
#define ELF_ELF_H

#include <lang/lang.h>
#include <fs/File.h>
#include <core/Process.h>

#define PT_NULL 0
#define PT_LOAD 1
#define PT_DYNAMIC 2
#define PT_INTERP 3
#define PT_NOTE 4
#define PT_SHLIB 5
#define PT_PHDR 6
#define PT_LOPROC 0x70000000
#define PT_HIPROC 0x7fffffff


#define Elf64_Addr      uint64_t
#define Elf64_Off       uint64_t
#define Elf64_Half      uint16_t
#define Elf64_Word      uint32_t
#define Elf64_Sword     uint32_t
#define Elf64_Xword     uint64_t
#define Elf64_Sxword    uint64_t

/*
 * ELF header at the beginning of the executable.
 */
struct elf_header_t {
    unsigned char e_ident[16]; /* ELF identification */
    Elf64_Half e_type; /* Object file type */
    Elf64_Half e_machine; /* Machine type */
    Elf64_Word e_version; /* Object file version */
    Elf64_Addr e_entry; /* Entry point address */
    Elf64_Off e_phoff; /* Program header offset */
    Elf64_Off e_shoff; /* Section header offset */
    Elf64_Word e_flags; /* Processor-specific flags */
    Elf64_Half e_ehsize; /* ELF header size */
    Elf64_Half e_phentsize; /* Size of program header entry */
    Elf64_Half e_phnum; /* Number of program header entries */
    Elf64_Half e_shentsize; /* Size of section header entry */
    Elf64_Half e_shnum; /* Number of section header entries */
    Elf64_Half e_shstrndx; /* Section name string table index */
};


/*
 * An entry in the ELF program header table.
 * This describes a single segment of the executable.
 */
struct elf_program_header_t {
    Elf64_Word p_type; /* Type of segment */
    Elf64_Word p_flags; /* Segment attributes */
    Elf64_Off p_offset; /* Offset in file */
    Elf64_Addr p_vaddr; /* Virtual address in memory */
    Elf64_Addr p_paddr; /* Reserved */
    Elf64_Xword p_filesz; /* Size of segment in file */
    Elf64_Xword p_memsz; /* Size of segment in memory */
    Elf64_Xword p_align; /* Alignment of segment */
};

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
struct elf_exe_segment_t {
    uint64_t offsetInFile;	 /* Offset of segment in executable file */
    uint64_t lengthInFile;	 /* Length of segment data in executable file */
    uint64_t startAddress;	 /* Start address of segment in user memory */
    uint64_t sizeInMemory;	 /* Size of segment in memory */
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
struct elf_exe_format_t {
    struct elf_exe_segment_t segmentList[EXE_MAX_SEGMENTS]; /* Definition of segments */
    int numSegments;		/* Number of segments contained in the executable */
    uint64_t entryAddr;	 	/* Code entry point address */
};



class ELF {
public:
    ELF();
    void loadFromFile(File* f);
    void loadIntoProcess(Process* p);
    uint64_t getEntryPoint();
private:
    uint8_t* data;
};


#endif 

