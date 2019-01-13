OUTPUT(payload.elf);

ENTRY(_start);

SECTIONS {
	. = 0x00840000;
	.text : {		
		*(.text*);
	}
	.data : {
		*(.rodata*);
		*(.data*);
		*(.bss*);
	}
	/DISCARD/ : {
		*(*);
	}
}

ASSERT((SIZEOF(.text) + SIZEOF(.data)) < 0x400000, "elf is too big");
