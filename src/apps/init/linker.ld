ENTRY (_myos_start)

SECTIONS
{
	. = 0x300000;

	.text ALIGN(4096) :
	{
		start_text = .; _start_text = .; __start_text = .;
		*(.text*)
		*(.gnu.linkonce.t*)
		end_text = .; _end_text = .; __end_text = .;
	}
/*
  .rela.fini     : { *(.rela.fini)	}
  .rel.bss       : { *(.rel.bss)		}
  .rela.bss      : { *(.rela.bss)		}
  .rel.plt       : { *(.rel.plt)		}
  .rela.plt      : { *(.rela.plt)		}
  .rel.plt      :
  {
    *(.rel.plt)
    PROVIDE_HIDDEN (__rel_iplt_start = .);
    *(.rel.iplt)
   PROVIDE_HIDDEN (__rel_iplt_end = .);
  }
  .rela.plt       :
  {
    *(.rela.plt)
    PROVIDE_HIDDEN (__rela_iplt_start = .);
    *(.rela.iplt)
    PROVIDE_HIDDEN (__rela_iplt_end = .);
  }
  .preinit_array     :
  {
    PROVIDE_HIDDEN (__preinit_array_start = .);
    KEEP (*(.preinit_array))
    PROVIDE_HIDDEN (__preinit_array_end = .);
  }
  .init_array     :
  {
    PROVIDE_HIDDEN (__init_array_start = .);
    KEEP (*(SORT_BY_INIT_PRIORITY(.init_array.*) SORT_BY_INIT_PRIORITY(.ctors.*)))
    KEEP (*(.init_array))
    KEEP (*(EXCLUDE_FILE (*crtbegin.o *crtbegin?.o *crtend.o *crtend?.o ) .ctors))
    PROVIDE_HIDDEN (__init_array_end = .);
  }
  .fini_array     :
  {
    PROVIDE_HIDDEN (__fini_array_start = .);
    KEEP (*(SORT_BY_INIT_PRIORITY(.fini_array.*) SORT_BY_INIT_PRIORITY(.dtors.*)))
    KEEP (*(.fini_array))
    KEEP (*(EXCLUDE_FILE (*crtbegin.o *crtbegin?.o *crtend.o *crtend?.o ) .dtors))
    PROVIDE_HIDDEN (__fini_array_end = .);
  }
*/


	.rodata ALIGN(4096) :
	{
		start_ctors = .; _start_ctors = .; __start_ctors = .;
		*(.ctor*)
		end_ctors = .; _end_ctors = .; __end_ctors = .;

		*(.rodata*)
		*(.gnu.linkonce.r*)
	}

	.data ALIGN(4096) :
	{
		*(.data*)
		*(.gnu.linkonce.d*)
	}

	.bss ALIGN(4096) :
	{
	    bss = .; _bss = .; __bss = .;
		*(.COMMON*)
		*(.bss*)
		*(.gnu.linkonce.b*)
	}

    end = .; _end = .; __end = .;
}
