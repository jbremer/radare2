include ../config.mk

NAME=r_core

DEPS=r_config r_cons r_io r_cmd r_util r_print r_flags r_asm r_lib
DEPS+=r_debug r_hash r_bin r_lang r_io r_anal r_parse r_print r_bp r_egg
DEPS+=r_reg r_search r_syscall r_sign r_diff r_socket r_fs r_magic r_db

OBJS=core.o cmd.o file.o config.o visual.o io.o yank.o libs.o hack.o vasm.o
OBJS+=anal.o project.o gdiff.o asm.o rtr.o vmenus.o disasm.o patch.o bin.o log.o

CFLAGS+=-DLIBDIR=\"${LIBDIR}\"
CFLAGS+=-DPREFIX=\"${PREFIX}\"
CFLAGS+=-DWWWROOT=\"${WWWROOT}\"
LDFLAGS+=${DL_LIBS}

ifeq ($(shell uname),OpenBSD)
LDFLAGS+=-lpthread
endif

include $(TOP)/libr/rules.mk

cmd.o: cmd_hash.c cmd_debug.c cmd_zign.c cmd_section.c cmd_project.c \
	cmd_open.c cmd_meta.c cmd_macro.c cmd_magic.c cmd_mount.c \
	cmd_seek.c cmd_print.c cmd_help.c cmd_anal.c cmd_search.c \
	cmd_cmp.c cmd_write.c cmd_egg.c cmd_info.c cmd_type.c cmd_flag.c 
