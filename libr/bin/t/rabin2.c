/* radare - LGPL - Copyright 2009 nibble<.ds@gmail.com> */

/* TODO:
 * -l        Linked libraries
 * -L [lib]  dlopen library and show address
 * -z        Strings
 * -x        XRefs (-s/-i/-z required)
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include <r_types.h>
#include <r_lib.h>
#include <r_bin.h>
#include <r_flags.h>
#include <r_util.h>

#define ACTION_UNK       0x0000
#define ACTION_ENTRY     0x0001 
#define ACTION_IMPORTS   0x0002 
#define ACTION_SYMBOLS   0x0004 
#define ACTION_SECTIONS  0x0008 
#define ACTION_INFO      0x0010
#define ACTION_OPERATION 0x0020
#define ACTION_HELP      0x0040
#define ACTION_STRINGS   0x0080 

static struct r_lib_t l;
static struct r_bin_t bin;
static int verbose = 0;
static int rad = 0;

static int rabin_show_help()
{
	printf( "rabin2 [options] [file]\n"
			" -e          Entrypoint\n"
			" -i          Imports (symbols imported from libraries)\n"
			" -s          Symbols (exports)\n"
			" -S          Sections\n"
			" -z          Strings\n"
			" -I          Binary info\n"
			" -o [str]    Operation action (str=help for help)\n"
			" -f [format] Override file format autodetection\n"
			" -r          Radare output\n"
			" -h          This help\n"
			"Available plugins:\n");
	r_bin_list(&bin);

	return R_FALSE;
}

static int rabin_show_entrypoint()
{
	struct r_bin_entry_t *entry;
	u64 baddr;
	char *env;

	if (r_bin_open(&bin) == R_FALSE) {
		fprintf(stderr, "Cannot open file\n");
		return R_FALSE;
	}

	baddr = r_bin_get_baddr(&bin);
	if ((entry = r_bin_get_entry(&bin)) == NULL)
		return R_FALSE;

	if (rad) {
		env = getenv("DEBUG");
		if (env == NULL || (env && strncmp(env, "1", 1)))
			printf("e io.vaddr=0x%08llx\n", baddr);
		printf("fs symbols\n");
		printf("f entry @ 0x%08llx\n", baddr+entry->rva);
		printf("s entry\n");
	} else {
		printf("[Entrypoint]\n");
		printf("address=0x%08llx offset=0x%08llx baddr=0x%08llx\n",
				baddr+entry->rva, entry->offset, baddr);
	}

	r_bin_close(&bin);
	free(entry);

	return R_TRUE;
}

static int rabin_show_imports()
{
	int ctr = 0;
	u64 baddr;
	struct r_bin_import_t *imports, *importsp;

	if (r_bin_open(&bin) == R_FALSE) {
		fprintf(stderr, "Cannot open file\n");
		return R_FALSE;
	}

	baddr = r_bin_get_baddr(&bin);

	if ((imports = r_bin_get_imports(&bin)) == NULL)
		return R_FALSE;

	if (rad)
		printf("fs imports\n");
	else printf("[Imports]\n");

	importsp = imports;
	while (!importsp->last) {
		if (rad) {
			r_flag_name_filter(importsp->name);
			printf("f imp.%s @ 0x%08llx\n",
					importsp->name, baddr+importsp->rva);
		} else printf("address=0x%08llx offset=0x%08llx ordinal=%03lli "
				"hint=%03lli bind=%s type=%s name=%s\n",
				baddr+importsp->rva, importsp->offset,
				importsp->ordinal, importsp->hint,  importsp->bind,
				importsp->type, importsp->name);
		importsp++; ctr++;
	}

	if (!rad) printf("\n%i imports\n", ctr);

	r_bin_close(&bin);
	free(imports);

	return R_TRUE;
}

static int rabin_show_symbols()
{
	int ctr = 0;
	u64 baddr;
	struct r_bin_symbol_t *symbols, *symbolsp;

	if (r_bin_open(&bin) == R_FALSE) {
		fprintf(stderr, "Cannot open file\n");
		return R_FALSE;
	}

	baddr = r_bin_get_baddr(&bin);

	if ((symbols = r_bin_get_symbols(&bin)) == NULL)
		return R_FALSE;

	if (rad) printf("fs symbols\n");
	else printf("[Symbols]\n");

	symbolsp = symbols;
	while (!symbolsp->last) {
		if (rad) {
			r_flag_name_filter(symbolsp->name);
			if (symbolsp->size) {
				if (!strncmp(symbolsp->type,"FUNC", 4))
					printf("CF %lli @ 0x%08llx\n",
							symbolsp->size, baddr+symbolsp->rva);
				else
				if (!strncmp(symbolsp->type,"OBJECT", 6))
					printf("Cd %lli @ 0x%08llx\n",
							symbolsp->size, baddr+symbolsp->rva);
				printf("b %lli && ", symbolsp->size);
			}
			printf("f sym.%s @ 0x%08llx\n",
					symbolsp->name, baddr+symbolsp->rva);
		} else printf("address=0x%08llx offset=0x%08llx ordinal=%03lli "
				"forwarder=%s size=%08lli bind=%s type=%s name=%s\n",
				baddr+symbolsp->rva, symbolsp->offset,
				symbolsp->ordinal, symbolsp->forwarder,
				symbolsp->size, symbolsp->bind, symbolsp->type, 
				symbolsp->name);
		symbolsp++; ctr++;
	}

	if (!rad) printf("\n%i symbols\n", ctr);

	r_bin_close(&bin);
	free(symbols);

	return R_TRUE;
}

static int rabin_show_strings()
{
	int ctr = 0;
	u64 baddr;
	struct r_bin_string_t *strings, *stringsp;

	if (r_bin_open(&bin) == R_FALSE) {
		fprintf(stderr, "Cannot open file\n");
		return R_FALSE;
	}

	baddr = r_bin_get_baddr(&bin);

	if ((strings = r_bin_get_strings(&bin)) == NULL)
		return R_FALSE;

	if (rad)
		printf("fs strings\n");
	else printf("[strings]\n");

	stringsp = strings;
	while (!stringsp->last) {
		if (rad) {
			r_flag_name_filter(stringsp->string);
			printf("f str.%s @ 0x%08llx\n",
					stringsp->string, baddr+stringsp->rva);
		} else printf("address=0x%08llx offset=0x%08llx ordinal=%03lli "
				"string=%s\n",
				baddr+stringsp->rva, stringsp->offset,
				stringsp->ordinal, stringsp->string);
		stringsp++; ctr++;
	}

	if (!rad) printf("\n%i strings\n", ctr);

	r_bin_close(&bin);
	free(strings);

	return R_TRUE;
}

static int rabin_show_sections()
{
	int ctr = 0;
	u64 baddr;
	struct r_bin_section_t *sections, *sectionsp;

	if (r_bin_open(&bin) == R_FALSE) {
		fprintf(stderr, "Cannot open file\n");
		return R_FALSE;
	}

	baddr = r_bin_get_baddr(&bin);

	if ((sections = r_bin_get_sections(&bin)) == NULL)
		return R_FALSE;
	
	if (rad) printf("fs sections\n");
	else printf("[Sections]\n");

	sectionsp = sections;
	while (!sectionsp->last) {
		if (rad) {
			r_flag_name_filter(sectionsp->name);
			printf("f section.%s @ 0x%08llx\n",
					sectionsp->name, baddr+sectionsp->rva);
			printf("f section.%s_end @ 0x%08llx\n",
					sectionsp->name, baddr+sectionsp->rva+sectionsp->size);
			printf("[%02i] address=0x%08llx offset=0x%08llx size=%08lli "
					"privileges=%c%c%c%c name=%s\n",
					ctr, baddr+sectionsp->rva, sectionsp->offset, sectionsp->size,
					R_BIN_SCN_SHAREABLE(sectionsp->characteristics)?'s':'-',
					R_BIN_SCN_READABLE(sectionsp->characteristics)?'r':'-',
					R_BIN_SCN_WRITABLE(sectionsp->characteristics)?'w':'-',
					R_BIN_SCN_EXECUTABLE(sectionsp->characteristics)?'x':'-',
					sectionsp->name);
		} else printf("idx=%02i address=0x%08llx offset=0x%08llx size=%08lli "
				"privileges=%c%c%c%c name=%s\n",
				ctr, baddr+sectionsp->rva, sectionsp->offset, sectionsp->size,
				R_BIN_SCN_SHAREABLE(sectionsp->characteristics)?'s':'-',
				R_BIN_SCN_READABLE(sectionsp->characteristics)?'r':'-',
				R_BIN_SCN_WRITABLE(sectionsp->characteristics)?'w':'-',
				R_BIN_SCN_EXECUTABLE(sectionsp->characteristics)?'x':'-',
				sectionsp->name);
		sectionsp++; ctr++;
	}

	if (!rad) printf("\n%i sections\n", ctr);

	r_bin_close(&bin);
	free(sections);

	return R_TRUE;

}

static int rabin_show_info()
{
	struct r_bin_info_t *info;

	if (r_bin_open(&bin) == R_FALSE) {
		fprintf(stderr, "Cannot open file\n");
		return R_FALSE;
	}

	if ((info = r_bin_get_info(&bin)) == NULL)
		return R_FALSE;

	if (rad) {
		printf("e file.type=%s\n"
				"e cfg.bigendian=%s\n"
				"e asm.os=%s\n"
				"e asm.arch=%s\n"
				"e dbg.dwarf=%s\n",
				info->rclass, info->big_endian?"True":"False", info->os, info->arch,
				R_BIN_DBG_STRIPPED(info->dbg_info)?"False":"True");
	} else printf("[File info]\n"
			"Type=%s\n"
			"Class=%s\n"
			"Arch=%s\n"
			"Machine=%s\n"
			"OS=%s\n"
			"Subsystem=%s\n"
			"Big endian=%s\n"
			"Stripped=%s\n"
			"Static=%s\n"
			"Line_nums=%s\n"
			"Local_syms=%s\n"
			"Relocs=%s\n",
			info->type, info->class, info->arch, info->machine, info->os, 
			info->subsystem, info->big_endian?"True":"False",
			R_BIN_DBG_STRIPPED(info->dbg_info)?"True":"False",
			R_BIN_DBG_STATIC(info->dbg_info)?"True":"False",
			R_BIN_DBG_LINENUMS(info->dbg_info)?"True":"False",
			R_BIN_DBG_SYMS(info->dbg_info)?"True":"False",
			R_BIN_DBG_RELOCS(info->dbg_info)?"True":"False");

	r_bin_close(&bin);
	free(info);

	return R_TRUE;
}

static int rabin_do_operation(const char *op)
{
	char *arg, *ptr, *ptr2;

	if (!strcmp(op, "help")) {
		printf("Operation string:\n"
				"  Resize section: r/.data/1024 (ONLY ELF32)\n");
		return R_FALSE;
	}
	arg = alloca(strlen(op)+1);
	strcpy(arg, op);

	ptr = strchr(op, '/');
	if (!ptr) {
		printf("Unknown action. use -o help\n");
		return R_FALSE;
	}

	if (r_bin_open(&bin) == R_FALSE) {
		fprintf(stderr, "cannot open file\n");
		return R_FALSE;
	}

	ptr = ptr+1;
	switch(arg[0]) {
	case 'r':
		ptr2 = strchr(ptr, '/');
		ptr2[0]='\0';

		if (r_bin_resize_section(&bin, ptr, r_num_math(NULL,ptr2+1)) == 0) {
			fprintf(stderr, "Delta = 0\n");
			return R_FALSE;
		}
	}

	r_bin_close(&bin);

	return R_TRUE;
}

/* bin callback */
static int __lib_bin_cb(struct r_lib_plugin_t *pl, void *user, void *data)
{
	struct r_bin_handle_t *hand = (struct r_bin_handle_t *)data;
	//printf(" * Added (dis)assembly handler\n");
	r_bin_add(&bin, hand);
	return R_TRUE;
}
static int __lib_bin_dt(struct r_lib_plugin_t *pl, void *p, void *u) { return R_TRUE; }

