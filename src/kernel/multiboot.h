#include <lang/lang.h>

typedef uint16_t multiboot_uint16_t;
typedef uint32_t multiboot_uint32_t;
typedef uint64_t multiboot_uint64_t;


 /* The symbol table for a.out. */
struct multiboot_aout_symbol_table_t
{
	multiboot_uint32_t tabsize;
	multiboot_uint32_t strsize;
	multiboot_uint32_t addr;
	multiboot_uint32_t reserved;
};

 /* The section header table for ELF. */
struct multiboot_elf_section_header_table_t
{
	multiboot_uint32_t num;
	multiboot_uint32_t size;
	multiboot_uint32_t addr;
	multiboot_uint32_t shndx;
};

struct multiboot_info_t
{
	 /* Multiboot info version number */
	multiboot_uint32_t flags;

	 /* Available memory from BIOS */
	multiboot_uint32_t mem_lower;
	multiboot_uint32_t mem_upper;

	 /* "root" partition */
	multiboot_uint32_t boot_device;

	 /* Kernel command line */
	multiboot_uint32_t cmdline;

	 /* Boot-Module list */
	multiboot_uint32_t mods_count;
	multiboot_uint32_t mods_addr;

	union
	{
		multiboot_aout_symbol_table_t aout_sym;
		multiboot_elf_section_header_table_t elf_sec;
	} u;

	 /* Memory Mapping buffer */
	multiboot_uint32_t mmap_length;
	multiboot_uint32_t mmap_addr;

	 /* Drive Info buffer */
	multiboot_uint32_t drives_length;
	multiboot_uint32_t drives_addr;

	 /* ROM configuration table */
	multiboot_uint32_t config_table;

	 /* Boot Loader Name */
	multiboot_uint32_t boot_loader_name;

	 /* APM table */
	multiboot_uint32_t apm_table;

	 /* Video */
	multiboot_uint32_t vbe_control_info;
	multiboot_uint32_t vbe_mode_info;
	multiboot_uint16_t vbe_mode;
	multiboot_uint16_t vbe_interface_seg;
	multiboot_uint16_t vbe_interface_off;
	multiboot_uint16_t vbe_interface_len;
};

extern "C" multiboot_info_t* multiboot_info;
 

struct multiboot_mmap_entry_t
{
	multiboot_uint32_t size;
	multiboot_uint64_t addr;
	multiboot_uint64_t len;
 #define MULTIBOOT_MEMORY_AVAILABLE              1
 #define MULTIBOOT_MEMORY_RESERVED               2
	multiboot_uint32_t type;
} __attribute__((packed));

struct multiboot_mod_list_t
{
	 /* the memory used goes from bytes 'mod_start' to 'mod_end-1' inclusive */
	multiboot_uint32_t mod_start;
	multiboot_uint32_t mod_end;

	 /* Module command line */
	multiboot_uint32_t cmdline;

	 /* padding to take it to 16 bytes (must be zero) */
	multiboot_uint32_t pad;
};