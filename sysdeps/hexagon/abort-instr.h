
/* An op-code of 0 is guaranteed to be illegal.
 * Try ISDB breakpoint first.  */
#define ABORT_INSTRUCTION asm ("brkpt; .long 0")
