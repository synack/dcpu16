/* Copyright 2012, Brian Swetland.  BSD Licensed.  Share and Enjoy! */
/* A DCPU-16 Assembler */

/* DCPU-16 Spec is Copyright 2012 Mojang */
/* http://0x10c.com/doc/dcpu-16.txt */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>

typedef uint16_t u16;
typedef uint32_t u32;

static u16 image[65536] = { 0, };
static u16 PC = 0;
static FILE *fin;
static const char *filename = "";
static int linenumber = 0;

static char linebuffer[128] = { 0, };
static char *lineptr = linebuffer;
static int token;
static char tstring[128];
static u16 tnumber;

void die(const char *fmt, ...) {
	va_list ap;
	fprintf(stderr,"%s:%d: ", filename, linenumber);
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	fprintf(stderr,"\n");
	if (linebuffer[0])
		fprintf(stderr,"%s:%d: >> %s <<\n", filename, linenumber, linebuffer);
	exit(1);
}

struct fixup {
	struct fixup *next;
	struct label *label;
	u16 pc;
};

struct label {
	struct label *next;
	u16 pc;
	u16 defined;
	char name[1];
};

struct label *labels = 0;
struct fixup *fixups = 0;

struct label *mklabel(const char *name, u16 pc, u16 def) {
	struct label *l;
	for (l = labels; l; l = l->next) {
		if (!strcasecmp(name, l->name)) {
			if (def) {
				if (l->defined)
					die("cannot redefine label: %s", name);
				l->defined = def;
				l->pc = pc;
			}
			return l;
		}
	}
	l = malloc(sizeof(*l) + strlen(name));
	l->defined = def;
	l->pc = pc;
	strcpy(l->name, name);
	l->next = labels;
	labels = l;
	return l;
}

void use_label(const char *name, u16 pc) {
	struct label *l = mklabel(name, 0, 0);
	if (l->defined) {
		image[pc] = l->pc;
	} else {
		struct fixup *f = malloc(sizeof(*f));
		f->next = fixups;
		f->pc = pc;
		f->label = l;
		fixups = f;
	}	
}

void set_label(const char *name, u16 pc) {
	mklabel(name, pc, 1);
}

void resolve_fixups(void) {
	struct fixup *f;
	for (f = fixups; f; f = f->next) {
		if (f->label->defined) {
			image[f->pc] = f->label->pc;
		} else {
			die("undefined reference to '%s' at 0x%04x", f->label->name, f->pc);
		}
	}
}

enum tokens {
	tA, tB, tC, tX, tY, tZ, tI, tJ,
	tXXX, tSET, tADD, tSUB, tMUL, tDIV, tMOD, tSHL,
	tSHR, tAND, tBOR, tXOR, tIFE, tIFN, tIFG, tIFB,
	tJSR,
	tPOP, tPEEK, tPUSH, tSP, tPC, tO,
	tWORD,
	tCOMMA, tOBRACK, tCBRACK, tCOLON,
	tSTRING, tNUMBER, tEOF,
};
static const char *tnames[] = {
	"A", "B", "C", "X", "Y", "Z", "I", "J",
	"XXX", "SET", "ADD", "SUB", "MUL", "DIV", "MOD", "SHL",
	"SHR", "AND", "BOR", "XOR", "IFE", "IFN", "IFG", "IFB",
	"JSR",
	"POP", "PEEK", "PUSH", "SP", "PC", "O",
	"WORD",
	",", "[", "]", ":",
	"<STRING>", "<NUMBER>", "<EOF>",
};
static const char *rnames[] = {
	"A", "B", "C", "X", "Y", "Z", "I", "J",
};

#define LASTKEYWORD	tWORD

int _next(void) {
	char c;
nextline:
	if (!*lineptr) {
		if (feof(fin)) return tEOF;
		if (fgets(linebuffer, 128, fin) == 0) return tEOF;
		lineptr = linebuffer;
		linenumber++;
	}
	while (*lineptr <= ' ') {
		if (*lineptr == 0) goto nextline;
		lineptr++;
	}
	switch ((c = *lineptr++)) {
	case ',': return tCOMMA;
	case '[': return tOBRACK;
	case ']': return tCBRACK;
	case ':': return tCOLON;
	case '/': case ';': *lineptr = 0; goto nextline;
	default:
		if (isdigit(c) ||
			(((c == '+') || (c == '-')) && isdigit(*lineptr))) {
			tnumber = strtoul(lineptr-1, &lineptr, 0);
			return tNUMBER;
		}
		if (isalpha(c)) {
			int n;
			char *x = tstring;
			lineptr--;
			while (isalnum(*lineptr))
				*x++ = tolower(*lineptr++);
			*x = 0;
			for (n = 0; n <= LASTKEYWORD; n++)
				if (!strcasecmp(tnames[n], tstring))
					return n;
			return tSTRING;
		}
		die("illegal character '%c'", c);
		return tEOF;
	}
}

