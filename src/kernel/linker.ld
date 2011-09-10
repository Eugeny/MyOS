ENTRY (loader)

SECTIONS
{
	. = 0x100000;

	.text ALIGN(4096) :
	{
		*(.text*)
		*(.gnu.linkonce.t*)
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
	
    end = .; _end = .; __end = .;
}
