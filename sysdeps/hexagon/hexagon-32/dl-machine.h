/* Machine-dependent ELF dynamic relocation inline functions.  Hexagon-32 version.
   Copyright (C) 2010 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Linas Vepstas <linasvepstas@gmail.com>, 2010.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#ifndef dl_machine_h
#define dl_machine_h

#define ELF_MACHINE_NAME "hexagon"

#include <ldsodefs.h>
#include <libintl.h>
#include <link.h>
#include <string.h>
#include <tls.h>

#ifdef DEBUG
#define CDB_DEBUG _dl_debug_printf
#define CDB_DEBUG_VARX(X) _dl_debug_printf ("debug:" #X " = %x\n", ((unsigned int) (X)))
#else
#define CDB_DEBUG(X,...)
#define CDB_DEBUG_VARX(X)
#endif

/* Return the load-time base address of the GOT. */
#define LOAD_PIC_REG(g) \
{ \
  void * pc; \
  __asm__ \
    ("1:" /* Calculate the GOT address. */ \
     "{" \
     "%1 = pc;" /* P */ \
     "%0.h = #hi (1b@GOTOFF);" /* P - GOT */ \
     "};" \
     "%0.l = #lo (1b@GOTOFF);" \
     "%0 = sub (%1, %0);" /* P - (P - GOT) */ \
     : "=r" (g), "=r" (pc) : : "memory" ); \
}

/* Return the link-time address of _DYNAMIC.  Conveniently, this is
   stored as the first element of the GOT.  This must be inlined in
   a function which uses global data.  */
static inline Elf32_Addr
elf_machine_dynamic (void)
{
  register Elf32_Addr *got;

  LOAD_PIC_REG (got);

  return (*got);
}

/* Return the run-time load address of the shared object, assuming it
   was originally linked at zero.  */
static inline Elf32_Addr
elf_machine_load_address (void) __attribute__ ((const));

static inline Elf32_Addr
elf_machine_load_address (void)
{
  register Elf32_Addr addr, pc, off;

  /* This code computes the address at which the elf file was loaded
     by the kernel loader.

     This is done by taking the offset between the location of a label
     in the file image, before it is relocated, and the actual address
     at which it is found, obtained from the pc.

     We can't just use ".word ." for "right here", since that will
     put a R_HEXAGON_32 relocation in the read-only text segment, which
     will fault when written to.  So instead, do a ".word 0 -." to use
     the R_HEXAGON_32_PCREL *ABS*  relocation, which will not need to be
     relocated any further.
  */

  __asm__
    (/* Get the offset the image base and its run-time absolute address. */
     "jump 1f;"
     ".word 0 - .;"
     "1:"
     "%0 = pc;"
     "{"
     "%0 = add (%0, #-4);"
     "%1 = memw (%0 + #-4);" /* 0 - Offset */
     "}"
     : "=r" (pc), "=r" (off));

  /* Image base = PC + (0 - Offset)*/
  addr = pc + off;

  return (addr);
}

#ifdef THIS_WORKS_BUT_IS_SNARKY
static inline Elf32_Addr
elf_machine_load_address (void)
{
  register Elf32_Addr addr, got;
  /* The bogus section gets an address of zero, and so the image
   * offset is just (minus) the location of the got.  Add the got
   * back in, and that's our run-time load address.
   */
  __asm__(
    ".section bogus;"
    ".Lbogus:"
    ".word 0;"
    ".text;"
    "%0.h = #HI (.Lbogus@GOTOFF);"
    "%0.l = #LO (.Lbogus@GOTOFF);"
    : "=r" (addr) );

  LOAD_PIC_REG(got);
  addr += ((unsigned int) got);
  return addr;
}
#endif /* THIS_WORKS_BUT_IS_SNARKY */

#ifdef THIS_WORKS_BUT_IS_COMPLICATED
static inline Elf32_Addr
elf_machine_load_address (void)
{
  register Elf32_Addr dynoff, *got, addr;

  /* Get (minus) the image offset of _DYNAMIC. Note -- this only works
   * if the code runs before relocation is done, as otherwise the load
   * would get fixed up.
   */
  __asm__(
    "%0.h = #HI (_DYNAMIC@GOTOFF);"
    "%0.l = #LO (_DYNAMIC@GOTOFF);"
    : "=r" (dynoff));

  /* Now, fix it up by hand, by adding got. Now, *got just happens to
   * contain the image offset of _DYNAMIC, so subtracting this will
   * give "zero" i.e. the base address of the runtime.
   */
  LOAD_PIC_REG(got);
  addr = dynoff + ((unsigned int) got) - *got;
  return addr;
}
#endif /* THIS_WORKS_BUT_IS_COMPLICATED */

