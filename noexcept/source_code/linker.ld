/**
 * Currently set for an stm32f103c8 device change to whatever arm devive you
 * are using.
 */

__flash = 0x08000000;
__flash_size = 64K;
__ram = 0x20000000;
__ram_size = 10K;
__stack_size = 1K;

INCLUDE "third_party/standard_arm.ld"
