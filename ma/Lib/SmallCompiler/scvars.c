/*  Small compiler
 *
 *  Global (cross-module) variables.
 *
 *  Copyright (c) ITB CompuPhase, 1997-2002
 *  This file may be freely used. No warranties of any kind.
 *
 *  Version: $Id: Scvars.c,v 1.29 2002-02-13 11:53:04+01 thiadmer Exp thiadmer $
 */
#include <stdio.h>
#include <stdlib.h>     /* for _MAX_PATH */
#include "sc.h"

/*  global variables
 *
 *  All global variables that are shared amongst the compiler files are
 *  declared here.
 */
SC_VDEFINE symbol loctab;          /* local symbol table */
SC_VDEFINE symbol glbtab;          /* global symbol table */
SC_VDEFINE cell *litq;             /* the literal queue */
SC_VDEFINE char pline[sLINEMAX+1]; /* the line read from the input file */
SC_VDEFINE char *lptr;             /* points to the current position in "pline" */
SC_VDEFINE constval tagname_tab = { "", 0, NULL};  /* tagname table */
SC_VDEFINE constval libname_tab = { "", 0, NULL};  /* library table (#pragma library "..." syntax) */
SC_VDEFINE constval *curlibrary = NULL;/* current library */
SC_VDEFINE symbol *curfunc;        /* pointer to current function */
SC_VDEFINE char *inpfname;         /* pointer to name of the file currently read from */
SC_VDEFINE char outfname[_MAX_PATH];   /* output file name */
SC_VDEFINE char errfname[_MAX_PATH];   /* error file name */
SC_VDEFINE char includepath[_MAX_PATH];/* directory for system include files */

SC_VDEFINE char ctrlchar = CTRL_CHAR;  /* the control character (or escape character)*/
SC_VDEFINE int litidx    = 0;      /* index to literal table */
SC_VDEFINE int litmax    = sDEF_LITMAX;/* current size of the literal table */
SC_VDEFINE int stgidx    = 0;      /* index to the staging buffer */
SC_VDEFINE int labnum    = 0;      /* number of (internal) labels */
SC_VDEFINE int staging   = 0;      /* true if staging output */
SC_VDEFINE cell declared = 0;      /* number of local cells declared */
SC_VDEFINE cell glb_declared=0;    /* number of global bytes declared */
SC_VDEFINE cell code_idx = 0;      /* number of bytes with generated code */
SC_VDEFINE int ntv_funcid= 0;      /* incremental number of native function */
SC_VDEFINE int errnum    = 0;      /* number of errors */
SC_VDEFINE int warnnum   = 0;      /* number of warnings */
SC_VDEFINE int debug     = sCHKBOUNDS; /* by default: bounds checking+assertions */
SC_VDEFINE int charbits  = 8;      /* a "char" is 8 bits */
SC_VDEFINE int packstr   = FALSE;  /* strings are packed by default? */
SC_VDEFINE int sc_listing= FALSE;  /* create .ASM file? */
SC_VDEFINE int sc_compress=TRUE;   /* compress bytecode? */
SC_VDEFINE int needsemicolon=TRUE; /* semicolon required to terminate expressions? */
SC_VDEFINE int makereport=FALSE;   /* do not generate a cross-reference report */
SC_VDEFINE int curseg    = 0;      /* 1 if currently parsing CODE, 2 if parsing DATA */
SC_VDEFINE cell stksize  = sDEF_AMXSTACK;/* default stack size */
SC_VDEFINE int freading  = FALSE;  /* Is there an input file ready for reading? */
SC_VDEFINE int fline     = 0;      /* the line number in the current file */
SC_VDEFINE int fnumber   = 0;      /* the file number in the file table (debugging) */
SC_VDEFINE int fcurrent  = 0;      /* current file being processed (debugging) */
SC_VDEFINE int intest    = 0;      /* true if inside a test */
SC_VDEFINE int sideeffect= 0;      /* true if an expression causes a side-effect */
SC_VDEFINE int stmtindent= 0;      /* current indent of the statement */
SC_VDEFINE int indent_nowarn=FALSE;/* skip warning "217 loose indentation" */
SC_VDEFINE int sc_tabsize=8;       /* number of spaces that a TAB represents */
SC_VDEFINE int sc_allowtags=TRUE;  /* allow/detect tagnames in lex() */
SC_VDEFINE int sc_status;          /* read/write status */
SC_VDEFINE int sc_rationaltag=0;   /* tag for rational numbers */
SC_VDEFINE int rational_digits=0;  /* number of fractional digits */

SC_VDEFINE FILE *inpf    = NULL;   /* file read from (source or include) */
SC_VDEFINE FILE *inpf_org= NULL;   /* main source file */
SC_VDEFINE FILE *outf    = NULL;   /* file written to */

SC_VDEFINE jmp_buf errbuf;

#if defined __WATCOMC__ && !defined NDEBUG
  /* Watcom's CVPACK dislikes .OBJ files without functions */
  static int dummyfunc(void)
  {
    return 0;
  }
#endif