static inline Elf32_Ehdr *
elf_machine_elf_header_address (void)
{
  int i, phnum=0;
  Elf32_Addr base;
  const Elf32_Phdr *phdr=NULL;
#if defined(HAVE_AUX_VECTOR) && !defined(SHARED)
  long int argc, *argptr;
  Elf32_auxv_t *auxvec, *av;
#endif

  /* Base load adress of object */
  base = elf_machine_load_address();

  /* For statically-linked executables, we can find program headers
     in the auxvec.  In the shared case, this leads to confusion w.r.t.
     the base address.  On the other hand, dl_rtld_map doesn't exist
     in the non-shared case. */
#ifdef SHARED
  phdr = GL(dl_rtld_map).l_phdr;
  phnum = GL(dl_rtld_map).l_phnum;
#else
# ifdef HAVE_AUX_VECTOR
  argptr = __libc_stack_end;
  argc = *argptr;
  argptr += argc + 2;
  while (*argptr) argptr ++;
  auxvec = (Elf32_auxv_t *) (argptr + 1);

  /* Get the program header ... */
  for (av = auxvec; av->a_type != AT_NULL; av++)
    switch (av->a_type)
      {
      case AT_PHDR:
        phdr = (const Elf32_Phdr *) av->a_un.a_val;
        break;
      case AT_PHNUM:
        phnum = av->a_un.a_val;
        break;
      }
# endif /* HAVE_AUX_VECTOR */
#endif

  if (0 == phnum || NULL == phdr)
    return NULL;

  /* Presume the ELF header is the first thing in the file;
     so look for the program header with file offset zero. */
  for(i=0; i<phnum; i++)
    {
      if (0 == phdr->p_offset)
          return (Elf32_Ehdr *) (base + phdr->p_vaddr);
      phdr ++;
    }

  /* fail */
  return NULL;
}

/* Return nonzero if object ELF header is compatible with
   both the running host and the loaded program.

   A program can only refer to a library that uses a compatible ABI.
   V3 and V4 share the same calling convention, unlike V2.  Moreover,
   subsequent Hexagon hardware versions can run previous versions of
   user-mode code.  Therefore, it is prudent that the dynamic linker
   enforces such combinations:

   - A V2 program can only link to V2 dynamic libraries, but may
     run on V2, V3 or V4.
   - A V3 program can only link to V3 dynamic libraries, but may
     run on V3 or V4.
   - A V4 program can link to either V3 or V4 dynamic libraries,
     but can run on only V4.

   A V3 program could have V4 dynamic libraries in the same process,
   but at this moment it is not possible to confirm that the hardware
   can run V4 code.  It is also assumed that the kernel confirmed that
   a program can be run by the hardware before it was loaded.  */

static inline int
elf_machine_matches_host (const Elf32_Ehdr *libhdr)
{
  Elf32_Ehdr *prghdr;
  Elf32_Word prgver, libver;

  if (libhdr->e_machine != EM_HEXAGON)
    return 0;
  if (libhdr->e_ident[EI_CLASS] != ELFCLASS32)
    return 0;

  /* ELF header for the executable containing _start. */
  prghdr = elf_machine_elf_header_address();
  if (!prghdr)
    return 1;

  /* Machine versions for the program and the library. */
  prgver = prghdr->e_flags & 0xf;
  libver = libhdr->e_flags & 0xf;

  if (prgver != EF_HEXAGON_MACH_V2
      && prgver != EF_HEXAGON_MACH_V3
      && prgver != EF_HEXAGON_MACH_V4
      && prgver != EF_HEXAGON_MACH_V5)
    {
        _dl_error_printf(
          "Unknown Hexagon machine version %u\n", prgver+1);
        _dl_signal_error(ENOEXEC, NULL, NULL,
          N_("Unknown Hexagon machine version"));
        return 0;
    }

  if ((prgver == EF_HEXAGON_MACH_V2 && libver != EF_HEXAGON_MACH_V2)
      || (prgver == EF_HEXAGON_MACH_V3 && libver != EF_HEXAGON_MACH_V3)
      || (prgver == EF_HEXAGON_MACH_V4
	  && (libver < EF_HEXAGON_MACH_V3 || libver > EF_HEXAGON_MACH_V4))
      || (prgver == EF_HEXAGON_MACH_V5
	  && (libver < EF_HEXAGON_MACH_V3 || libver > EF_HEXAGON_MACH_V5)))
    {
        _dl_error_printf(
          "Hexagon library version %u is incompatible with executable version %u\n",
          libver+1, prgver+1);
        _dl_signal_error(ENOEXEC, NULL, NULL,
          N_("Library not compatible with executable"));
        return 0;
    }
  return 1;
}

