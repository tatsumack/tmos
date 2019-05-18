/* tmos_app.ls */
OUTPUT_FORMAT("binary");

SECTIONS {
	.head 0x0 : {
		LONG(64 * 1024)         /* 0 : size(stack+.data+heap) */
		LONG(0x534f4d54)        /* 4 : "TMOS" */
		LONG(0)                 /* 8 : mmarea*/
		LONG(0x0400)            /* 12 : start address stack & .data */
		LONG(SIZEOF(.data))     /* 16 : size of .data */
		LONG(LOADADDR(.data))   /* 20 : size of .data */
		LONG(0xE9000000)        /* 24 : E9000000 */
		LONG(tmos_main - 0x20)  /* 28 : entry - 0x20 */
		LONG(24 * 1024)              /* 32 : start address of heap */
	}

	.text	: {*(.text)}

	.data 0x0400: AT ( ADDR(.text) + SIZEOF(.text) ) {
		*(.data)
		*(.rodata*)
		*(.bss)
	}

	/DISCARD/ : { *(.eh_frame) }
}
