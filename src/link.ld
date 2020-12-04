OUTPUT(payload.elf);

ENTRY(_start);

SECTIONS {
	. = 0x00840000;
	.text : {
		*(.kernel_code*);   
		*(.text*);
		/* Tell linker to not garbage collect this section as it is not referenced anywhere */
		KEEP(*(.kernel_code*));
    }
	.data : {
		*(.rodata*);
		*(.data*);
		*(.sdata*);
		*(.bss*);
		*(.sbss*);
	}
    __CODE_END = .;
	/DISCARD/ : {
		*(*);
	}
}

ASSERT((SIZEOF(.text) + SIZEOF(.data)) < 0x400000, "elf is too big");