int next(void) {
	token = _next();
	//fprintf(stderr,"%3d %s\n", token, tnames[token]);
	return token;
}

void expect(int t) {
	if (next() != t)
		die("expecting %s, found %s", tnames[t], tnames[token]);
}

void assemble_imm_or_label(void) {
	next();
	if (token == tNUMBER) {
		image[PC++] = tnumber;
	} else if (token == tSTRING) {
		image[PC] = 0;
		use_label(tstring, PC++);
	} else {
		die("expected number or label");
	}
}

int assemble_operand(void) {
	int n;
	next();
	switch (token) {
	case tA: case tB: case tC: case tX:
	case tY: case tZ: case tI: case tJ:
		return token & 7;
	case tPOP: return 0x18;
	case tPEEK: return 0x19;
	case tPUSH: return 0x1a;
	case tSP: return 0x1b;
	case tPC: return 0x1c;
	case tO: return 0x1d;
	case tNUMBER:
		image[PC++] = tnumber;
		return 0x1f;
	case tSTRING:
		image[PC] = 0;
		use_label(tstring, PC++);
		return 0x1f;
	default:
		if (token != tOBRACK)
			die("expected [");
	}
	next();
	switch (token) {
	case tA: case tB: case tC: case tX:
	case tY: case tZ: case tI: case tJ:
		n = token & 7;
		next();
		if (token == tCBRACK) {
			return n | 0x08;
		} else if (token == tCOMMA) {
			assemble_imm_or_label();
			expect(tCBRACK);
			return n | 0x10;
		} else {
			die("invalid operand");
		}
	case tSTRING:
		use_label(tstring, PC);
	case tNUMBER:
		image[PC++] = tnumber;
		next();
		if (token == tCBRACK) {
			return 0x1e;
		} else if ((token >= tA) && (token <= tJ)) {
			return 0x10 | (token & 7);
		} else {
			die("invalid operand");
		}
	default:
		die("invalid operand");
	}
	return 0;
}

void assemble_binop(int op) {
	int pc = PC++;
	int a, b;
	a = assemble_operand();
	expect(tCOMMA);
	b = assemble_operand();
	image[pc] = op | (a << 4) | (b << 10);
}

void assemble(const char *fn) {
	fin = fopen(fn, "r");
	filename = fn;
	linenumber = 0;
	if (!fin) die("cannot read file");

	for (;;) {
		next();
		switch (token) {
		case tEOF:
			goto done;
		case tCOLON:
			expect(tSTRING);
			set_label(tstring, PC);
			continue;
		case tWORD:
			assemble_imm_or_label();
			continue;
		case tSET: case tADD: case tSUB: case tMUL:
		case tDIV: case tMOD: case tSHL: case tSHR:
		case tAND: case tBOR: case tXOR: case tIFE:
		case tIFN: case tIFG: case tIFB:
			assemble_binop(token - tXXX); 
			continue;
		default:
			die("unexpected: %s", tnames[token]);
		}
	}
done:
	fclose(fin);
}

void emit(const char *fn) {
	FILE *fp;
	int n;
	filename = fn;
	linenumber = 0;
	fp = fopen(fn, "w");
	if (!fp) die("cannot write file");
	for (n = 0; n < PC; n++)
		fprintf(fp, "%04x\n", image[n]);
	fclose(fp);
}

int main(int argc, char **argv) {
	const char *outfn = "out.hex";

	while (argc > 1) {
		argc--;
		argv++;
		if (argv[0][0] == '-') {
			if (!strcmp(argv[0],"-o")) {
				if (argc > 1) {
					outfn = argv[1];
					argc--;
					argv++;
					continue;
				}
			}
			die("unknown option: %s", argv[0]);
		}
		assemble(argv[0]);
	}

	linebuffer[0] = 0;
	resolve_fixups();
	emit(outfn);
	return 0;
}