/* Set up the loaded object described by L so its unrelocated PLT
   entries will jump to the on-demand fixup code in dl-runtime.c.  */

   static inline int __attribute__ ((always_inline))
elf_machine_runtime_setup (struct link_map *l, int lazy, int profile)
{
   Elf32_Addr *got;
   extern void _dl_runtime_resolve (Elf32_Word);
   extern void _dl_runtime_profile (Elf32_Word);
   CDB_DEBUG("elf_machine_runtime_setup(l=0x%x, lazy=0x%x, profile=0x%x)\n", l, lazy, profile);

   if (l->l_info[DT_JMPREL] && lazy)
   {
#define HEX_DL_RESOLVER  1
#define HEX_DL_LINK_MAP  2

      /* The GOT entries for functions in the PLT have not yet been
         filled in.  Initially, they will contain only a pointer to
         the resolver, in _GLOBAL_OFFSET_TABLE_[1], and a pointer to
         the link map, in _GLOBAL_OFFSET_TABLE_[2]. The run-time
         resolver will do the rest.  */
      got = (Elf32_Addr *) D_PTR (l, l_info[DT_PLTGOT]);

      /* Identify this shared object.  */
      got[HEX_DL_LINK_MAP] = (Elf32_Addr) l;

      /* The got[1] entry contains the address of a function which gets
         called to get the address of a so far unresolved function and
         jump to it.  The profiling extension of the dynamic linker allows
         to intercept the calls to collect information.  In this case we
         don't store the address in the GOT so that all future calls also
         end in this function.  */
      if (profile)
      {
         got[HEX_DL_RESOLVER] = (Elf32_Addr) &_dl_runtime_profile;

         if (_dl_name_match_p (GLRO(dl_profile), l))
         {
            /* This is the object we are looking for.  Say that we really
               want profiling and the timers are started.  */
            GL(dl_profile_map) = l;
         }
      }
      else
      {
         /* This function will get called to fix up the GOT entry indicated by
            the offset on the stack, and then jump to the resolved address.  */
         got[HEX_DL_RESOLVER] = (Elf32_Addr) &_dl_runtime_resolve;
      }
      if (GLRO(dl_debug_mask) & DL_DEBUG_SYMBOLS) {
         _dl_debug_printf (
            "elf_machine_runtime_setup: got=0x%x, got[1]=0x%x, got[2]=0x%x\n",
            (unsigned int) got, got[1], got[2]);
      }
   }

   return lazy;
}

/* Initial entry point code for the dynamic linker.
   The C function `_dl_start' is the real entry point;
   its return value is the user program's entry point.

   The incoming stack is assumed to look like this:
        sp -> argc  // 4 bytes
           argv[0]
           ...
           NULL
           envp[0]
           ...
           NULL

   Call _dl_start  which expects ptr to stack end as param.
   (this is passed through to _dl_sysdep_start as first arg)

   Call _dl_init_internal with these args:
        link_map *map,    // r0 _rtld_local
        int argc          // r1 _dl_argc
        char **argv,      // r2 _dl_argv
        char **env        // r3
 */

