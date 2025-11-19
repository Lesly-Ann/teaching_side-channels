SECTIONS
{
  .data :
  {
    /* Place secret and public_arr in a specific order */
    . = ALIGN(8);
    __public_arr_start = .;
    KEEP(*(.data.public_arr))
    __public_arr_end = .;
    __secret_start = .;
    KEEP(*(.data.secret))
    __secret_end = .;

    *(.data)
    *(.data.rel*)
    *(.data.*)
  }
}

INSERT AFTER .data;
