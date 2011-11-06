ENTRY (_myos_start)

SECTIONS
{
	. = 0x500000;

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
	    bss = .; _bss = .; __bss = .;
		*(.COMMON*)
		*(.bss*)
		*(.gnu.linkonce.b*)
	}

    end = .; _end = .; __end = .;
}
