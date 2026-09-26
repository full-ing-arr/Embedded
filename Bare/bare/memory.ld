ENTRY(app_main)

/* IRAM aliases DRAM at -0x6F0000; both regions remain below ROM download buffers at 0x3FCD7E00. */
MEMORY
{
  iram (rx) : ORIGIN = 0x40378000, LENGTH = 32K
  dram (rw) : ORIGIN = 0x3FC90000, LENGTH = 16K
}

SECTIONS
{
  .iram0.text : ALIGN(4)
  {
    *(.literal .literal.*)
    *(.text .text.*)
  } > iram

  .dram0.data : ALIGN(4)
  {
    *(.rodata .rodata.*)
    *(.data .data.*)
  } > dram

  .dram0.bss (NOLOAD) : ALIGN(4)
  {
    _bss_start = .;
    *(.bss .bss.*)
    *(COMMON)
    . = ALIGN(4);
    _bss_end = .;
  } > dram

  /DISCARD/ :
  {
    *(.eh_frame .eh_frame.*)
  }
}

/* ESP32-S3 ROM entries verified against ESP-IDF 6.0.1 esp32s3.rom.ld and the ROM ELF. */
ets_printf = 0x400005D0;
ets_install_uart_printf = 0x400005E8;
ets_delay_us = 0x40000600;
uart_tx_switch = 0x400006C0;
disable_default_watchdog = 0x40000714;
ets_get_cpu_frequency = 0x40001A40;