int main(int argc, char **argv)
{
	int c;
	int action = ACTION_UNK, rw = 0;
	const char *file = NULL, *format = NULL;
	const char *op = NULL;

	r_bin_init(&bin);
	r_lib_init(&l, "radare_plugin");
	r_lib_add_handler(&l, R_LIB_TYPE_BIN, "bin plugins",
		&__lib_bin_cb, &__lib_bin_dt, NULL);
	r_lib_opendir(&l, getenv("LIBR_PLUGINS"));

	while ((c = getopt(argc, argv, "isSzIeo:f:rvh")) != -1)
	{
		switch( c ) {
		case 'i':
			action |= ACTION_IMPORTS;
			break;
		case 's':
			action |= ACTION_SYMBOLS;
			break;
		case 'S':
			action |= ACTION_SECTIONS;
			break;
		case 'z':
			action |= ACTION_STRINGS;
			break;
		case 'I':
			action |= ACTION_INFO;
			break;
		case 'e':
			action |= ACTION_ENTRY;
			break;
		case 'o':
			op = optarg;
			action |= ACTION_OPERATION;
			rw = 1;
			break;
		case 'f':
			format = optarg;
			break;
		case 'r':
			rad = 1;
			break;
		case 'v':
			verbose++;
			break;
		case 'h':
		default:
			action |= ACTION_HELP;
		}
	}
	
	file = argv[optind];
	
	if (action == ACTION_HELP || action == ACTION_UNK || file == NULL)
		return rabin_show_help();

	r_bin_set_file(&bin, file, rw);
	if (format) {
		char *str = malloc(strlen(format)+10);
		sprintf(str, "bin_%s", format);
		if (!r_bin_set(&bin, str)) {
			fprintf(stderr, "Unknown format\n");
			return R_FALSE;
		}
		free (str);
	} else {
		if (!r_bin_autoset(&bin)) {
			fprintf(stderr, "Not supported format\n");
			return R_FALSE;
		}
	}

	if (action&ACTION_ENTRY)
		rabin_show_entrypoint();
	if (action&ACTION_IMPORTS)
		rabin_show_imports();
	if (action&ACTION_SYMBOLS)
		rabin_show_symbols();
	if (action&ACTION_SECTIONS)
		rabin_show_sections();
	if (action&ACTION_STRINGS)
		rabin_show_strings();
	if (action&ACTION_INFO)
		rabin_show_info();
	if (op != NULL && action&ACTION_OPERATION)
		rabin_do_operation(op);

	return R_TRUE;
}
