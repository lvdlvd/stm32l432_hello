/*
  This file assumes FLASHROM, CODE, SRAM1 and SRAM2 have been defined before inclusion.
*/ 

ENTRY(Reset_Handler)
EXTERN(vector_table)  /* force inclusion of vector.c::vector_table */

_estack = ORIGIN(SRAM2) + LENGTH(SRAM2); /* top of the stack */

SECTIONS
{

  .text :
  {
    . = ALIGN(8);

    PROVIDE (isr_vector_table_base = .);
    KEEP(*(.isr_vector)) /* Startup code */

    . = ALIGN(8);

    *(.text)
    *(.text.*)
    *(.rodata)       
    *(.rodata.*)

    . = ALIGN(8);

  }  >CODE AT> FLASHROM

  .textimage (NOLOAD) :
  {
    . = . + SIZEOF(.text);
    . = ALIGN(8);
  } >SRAM1

  .data :
  {
    . = ALIGN(8);

    *(.data)
    *(SORT_BY_ALIGNMENT(.data.*))

    /* align config on a flash page */
    . = ALIGN(2048);
    KEEP(*(.config))

    . = ALIGN(8);
  
  } >SRAM1 AT> FLASHROM

  _imgsrc  = LOADADDR(.text);
  _imgdst  = ORIGIN(SRAM1);
  _imgsize = SIZEOF(.text) + SIZEOF(.data);

  .bss :
  {
    . = ALIGN(8);

    *(.bss)
    *(SORT_BY_ALIGNMENT(.bss.*))
    *(COMMON)

    . = ALIGN(8);

  } >SRAM1

  _bssstart = ADDR(.bss);
  _bsssize  = SIZEOF(.bss);

  /* Remove information from the standard libraries */
  /DISCARD/ :
  {
    libc.a ( * )
    libm.a ( * )
    libgcc.a ( * )
    *(.ARM.exidx)     /* not removing this messes with the data segment initialisation */
    *(.ARM.exidx.*)   /* not removing this messes with the data segment initialisation */
  }

}