#define RTLD_START __asm__ ("\
        .text;\n\
        .p2align 2; \n\
        .global _start;\n\
        .type   _start, @function;\n\
     _start:\n\
        /* Pass the bottom of the stack as an argument. */\n\
        {\n\
           r0 = sp; \n\
           call _dl_start;\n\
        }\n\
        /* Save the user entry point address. */\n\
        r27 = r0; \n\
        .size _start, . - _start;\n\
        /* FALLTHRU */\n\
        \n\
        .globl _dl_start_user\n\
        .type _dl_start_user,@function\n\
     _dl_start_user:\n\
        /* Compute the GOT address, put it in r28. */\n\
        1: \n\
        { \n\
           r28 = pc; /* P */ \n\
           r10.h = #hi (1b@GOTOFF); /* P - GOT */ \n\
           r0.h = #hi (_rtld_local@GOTOFF); \n\
        } \n\
        { \n\
           r10.l = #lo (1b@GOTOFF); \n\
           r0.l = #lo (_rtld_local@GOTOFF); \n\
           r1 = memw(sp);            /* argc */ \n\
           r2 = add(sp, #4);         /* argv */ \n\
        } \n\
        /* Set up the args for _dl_init. */ \n\
        { \n\
           r28 = sub (r28, r10); /* P - (P - GOT) */ \n\
           r3 = addasl (r2, r1, #2); /* envp */ \n\
        } \n\
        { \n\
           r3 = add (r3, #4);        /* one past the null */ \n\
           r0 = add (r0, r28);       /* _rtld_local */ \n\
        } \n\
        { \n\
           r0 = memw(r0); \n\
           call _dl_init_internal@plt; \n\
        } \n\
        /* GOT address, again. r28 got clobbered somehow! */\n\
        2: \n\
        { \n\
           r28 = pc; /* P */ \n\
           r10.h = #hi (2b@GOTOFF); /* P - GOT */ \n\
           r6.h = #hi (_dl_skip_args@GOTOFF);\n\
        } \n\
        { \n\
           r10.l = #lo (2b@GOTOFF); \n\
           r6.l = #lo (_dl_skip_args@GOTOFF);\n\
        } \n\
        r28 = sub (r28, r10); /* P - (P - GOT) */ \n\
        r6 = add (r6, r28);\n\
        \n\
        /* Account for the filename as the 1st argument when\n\
         * invoked from the ld.so command-line. */\n\
        { \n\
           r6 = memw (r6);\n\
           /* Get original argument count. */\n\
           r7 = memw (sp);\n\
           r8.h = #hi (_dl_fini@GOTOFF);\n\
        } \n\
        { \n\
           /* Adjust stack to skip arguments. */\n\
           sp = addasl (sp, r6, #2);\n\
           /* Subtract skipped arguments from original argument count. */\n\
           r7 = sub (r7, r6);\n\
           r8.l = #lo (_dl_fini@GOTOFF);\n\
        } \n\
        { \n\
           /* Put new argument count on the top of the stack. */\n\
           memw (sp) = r7;\n\
           \n\
           /* Pass our finalizer function in r28, per ABI\n\
            * (rtld_fini in elf/start.S). */\n\
           r28 = add (r8, r28);\n\
           \n\
           /* Jump to the user's entry point. */\n\
           jumpr r27;\n\
        } \n\
        \n\
        .size _dl_start_user, . - _dl_start_user\n\
");

/* ELF_RTYPE_CLASS_PLT iff TYPE describes relocation of a PLT entry or
   TLS variable, so undefined references should not be allowed to
   define the value.
   ELF_RTYPE_CLASS_NOCOPY iff TYPE should not be allowed to resolve to one
   of the main executable's symbols, as for a COPY reloc.  */
#if !defined RTLD_BOOTSTRAP || USE___THREAD
# define elf_machine_type_class(type) \
  ((((type) == R_HEXAGON_JMP_SLOT ||  \
     (type) == R_HEXAGON_DTPMOD_32 || \
     (type) == R_HEXAGON_DTPREL_32 || \
     (type) == R_HEXAGON_TPREL_32) * \
    ELF_RTYPE_CLASS_PLT)              \
   | (((type) == R_HEXAGON_COPY) * ELF_RTYPE_CLASS_COPY))
#else
# define elf_machine_type_class(type) \
  ((((type) == R_HEXAGON_JMP_SLOT) * ELF_RTYPE_CLASS_PLT) \
   | (((type) == R_HEXAGON_COPY) * ELF_RTYPE_CLASS_COPY))
#endif

/* A reloc type used for ld.so cmdline arg lookups to reject PLT entries.  */
#define ELF_MACHINE_JMP_SLOT R_HEXAGON_JMP_SLOT

/* The Hexagon never uses Elf32_Rel relocations.  */
#define ELF_MACHINE_NO_REL 1

/* Fixup a PLT entry to bounce directly to the function at VALUE.  */
static inline Elf32_Addr
elf_machine_fixup_plt (struct link_map *map, lookup_t t,
      const Elf32_Rela *reloc,
      Elf32_Addr *reloc_addr, Elf32_Addr value)
{
   CDB_DEBUG("elf_machine_fixup_plt() entered\n");
   return *reloc_addr = value;
}

/* Return the final value of a plt relocation.  */
static inline Elf32_Addr
elf_machine_plt_value (struct link_map *map, const Elf32_Rela *reloc,
      Elf32_Addr value)
{
   CDB_DEBUG("elf_machine_plt_value() entered\n");
   return value + reloc->r_addend;
}

/* Names of the architecture-specific auditing callback functions.  */
#define ARCH_LA_PLTENTER hexagon_32_gnu_pltenter
#define ARCH_LA_PLTEXIT hexagon_32_gnu_pltexit

#endif /* dl_machine_h */

#ifdef RESOLVE_MAP

/* Perform the relocation specified by RELOC and SYM (which is fully resolved).
   MAP is the object containing the reloc.  */

auto inline void
__attribute__ ((always_inline))
elf_machine_rela (struct link_map *map, const Elf32_Rela *reloc,
      const Elf32_Sym *sym, const struct r_found_version *version,
		  void *const reloc_addr_arg, int skip_ifunc)
{
   Elf32_Addr *const reloc_addr = reloc_addr_arg;
   const unsigned int r_type = ELF32_R_TYPE (reloc->r_info);

   CDB_DEBUG("elf_machine_rela() entered\n");
   CDB_DEBUG("   map=0x%x\n", map);
   CDB_DEBUG("   reloc=0x%x\n", reloc);
   CDB_DEBUG("   sym=0x%x\n", sym);
   CDB_DEBUG("   version=0x%x\n", version);
   CDB_DEBUG("   reloc_addr_arg=0x%x\n", reloc_addr_arg);
   if (__builtin_expect (r_type == R_HEXAGON_RELATIVE, 0))
   {
      *reloc_addr = map->l_addr + reloc->r_addend;
      if (GLRO(dl_debug_mask) & DL_DEBUG_SYMBOLS) {
         _dl_debug_printf ("  R_HEXAGON_RELATIVE: reloc_addr=0x%x reloc_value=0x%x\n",
                           (unsigned int) reloc_addr, *reloc_addr);
      }
   }
   else
   {
      const Elf32_Sym *const refsym = sym;
      struct link_map *sym_map = RESOLVE_MAP (&sym, version, r_type);
      Elf32_Addr value = sym_map == NULL ? 0 : sym_map->l_addr + sym->st_value;
      CDB_DEBUG("   value=0x%x\n", value);
      CDB_DEBUG("   reloc->r_addend=0x%x\n", reloc->r_addend);

      switch (r_type)
      {
         case R_HEXAGON_NONE:
            if (GLRO(dl_debug_mask) & DL_DEBUG_SYMBOLS) {
               _dl_debug_printf ("    rtype=0x%x (R_HEXAGON_NONE)\n", r_type);
            }
            break;

         case R_HEXAGON_COPY:
            if (GLRO(dl_debug_mask) & DL_DEBUG_SYMBOLS) {
               _dl_debug_printf ("  R_HEXAGON_COPY: reloc_addr=0x%x  value=0x%x\n",
                                 (unsigned int) reloc_addr, value);
            }
            CDB_DEBUG("   rtype=0x%x (R_HEXAGON_COPY)\n", r_type);
            if (sym == NULL)
               /* This can happen in trace mode if an object could not be
                  found.  */
               break;
            if (sym->st_size > refsym->st_size ||
                (sym->st_size < refsym->st_size && GLRO(dl_verbose)))
            {
               const char *strtab;

               strtab = (const void *) D_PTR (map, l_info[DT_STRTAB]);
               _dl_error_printf (
                     "%s: Symbol `%s' has different size in shared object, consider re-linking\n",
                     rtld_progname ?: "<program name unknown>",
                     strtab + refsym->st_name);
            }
            memcpy (reloc_addr_arg, (void *) value,
                    MIN (sym->st_size, refsym->st_size));
            break;

         case R_HEXAGON_32:
            *reloc_addr = value + reloc->r_addend;
            if (GLRO(dl_debug_mask) & DL_DEBUG_SYMBOLS) {
               _dl_debug_printf ("  R_HEXAGON_32: reloc_addr=0x%x  reloc_value=0x%x\n",
                                 (unsigned int) reloc_addr, *reloc_addr);
            }
            break;
         case R_HEXAGON_GLOB_DAT:
            *reloc_addr = value + reloc->r_addend;
            if (GLRO(dl_debug_mask) & DL_DEBUG_SYMBOLS) {
               _dl_debug_printf ("  R_HEXAGON_GLOB_DAT: reloc_addr=0x%x reloc_value=0x%x\n",
                                 (unsigned int) reloc_addr, *reloc_addr);
            }
            break;
         case R_HEXAGON_JMP_SLOT:
            *reloc_addr = value + reloc->r_addend;
            if (GLRO(dl_debug_mask) & DL_DEBUG_SYMBOLS) {
               _dl_debug_printf ("  R_HEXAGON_JMP_SLOT: reloc_addr=0x%x  reloc_value=0x%x\n",
                                 (unsigned int) reloc_addr, *reloc_addr);
            }
            break;

#if !defined RTLD_BOOTSTRAP
         case R_HEXAGON_DTPMOD_32:
            /* Get the information from the link map returned by the
             * resolv function.  */
            if (sym_map != NULL)
               *reloc_addr = sym_map->l_tls_modid;
            break;

         case R_HEXAGON_DTPREL_32:
            *reloc_addr = sym->st_value + reloc->r_addend;
            break;

         case R_HEXAGON_TPREL_32:
            /* The offset is negative, down from the thread pointer. */
            CHECK_STATIC_TLS (map, sym_map);
            *reloc_addr = sym->st_value - sym_map->l_tls_offset
               + reloc->r_addend;
#endif
            break;
         default:
            if (GLRO(dl_debug_mask) & DL_DEBUG_SYMBOLS) {
               _dl_debug_printf ("    rtype=0x%x (UNKNOWN/BAD_TYPE)\n", r_type);
            }
            _dl_reloc_bad_type (map, r_type, 0);
            break;
      }
   }
}

auto inline void
__attribute__ ((always_inline))
elf_machine_rela_relative (Elf32_Addr l_addr, const Elf32_Rela *reloc,
      void *const reloc_addr_arg)
{
   CDB_DEBUG("elf_machine_rela_relative() entered\n");
   Elf32_Addr *const reloc_addr = reloc_addr_arg;
   CDB_DEBUG_VARX(reloc_addr);
   *reloc_addr = l_addr + reloc->r_addend;
   CDB_DEBUG_VARX(*reloc_addr);
}

auto inline void
__attribute__ ((always_inline))
elf_machine_lazy_rel (struct link_map *map,
                      Elf32_Addr l_addr, const Elf32_Rela *reloc, int skip_ifunc)
{
   Elf32_Addr *const reloc_addr = (void *) (l_addr + reloc->r_offset);
   unsigned long int const r_type = ELF32_R_TYPE (reloc->r_info);

   CDB_DEBUG("elf_machine_lazy_rel() entered\n");
   switch (r_type) {
      case R_HEXAGON_NONE:
         break;
      case R_HEXAGON_JMP_SLOT:
         *reloc_addr += l_addr;
         if (GLRO(dl_debug_mask) & DL_DEBUG_SYMBOLS) {
            _dl_debug_printf ("  lazy_rel: JMP_SLOT: reloc_addr=0x%x reloc_value=0x%x\n",
                              (unsigned int) reloc_addr, *reloc_addr);
         }
         break;
      default:
         _dl_reloc_bad_type(map, r_type, 1);
         break;
   }
}

#endif   /* RESOLVE_MAP */
