/* Macros to support TLS testing; do what the compiler should have done.
 * In-lined by the elf/tls-macros.h file.
 */

#define CALL_CLOBBER_LIST					\
  "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",		\
    "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",	\
    "r28",							\
    "p0", "p1", "p2", "p3"

/* Hexagon Local Exec TLS access. */
#define TLS_LE(x)				\
  ({						\
    int *__result;				\
    int *tp;					\
    asm ("%0 = ugp \n\t"			\
	 "%1.h = #hi (" #x "@TPREL)\n\t"	\
	 "%1.l = #lo (" #x "@TPREL)\n\t"	\
	 "%1 = add (%1, %0)\n"			\
	 : "=r"(tp), "=r"(__result));		\
    __result; })

/* Hexagon Initial Exec TLS access. */
#ifdef PIC
#define TLS_IE(x)				\
  ({						\
    int *__result;				\
    int *tp, *pc, *got;				\
    asm ("1:\n\t"				\
	 "%2 = pc \n\t"				\
	 "%3.h = #hi (1b@GOTREL)\n\t"		\
	 "%3.l = #lo (1b@GOTREL)\n\t"		\
	 "%3 = sub (%2, %3)\n\t"		\
	 "%0 = ugp \n\t"			\
	 "%1.h = #hi (" #x "@IEGOT)\n\t"	\
	 "%1.l = #lo (" #x "@IEGOT)\n\t"	\
	 "%1 = add (%1, %3)\n\t"		\
	 "%1 = memw (%1)\n\t"			\
	 "%1 = add (%1, %0)\n\t"		\
	 : "=r"(tp), "=r"(__result), "=r"(pc),	\
	   "=r"(got));				\
    __result; })
#else
#define TLS_IE(x)				\
  ({						\
    int *__result;				\
    int *tp;					\
    asm ("%0 = ugp \n\t"			\
	 "%1.h = #hi (" #x "@IE)\n\t"		\
	 "%1.l = #lo (" #x "@IE)\n\t"		\
	 "%1 = memw (%1)\n\t"			\
	 "%1 = add (%1, %0)\n\t"		\
	 : "=r"(tp), "=r"(__result));		\
    __result; })
#endif

/* Hexagon Local Dynamic TLS access. */
#ifdef PIC
#define TLS_LD(x)				\
  ({						\
    int *__result;				\
    int *gotoff, *pc, *got, *tpoff;		\
    asm ("1:\n\t"				\
	 "%2 = pc \n\t"				\
	 "%3.h = #hi (1b@GOTREL)\n\t"		\
	 "%3.l = #lo (1b@GOTREL)\n\t"		\
	 "%3 = sub (%2, %3)\n\t"		\
	 "%1.h = #hi (" #x "@LDGOT)\n\t"	\
	 "%1.l = #lo (" #x "@LDGOT)\n\t"	\
	 "r0 = add (%1, %3)\n\t"		\
	 "call " #x "@LDPLT\n\t"		\
	 "%4.h = #hi (" #x "@DTPREL)\n\t"	\
	 "%4.l = #lo (" #x "@DTPREL)\n\t"	\
	 "%0 = add (%4, r0)\n\t"		\
	 : "=r"(__result),			\
	   "=r"(gotoff), "=r"(pc), "=r"(got),	\
	   "=r"(tpoff)				\
	 : : CALL_CLOBBER_LIST);		\
    __result; })
#else
#define TLS_LD(x)					\
  ({							\
    int *__result;					\
    int *gotoff, *got, *tpoff;				\
    asm ("%2.h = #hi (_GLOBAL_OFFSET_TABLE_)\n\t"	\
	 "%2.l = #lo (_GLOBAL_OFFSET_TABLE_)\n\t"	\
	 "%1.h = #hi (" #x "@LDGOT)\n\t"		\
	 "%1.l = #lo (" #x "@LDGOT)\n\t"		\
	 "r0 = add (%1, %2)\n\t"			\
	 "call " #x "@LDPLT\n\t"			\
	 "%3.h = #hi (" #x "@DTPREL)\n\t"		\
	 "%3.l = #lo (" #x "@DTPREL)\n\t"		\
	 "%0 = add (%3, r0)\n\t"			\
	 : "=r"(__result),				\
	   "=r"(gotoff), "=r"(got), "=r"(tpoff)		\
	 : : CALL_CLOBBER_LIST);			\
    __result; })
#endif

/* Hexagon General Dynamic TLS access. */
#ifdef PIC
#define TLS_GD(x)				\
  ({						\
    int *__result;				\
    int *gotoff, *pc, *got;			\
    asm ("1:\n\t"				\
	 "%2 = pc \n\t"				\
	 "%3.h = #hi (1b@GOTREL)\n\t"		\
	 "%3.l = #lo (1b@GOTREL)\n\t"		\
	 "%3 = sub (%2, %3)\n\t"		\
	 "%1.h = #hi (" #x "@GDGOT)\n\t"	\
	 "%1.l = #lo (" #x "@GDGOT)\n\t"	\
	 "r0 = add (%1, %3)\n\t"		\
	 "call " #x "@GDPLT\n\t"		\
	 "%0 = r0\n\t"				\
	 : "=r"(__result),			\
	   "=r"(gotoff), "=r"(pc), "=r"(got)	\
	 : : CALL_CLOBBER_LIST);		\
    __result; })
#else
#define TLS_GD(x)					\
  ({							\
    int *__result;					\
    int *gotoff, *got;					\
    asm ("%2.h = #hi (_GLOBAL_OFFSET_TABLE_)\n\t"	\
	 "%2.l = #lo (_GLOBAL_OFFSET_TABLE_)\n\t"	\
	 "%1.h = #hi (" #x "@GDGOT)\n\t"		\
	 "%1.l = #lo (" #x "@GDGOT)\n\t"		\
	 "r0 = add (%1, %2)\n\t"			\
	 "call " #x "@GDPLT\n\t"			\
	 "%0 = r0\n\t"					\
	 : "=r"(__result),				\
	   "=r"(got), "=r"(gotoff)			\
	 : : CALL_CLOBBER_LIST);			\
    __result; })
#endif
