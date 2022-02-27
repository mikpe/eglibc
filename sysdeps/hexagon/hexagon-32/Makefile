# Needed to support `long double' via ieee754/ldbl-96
# Disabled for now, needs gcc help.
# long-double-fcts = yes

ifeq ($(subdir),elf)
CFLAGS-rtld.c += -Wno-uninitialized -Wno-unused
CFLAGS-dl-load.c += -Wno-unused
CFLAGS-dl-reloc.c += -Wno-unused
endif
