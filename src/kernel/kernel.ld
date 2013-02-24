ENTRY (loader)

SECTIONS
{
	. = 0x1000000;

	.text ALIGN(4096) :
	{
		start_text = .; _start_text = .; __start_text = .;
		*(.text*)
		*(.gnu.linkonce.t*)
		end_text = .; _end_text = .; __end_text = .;
	}

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
		*(.COMMON*)
		*(.bss*)
		*(.gnu.linkonce.b*)
	}


	.rel.init       : { *(.rel.init) }
	.rel.text       : { *(.rel.text .rel.text.* .rel.gnu.linkonce.t.*) }
	.rel.fini       : { *(.rel.fini) }
	.rel.rodata     : { *(.rel.rodata .rel.rodata.* .rel.gnu.linkonce.r.*) }
	.rel.data.rel.ro   : { *(.rel.data.rel.ro* .rel.gnu.linkonce.d.rel.ro.*) }
	.rel.data       : { *(.rel.data .rel.data.* .rel.gnu.linkonce.d.*) }
	.rel.tdata      : { *(.rel.tdata .rel.tdata.* .rel.gnu.linkonce.td.*) }
	.rel.tbss       : { *(.rel.tbss .rel.tbss.* .rel.gnu.linkonce.tb.*) }
	.rel.ctors      : { *(.rel.ctors) }
	.rel.dtors      : { *(.rel.dtors) }
	.rel.got        : { *(.rel.got) }
	.rel.bss        : { *(.rel.bss .rel.bss.* .rel.gnu.linkonce.b.*) }
	.rel.ifunc      : { *(.rel.ifunc) }
	.rel.plt        :
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

		end = .; _end = .; __end = .;
}
