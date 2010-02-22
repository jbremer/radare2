/* radare - LGPL - Copyright 2009 nibble<.ds@gmail.com> */

#include <r_types.h>
#include <r_util.h>
#include <r_bin.h>

R_API int r_bin_meta_get_line(RBin *bin, ut64 addr, char *file, int len, int *line) {
	if (bin && bin->cur && bin->cur->meta && bin->cur->meta->get_line)
		return bin->cur->meta->get_line (bin, addr, file, len, line);
	return R_FALSE;
}

R_API char *r_bin_meta_get_file_line(RBin *bin, const char *file, int line) {
	return r_file_slurp_line(file, line, 1);
}