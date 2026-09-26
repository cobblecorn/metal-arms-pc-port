/*  Small compiler
 *
 *  Function and variable definition and declaration, statement parser.
 *
 *  Copyright (c) ITB CompuPhase, 1997-2002
 *  This file may be freely used. No warranties of any kind.
 *
 *  Version: $Id: Sc1.c,v 1.51 2002-02-13 11:40:12+01 thiadmer Exp thiadmer $
 */
#include <assert.h>
#include <ctype.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined	__WIN32__ || defined _WIN32 || defined __MSDOS__
  #include <conio.h>
  #include <io.h>
#endif

#if defined LINUX
  #include <unistd.h>
  #include <sclinux.h>
#endif

#if defined __BORLANDC__ || defined __WATCOMC__
  #include <dos.h>
  static unsigned total_drives; /* dummy variable */
  #define dos_setdrive(i)       _dos_setdrive(i,&total_drives)
#elif defined _MSC_VER && defined _WIN32
  #include <direct.h>           /* for _chdrive() */
  #define dos_setdrive(i)       _chdrive(i)
#endif
#if defined __BORLANDC__
  #include <dir.h>              /* for chdir() */
#elif defined __WATCOMC__
  #include <direct.h>           /* for chdir() */
#endif
#if defined __WIN32__ || defined _WIN32 || defined _Windows
  #include <windows.h>
#endif

#include "sc.h"
#define VERSION_STR "1.8.3"
#define VERSION_INT 183

static void resetglobals(void);
static void initglobals(void);
static void setopt(int argc,char **argv,char *iname,char *oname,char *bname,
                   char *ename,char *pname,char *rname,char *incpath);
static void setconfig(char *root);
static void setcaption(void);
static void about(void);
static void setconstants(void);
static void parse(void);
static void dumplits(void);
static void dumpzero(int count);
static void declfuncvar(int tok,char *symname,int tag,int fpublic,int fstatic,int fstock,int fconst);
static void declglb(char *firstname,int firsttag,int fpublic,int fstatic,int stock,int fconst);
static int declloc(int fstatic);
static void decl_const(int table);
static void decl_enum(int table);
static cell needsub(int *tag);
static void initials(int ident,int tag,cell *size,int dim[],int numdim);
static cell initvector(int ident,int tag,cell size,int fillzero);
static cell init(int ident,int *tag);
static void funcstub(int native);
static int newfunc(char *firstname,int firsttag,int fpublic,int fstatic,int stock);
static int declargs(symbol *sym);
static void doarg(char *name,int ident,int offset,int tags[],int numtags,
                  int fpublic,int fconst,arginfo *arg);
static void dump_referrers(symbol *root,FILE *log);
static void reduce_referrers(symbol *root);
static int testsymbols(symbol *root,int level,int testlabs,int testconst);
static constval *find_constval_byval(constval *table,cell val);
static void statement(int *lastindent);
static void compound(void);
static void doexpr(int comma,int chkeffect,int allowarray,int *tag);
static void doassert(void);
static void doexit(void);
static void test(int label,int parens,int invert);
static void doif(void);
static void dowhile(void);
static void dodo(void);
static void dofor(void);
static void doswitch(void);
static void dogoto(void);
static void dolabel(void);
static symbol *fetchlab(char *name);
static void doreturn(void);
static void dobreak(void);
static void docont(void);
static void dosleep(void);
static void addwhile(int *ptr);
static void delwhile(void);
static int *readwhile(void);

static int lastst     = 0;      /* last executed statement type */
static int ncmp       = 0;      /* number of active (open) compound statements */
static int rettype    = 0;      /* the type that a "return" expression should have */
static int skipinput  = 0;      /* number of lines to skip from the first input file */
static int wq[wqTABSZ];         /* "while queue", internal stack for nested loops */
static int *wqptr;              /* pointer to next entry */
static char binfname[_MAX_PATH];/* binary file name */
#if defined	__WIN32__ || defined _WIN32 || defined _Windows
  static HWND hwndFinish = 0;
#endif

#if !defined NO_MAIN

#if defined __TURBOC__ && !defined __32BIT__
  extern unsigned int _stklen = 0x2000;
#endif

#if defined __BORLANDC__ || defined __WATCOMC__
  #pragma argsused
#endif

int main(int argc, char *argv[], char *env[])
{
  #if defined LINUX
    char argv0[_MAX_PATH];
    int i;

    strcpy(argv0,argv[0]);
    /* Linux stores the name of the program in argv[0], but not the path.
     * To adjust this, I store a string with the path in argv[0]. To do
     * so, one must first search for the PWD= setting in the environment.
     */
    for (i=0; strlen(env[i])>0; i++) {
      if (strncmp(env[i],"PWD=",4)==0) {
        sprintf(argv0,"%s/%s",env[i]+4,argv[0]);
        break;          /* no need to search further */
      } /* if */
    } /* for */
    argv[0]=argv0;      /* set location to new first parameter */
  #endif

  return sc_compile(argc,argv);
}

int sc_printf(const char *message,...)
{
  int ret;
  va_list argptr;

  va_start(argptr,message);
  ret=vprintf(message,argptr);
  va_end(argptr);

  return ret;
}

int sc_error(int number,char *message,char *filename,int firstline,int lastline,va_list argptr)
{
static char *prefix[3]={ "Error", "Fatal", "Warning" };

  if (number!=0) {
    char *pre;

    pre=prefix[number/100];
    if (firstline>=0)
      printf("%s(%d -- %d) %s [%03d]: ",filename,firstline,lastline,pre,number);
    else
      printf("%s(%d) %s [%03d]: ",filename,lastline,pre,number);
  } /* if */
  vprintf(message,argptr);
  fflush(stdout);
  return 0;
}

void *sc_opensrc(char *filename)
{
  return fopen(filename,"rt");
}

void sc_closesrc(void *handle)
{
  assert(handle!=NULL);
  fclose((FILE*)handle);
}

void sc_resetsrc(void *handle,void *position)
{
  assert(handle!=NULL);
  fsetpos((FILE*)handle,(fpos_t *)position);
}

char *sc_readsrc(void *handle,char *target,int maxchars)
{
  return fgets(target,maxchars,(FILE*)handle);
}

void *sc_getpossrc(void *handle)
{
  static fpos_t lastpos;  /* may need to have a LIFO stack of such positions */

  fgetpos((FILE*)handle,&lastpos);
  return &lastpos;
}

int sc_eofsrc(void *handle)
{
  return feof((FILE*)handle);
}

void *sc_openasm(char *filename)
{
  return fopen(filename,"w+t");
}

void sc_closeasm(void *handle, int deletefile)
{
  if (handle!=NULL)
    fclose((FILE*)handle);
  if (deletefile)
    unlink(outfname);
}

void sc_resetasm(void *handle)
{
  fflush((FILE*)handle);
  fseek((FILE*)handle,0,SEEK_SET);
}

int sc_writeasm(void *handle,char *st)
{
  return fputs(st,(FILE*)handle) >= 0;
}

char *sc_readasm(void *handle, char *target, int maxchars)
{
  return fgets(target,maxchars,(FILE*)handle);
}

void *sc_openbin(char *filename)
{
  return fopen(filename,"wb");
}

void sc_closebin(void *handle,int deletefile)
{
  fclose((FILE*)handle);
  if (deletefile)
    unlink(binfname);
}

void sc_resetbin(void *handle)
{
  fflush((FILE*)handle);
  fseek((FILE*)handle,0,SEEK_SET);
}

int sc_writebin(void *handle,void *buffer,int size)
{
  return (int)fwrite(buffer,1,size,(FILE*)handle) == size;
}

long sc_lengthbin(void *handle)
{
  return ftell((FILE*)handle);
}

#endif  /* !defined NO_MAIN */


/*  "main" of the compiler
 */
#if defined __cplusplus
  extern "C"
#endif
int sc_compile(int argc, char *argv[])
{
  int entry,i,jmpcode;
  int retcode;
  char incfname[_MAX_PATH];
  char reportname[_MAX_PATH];
  FILE *binf;
  void *inpfmark;

  /* set global variables to their initial value */
  binf=NULL;
  initglobals();
  errorset(sRESET);
  errorset(sEXPRRELEASE);
  lexinit();

	_nErrorCnt = 0;

  /* make sure that we clean up on a fatal error; do this before the first
   * call to error(). */
  if ((jmpcode=setjmp(errbuf))!=0)
    goto cleanup;

  /* allocate memory for fixed tables */
  inpfname=(char *)malloc(_MAX_PATH);
  litq=(cell *)malloc(litmax*sizeof(cell));
  if (litq==NULL)
    error(103);         /* insufficient memory */
  if (!phopt_init())
    error(103);         /* insufficient memory */

  setopt(argc,argv,inpfname,outfname,binfname,errfname,incfname,reportname,includepath);
  if (strlen(errfname)!=0)
    unlink(errfname);   /* delete file on startup */
  else
    setcaption();
  setconfig(argv[0]);   /* the path to the include files */
  inpf=inpf_org=(FILE*)sc_opensrc(inpfname);
  if (inpf==NULL)
    error(100,inpfname);
  freading=TRUE;
  outf=(FILE*)sc_openasm(outfname); /* first write to assembler file (may be temporary) */
  if (outf==NULL)
    error(101,outfname);
  /* immediately open the binary file, for other programs to check */
  if (sc_listing) {
    binf=NULL;
  } else {
    binf=(FILE*)sc_openbin(binfname);
    if (binf==NULL)
      error(101,binfname);
  } /* if */
  setconstants();               /* set predefined constants and tagnames */
  for (i=0; i<skipinput; i++)   /* skip lines in the input file */
    if (sc_readsrc(inpf,pline,sLINEMAX)!=NULL)
      fline++;                  /* keep line number up to date */
  skipinput=fline;
  /* do the first pass through the file */
  inpfmark=sc_getpossrc(inpf);
  sc_status=statFIRST;
  if (strlen(incfname)>0)
    plungefile(incfname);       /* parse implicit include file */
  preprocess();                 /* fetch first line */
  parse();                      /* process all input */

  /* second pass */
  sc_status=statWRITE;          /* set, to enable warnings */
  /* write a report, if requested */
  #if !defined SC_LIGHT
    if (makereport) {
      FILE *frep=stdout;
      if (strlen(reportname)>0)
        frep=fopen(reportname,"wt");
      if (frep!=NULL) {
        dump_referrers(&glbtab,frep);
        if (strlen(reportname)>0)
          fclose(frep);
      } /* if */
    } /* if */
  #endif
  /* reset "defined" flag of all functions and global variables */
  reduce_referrers(&glbtab);
  delete_symbols(&glbtab,0,TRUE,FALSE);
  resetglobals();
  errorset(sRESET);
  /* reset the source file */
  inpf=inpf_org;
  freading=TRUE;
  sc_resetsrc(inpf,inpfmark);   /* reset file position */
  fline=skipinput;              /* reset line number */
  lexinit();                    /* clear internal flags of lex() */
  sc_status=statWRITE;          /* allow to write --this variable was reset by resetglobals() */
  setfile(inpfname,fnumber);
  if (strlen(incfname)>0)
    plungefile(incfname);       /* parse implicit include file (again) */
  preprocess();                 /* fetch first line */
  parse();                      /* process all input */
  /* inpf is already closed when readline() attempts to pop of a file */
  writetrailer();               /* write remaining stuff */

  entry=testsymbols(&glbtab,0,TRUE,FALSE);  /* test for unused or undefined
                                             * functions and variables */
  if (!entry)
    error(13);                  /* no entry point (no public functions) */

cleanup:
  if (inpf!=NULL)               /* main source file is not closed, do it now */
    sc_closesrc(inpf);
  /* write the binary file (the file is already open) */
  if (!sc_listing && errnum==0 && jmpcode==0) {
    assert(binf!=NULL);
    sc_resetasm(outf);          /* flush and loop back, for reading */
    assemble(binf,outf);        /* assembler file is now input */
  } /* if */
  if (outf!=NULL)
    sc_closeasm(outf,!sc_listing);
  if (binf!=NULL)
    sc_closebin(binf,errnum!=0);

  if (inpfname!=NULL)
    free(inpfname);
  if (litq!=NULL)
    free(litq);
  phopt_cleanup();
  stgbuffer_cleanup();
  assert(jmpcode!=0 || loctab.next==NULL);/* on normal flow, local symbols
                                           * should already have been deleted */
  delete_symbols(&loctab,0,TRUE,TRUE);    /* delete local variables if not yet
                                           * done (i.e. on a fatal error) */
  delete_symbols(&glbtab,0,TRUE,TRUE);
  delete_consttable(&tagname_tab);
  delete_consttable(&libname_tab);
  delete_aliastable();
  if (errnum!=0){
    if (strlen(errfname)==0)
      sc_printf("\n%d Error%s.\n",errnum,(errnum>1) ? "s" : "");
    retcode=2;
  } else if (warnnum!=0){
    if (strlen(errfname)==0)
      sc_printf("\n%d Warning%s.\n",warnnum,(warnnum>1) ? "s" : "");
    retcode=1;
  } else {
    if (strlen(errfname)==0 && jmpcode==0)
      sc_printf("Done.\n");
    retcode=jmpcode;
  } /* if */
  #if defined	__WIN32__ || defined _WIN32 || defined _Windows
    if (IsWindow(hwndFinish))
      PostMessage(hwndFinish,RegisterWindowMessage("SCNotify"),retcode,0L);
  #endif
  return retcode;
}

#if defined __cplusplus
  extern "C"
#endif
int sc_addconstant(char *name,cell value,int tag)
{
  errorset(sFORCESET);  /* make sure error engine is silenced */
  sc_status=statIDLE;
  add_constant(name,value,sGLOBAL,tag);
  return 1;
}

#if defined __cplusplus
  extern "C"
#endif
int sc_addtag(char *name)
{
  cell val;
  constval *ptr;
  int last,tag;

  if (name==NULL) {
    /* no tagname was given, check for one */
    if (lex(&val,&name)!=tLABEL) {
      lexpush();
      return 0;         /* untagged */
    } /* if */
  } /* if */

  assert(strchr(name,':')==NULL); /* colon should already have been stripped */
  last=0;
  ptr=tagname_tab.next;
  while (ptr!=NULL) {
    tag=(int)(ptr->value & TAGMASK);
    if (strcmp(name,ptr->name)==0)
      return tag;       /* tagname is known, return its sequence number */
    tag &= (int)~FIXEDTAG;
    if (tag>last)
      last=tag;
    ptr=ptr->next;
  } /* while */

  /* tagname currently unknown, add it */
  tag=last+1;           /* guaranteed not to exist already */
  if (isupper(*name))
    tag |= (int)FIXEDTAG;
  append_constval(&tagname_tab,name,(cell)tag);
  return tag;
}

static void resetglobals(void)
{
  /* reset the subset of global variables that is modified by the first pass */
  curfunc=NULL;         /* pointer to current function */
  lastst=0;             /* last executed statement type */
  ncmp=0;               /* number of active (open) compound statements */
  rettype=0;            /* the type that a "return" expression should have */
  litidx=0;             /* index to literal table */
  stgidx=0;             /* index to the staging buffer */
  labnum=0;             /* number of (internal) labels */
  staging=0;            /* true if staging output */
  declared=0;           /* number of local cells declared */
  glb_declared=0;       /* number of global bytes declared */
  code_idx=0;           /* number of bytes with generated code */
  ntv_funcid=0;         /* incremental number of native function */
  curseg=0;             /* 1 if currently parsing CODE, 2 if parsing DATA */
  freading=FALSE;       /* no input file ready yet */
  fline=0;              /* the line number in the current file */
  fnumber=0;            /* the file number in the file table (debugging) */
  fcurrent=0;           /* current file being processed (debugging) */
  intest=0;             /* true if inside a test */
  sideeffect=0;         /* true if an expression causes a side-effect */
  stmtindent=0;         /* current indent of the statement */
  indent_nowarn=FALSE;  /* do not skip warning "217 loose indentation" */
  sc_allowtags=TRUE;    /* allow/detect tagnames */
  sc_status=statIDLE;
}

static void initglobals(void)
{
  resetglobals();

  sc_listing=FALSE;     /* do not create .ASM file */
  skipinput=0;          /* number of lines to skip from the first input file */
  ctrlchar=CTRL_CHAR;   /* the control character (or escape character) */
  litmax=sDEF_LITMAX;   /* current size of the literal table */
  errnum=0;             /* number of errors */
  warnnum=0;            /* number of warnings */
  debug=sCHKBOUNDS;     /* by default: bounds checking+assertions */
  charbits=8;           /* a "char" is 8 bits */
  packstr=FALSE;        /* strings are unpacked by default */
  sc_compress=TRUE;     /* compress output bytecodes */
  needsemicolon=FALSE;  /* semicolon required to terminate expressions? */
  makereport=FALSE;     /* do not generate a cross-reference report */
  stksize=sDEF_AMXSTACK;/* default stack size */
  sc_tabsize=8;         /* assume a TAB is 8 spaces */
  sc_rationaltag=0;     /* assume no support for rational numbers */
  rational_digits=0;    /* number of fractional digits */

  outfname[0]='\0';     /* output file name */
  errfname[0]='\0';     /* error file name */
  includepath[0]='\0';  /* directory for system include files */
  inpf=NULL;            /* file read from */
  inpfname=NULL;        /* pointer to name of the file currently read from */
  outf=NULL;            /* file written to */
  litq=NULL;            /* the literal queue */
  glbtab.next=NULL;     /* clear global variables/constants table */
  loctab.next=NULL;     /*   "   local      "    /    "       "   */
  tagname_tab.next=NULL;/* tagname table */
  libname_tab.next=NULL;/* library table (#pragma library "..." syntax) */

  pline[0]='\0';        /* the line read from the input file */
  lptr=NULL;            /* points to the current position in "pline" */
  curlibrary=NULL;      /* current library */
  inpf_org=NULL;        /* main source file */

  wqptr=wq;             /* initialize while queue pointer */
}

static int toggle_option(char *optptr, int option)
{
  switch (*(optptr+1)) {
  case '\0':
    option=!option;
    break;
  case '-':
    option=FALSE;
    break;
  case '+':
    option=TRUE;
    break;
  default:
    about();
  } /* switch */
  return option;
}

/* Parsing command line options is indirectly recursive: parseoptions()
 * calls parserespf() to handle options in a a response file and
 * parserespf() calls parseoptions() at its turn after having created
 * an "option list" from the contents of the file.
 */
static void parserespf(char *filename,char *iname,char *oname,
                       char *bname,char *ename,char *pname,char *rname,char *incpath);

static void parseoptions(int argc,char **argv,char *iname,char *oname,
                         char *bname,char *ename,char *pname,char *rname,
                         char *incpath)
{
  char str[_MAX_PATH],*ptr;
  int arg,i,isoption;

  for (arg=1; arg<argc; arg++) {
    #if DIRSEP_CHAR=='/'
      isoption= argv[arg][0]=='-';
    #else
      isoption= argv[arg][0]=='/' || argv[arg][0]=='-';
    #endif
    if (isoption) {
      ptr=&argv[arg][1];
      switch (*ptr) {
      case 'a':
        if (*(ptr+1)!='\0')
          about();
        sc_listing=TRUE;        /* skip last pass of making binary file */
        break;
      case 'C':
        sc_compress=toggle_option(ptr,sc_compress);
        break;
      case 'c':
        i=atoi(ptr+1);
        if (i==8 || i==16)
          charbits=i;           /* character size is 8 or 16 bits */
        else
          about();
        break;
#if defined dos_setdrive
      case 'D':                 /* set active directory */
        ptr++;
        if (ptr[1]==':')
          dos_setdrive(toupper(*ptr)-'A'+1);    /* set active drive */
        chdir(ptr);
        break;
#endif
      case 'd':
        switch (ptr[1]) {
        case '0':
          debug=0;
          break;
        case '1':
          debug=sCHKBOUNDS;     /* assertions and bounds checking */
          break;
        case '2':
          debug=sCHKBOUNDS | sSYMBOLIC; /* also symbolic info */
          break;
        case '3':
          debug=sCHKBOUNDS | sSYMBOLIC | sNOOPTIMIZE;
          /* also avoid peephole optimization */
          break;
        default:
          about();
        } /* switch */
        break;
      case 'e':
        strcpy(ename,ptr+1);    /* set name of error file */
        break;
#if defined	__WIN32__ || defined _WIN32 || defined _Windows
      case 'H':
        hwndFinish=(HWND)atoi(ptr+1);
        if (!IsWindow(hwndFinish))
          hwndFinish=(HWND)0;
        break;
#endif
      case 'i':
        strcpy(incpath,ptr+1);  /* set name of include directory */
        i=strlen(incpath);
        if (i>0 && incpath[i-1]!=DIRSEP_CHAR) {
          incpath[i]=DIRSEP_CHAR;
          incpath[i+1]='\0';
        } /* if */
        break;
      case 'o':
        strcpy(bname,ptr+1);    /* set name of binary output file */
        break;
      case 'P':
        packstr=toggle_option(ptr,packstr);
        break;
      case 'p':
        strcpy(pname,ptr+1);    /* set name of implicit include file */
        break;
#if !defined SC_LIGHT
      case 'r':
        strcpy(rname,ptr+1);    /* set name of report file */
        makereport=TRUE;
        break;
#endif
      case 'S':
        i=atoi(ptr+1);
        if (i>64)
          stksize=(cell)i;      /* stack size has minimum size */
        else
          about();
        break;
      case 's':
        skipinput=atoi(ptr+1);
        break;
      case 't':
        sc_tabsize=atoi(ptr+1);
        break;
      case '\\':                /* use \ instead of ^ for control characters */
        ctrlchar='\\';
        break;
      case ';':
        needsemicolon=toggle_option(ptr,needsemicolon);
        break;
      default:                  /* wrong option */
        about();
      } /* switch */
    } else if (argv[arg][0]=='@') {
      #if !defined SC_LIGHT
        parserespf(&argv[arg][1],iname,oname,bname,ename,pname,rname,incpath);
      #endif
    } else if ((ptr=strchr(argv[arg],'='))!=NULL) {
      i=(int)(ptr-argv[arg]);
      if (i>sNAMEMAX) {
        i=sNAMEMAX;
        error(200,argv[arg],sNAMEMAX);  /* symbol too long, truncated to sNAMEMAX chars */
      } /* if */
      strncpy(str,argv[arg],i);
      str[i]='\0';              /* str holds symbol name */
      i=atoi(ptr+1);
      add_constant(str,i,sGLOBAL,0);
    } else if (strlen(iname)>0) {
      about();
    } else {
      strcpy(str,argv[arg]);
      strcpy(iname,str);
      if ((ptr=strchr(str,'.'))==NULL)
        strcat(iname,".sma");
      else
        *ptr=0;   /* set zero terminator at the position of the period */
      /* The output name is the input name with the extension .ASM. The
       * binary file has the extension .AMX. */
      if ((ptr=strrchr(str,DIRSEP_CHAR))!=NULL)
        ptr++;          /* strip path */
      else
        ptr=str;
      strcpy(oname,ptr);
      strcat(oname,".asm");
      if (strlen(bname)==0) {
        strcpy(bname,ptr);
        strcat(bname,".amx");
      } /* if */
    } /* if */
  } /* for */
}

#if !defined SC_LIGHT
static void parserespf(char *filename,char *iname,char *oname,
                       char *bname,char *ename,char *pname,char *rname,
                       char *incpath)
{
#define MAX_OPTIONS     100
  FILE *fp;
  char *string, *ptr, **argv;
  int argc;
  long size;

  if ((fp=fopen(filename,"rt"))==NULL)
    error(100,filename);        /* error reading input file */
  /* load the complete file into memory */
  fseek(fp,0L,SEEK_END);
  size=ftell(fp);
  fseek(fp,0L,SEEK_SET);
  assert(size<INT_MAX);
  if ((string=(char *)malloc((int)size+1))==NULL)
    error(103);                 /* insufficient memory */
  /* fill with zeros; in MS-DOS, fread() may collapse CR/LF pairs to
   * a single '\n', so the string size may be smaller than the file
   * size. */
  memset(string,0,(int)size+1);
  fread(string,1,(int)size,fp);
  fclose(fp);
  /* allocate table for option pointers */
  if ((argv=(char **)malloc(MAX_OPTIONS*sizeof(char*)))==NULL)
    error(103);                 /* insufficient memory */
  /* fill the options table */
  ptr=strtok(string," \t\r\n");
  for (argc=1; argc<MAX_OPTIONS && ptr!=NULL; argc++) {
    /* note: the routine skips argv[0], for compatibility with main() */
    argv[argc]=ptr;
    ptr=strtok(NULL," \t\r\n");
  } /* for */
  if (ptr!=NULL)
    error(102,"option table");   /* table overflow */
  /* parse the option table */
  parseoptions(argc,argv,iname,oname,bname,ename,pname,rname,incpath);
  /* free allocated memory */
  free(argv);
  free(string);
}
#endif

static void setopt(int argc,char **argv,char *iname,char *oname,char *bname,
                   char *ename,char *pname,char *rname,char *incpath)
{
  *iname='\0';
  *bname='\0';
  *ename='\0';
  *pname='\0';
  *rname='\0';

  #if 0 /* needed to test with BoundsChecker for DOS (it does not pass
         * through arguments) */
    strcpy(iname,"test.sma");
    strcpy(oname,"test.asm");
    strcpy(bname,"test.amx");
  #endif

  #if !defined SC_LIGHT
    /* first parse a "config" file with default options */
    if (argv[0]!=NULL) {
      char cfgfile[_MAX_PATH];
      char *ext;
      strcpy(cfgfile,argv[0]);
      if ((ext=strrchr(cfgfile,'.'))!=NULL && strchr(ext,DIRSEP_CHAR)==NULL)
        *ext='\0';                /* strip .EXE extension */
      strcat(cfgfile,".cfg");
      if (access(cfgfile,4)==0)
        parserespf(cfgfile,iname,oname,bname,ename,pname,rname,incpath);
    } /* if */
  #endif
  parseoptions(argc,argv,iname,oname,bname,ename,pname,rname,incpath);
  if (strlen(iname)==0)
    about();
}

static void setconfig(char *root)
{
  char *ptr;
  int len;

  if (strlen(includepath)==0) {
    #if defined __WIN32__ || defined _WIN32
      GetModuleFileName(NULL,includepath,_MAX_PATH);
    #else
      if (root!=NULL)
        strcpy(includepath,root);       /* path + filename (hopefully) */
    #endif
    #if defined __MSDOS__
      /* strip the options (appended to the path + filename) */
      if ((ptr=strpbrk(includepath," \t/"))!=NULL)
        *ptr='\0';
    #endif
    /* terminate just behind last \ or : */
    if ((ptr=strrchr(includepath,DIRSEP_CHAR))!=NULL || (ptr=strchr(includepath,':'))!=NULL)
      *(ptr+1)='\0';
    else
      /* there was no terminating \ or : so the filename probably does not
       * contain the path */
      includepath[0]='\0';
    strcat(includepath,"include");
    len=strlen(includepath);
    includepath[len]=DIRSEP_CHAR;
    includepath[len+1]='\0';
  } /* if */
}

static int waitkey(void)
{
  #if defined __WIN32__ || defined _WIN32 || defined __MSDOS__
    int ch;

    sc_printf("\nPress the space bar to continue, or \"Esc\" to abort");
    fflush(stdout);
    do
      ch=getch();
    while (ch>0 && ch!=' ' && ch!='\x1b');
    sc_printf("\n\n");
    return ch==' ';
  #else
    return TRUE;
  #endif
}

static void setcaption(void)
{
  sc_printf("Small compiler " VERSION_STR "\t\tCopyright (c) 1997-2002, ITB CompuPhase\n\n");
}

static void about(void)
{
  if (strlen(errfname)==0) {
    setcaption();
    sc_printf("Usage:   sc <filename> [options]\n\n");
    sc_printf("Options:\n");
    sc_printf("         -a       output assembler code\n");
    sc_printf("         -C[+/-]  compact encoding for output file (default=%c)\n", sc_compress ? '+' : '-');
    sc_printf("         -c8      [default] a character is 8-bits (ASCII/ISO Latin-1)\n");
    sc_printf("         -c16     a character is 16-bits (Unicode)\n");
#if defined dos_setdrive
    sc_printf("         -Dpath   active directory path\n");
#endif
    sc_printf("         -d0      no symbolic information, no run-time checks\n");
    sc_printf("         -d1      [default] run-time checks, no symbolic information\n");
    sc_printf("         -d2      full debug information and dynamic checking\n");
    sc_printf("         -d3      full debug information, dynamic checking, no optimization\n");
    sc_printf("         -e<name> set name of error file (quiet compile)\n");
#if defined	__WIN32__ || defined _WIN32 || defined _Windows
    sc_printf("         -H<hwnd> window handle to send notification message to\n");
#endif
    sc_printf("         -i<name> path for include files\n");
    sc_printf("         -o<name> set name of binary output file\n");
    sc_printf("         -P[+/-]  strings are \"packed\" by default (default=%c)\n", packstr ? '+' : '-');
    sc_printf("         -p<name> set name of \"prefix\" file\n");
#if !defined SC_LIGHT
    sc_printf("         -r[name] write cross reference report to console or to specified file\n");
#endif
    sc_printf("         -S<num>  stack/heap size in cells (default=%d)\n",(int)stksize);
    if (!waitkey())
      longjmp(errbuf,3);
    sc_printf("         -s<num>  skip lines from the input file\n");
    sc_printf("         -t<num>  TAB indent size (in character positions)\n");
    sc_printf("         -\\       use '\\' instead of '^' for control characters\n");
    sc_printf("         -;[+/-]  require a semicolon to end each statement (default=%c)\n", needsemicolon ? '+' : '-');
    sc_printf("         sym=val  define constant \"sym\" with value \"val\"\n");
    sc_printf("         sym=     define constant \"sym\" with value 0\n");
  } /* if */
  longjmp(errbuf,3);        /* user abort */
}

static void setconstants(void)
{
  assert(sc_status==statIDLE);
  append_constval(&tagname_tab,"_",0);  /* "untagged" */
  append_constval(&tagname_tab,"bool",1);

  add_constant("true",1,sGLOBAL,1);     /* boolean flags */
  add_constant("false",0,sGLOBAL,1);
  #if defined(BIT16)
    add_constant("cellbits",16,sGLOBAL,0);
    add_constant("cellmax",SHRT_MAX,sGLOBAL,0);
    add_constant("cellmin",SHRT_MIN,sGLOBAL,0);
  #else
    add_constant("cellbits",32,sGLOBAL,0);
    add_constant("cellmax",LONG_MAX,sGLOBAL,0);
    add_constant("cellmin",LONG_MIN,sGLOBAL,0);
  #endif
  add_constant("charbits",charbits,sGLOBAL,0);
  add_constant("charmin",0,sGLOBAL,0);
  add_constant("charmax",(charbits==16) ? 0xffff : 0xff,sGLOBAL,0);

  add_constant("__Small",VERSION_INT,sGLOBAL,0);
}

/*  parse       - process all input text
 *
 *  At this level, only static declarations and function definitions are legal.
 */
static void parse(void)
{
  int tok,tag,fconst,fstock,fstatic;
  cell val;
  char *str;

  while (freading){
    /* first try whether a declaration possibly is native or public */
    tok=lex(&val,&str);  /* read in (new) token */
    switch (tok) {
    case 0:
      /* ignore zero's */
      break;
    case tNEW:
      fconst=matchtoken(tCONST);
      declglb(NULL,0,FALSE,FALSE,FALSE,fconst);
      break;
    case tSTATIC:
      /* This can be a static function or a static global variable; we know
       * which of the two as soon as we have parsed up to the point where an
       * opening paranthesis of a function would be expected. To back out after
       * deciding it was a declaration of a static variable after all, we have
       * to store the symbol name and tag.
       */
      fstock=matchtoken(tSTOCK);
      fconst=matchtoken(tCONST);
      tag=sc_addtag(NULL);
      tok=lex(&val,&str);
      if (tok==tNATIVE || tok==tPUBLIC) {
        error(42);              /* invalid combination of class specifiers */
        break;
      } /* if */
      declfuncvar(tok,str,tag,FALSE,TRUE,fstock,fconst);
      break;
    case tCONST:
      decl_const(sGLOBAL);
      break;
    case tENUM:
      decl_enum(sGLOBAL);
      break;
    case tPUBLIC:
      /* This can be a public function or a public variable; see the comment
       * above (for static functions/variables) for details.
       */
      fconst=matchtoken(tCONST);
      tag=sc_addtag(NULL);
      tok=lex(&val,&str);
      if (tok==tNATIVE || tok==tSTOCK || tok==tSTATIC) {
        error(42);              /* invalid combination of class specifiers */
        break;
      } /* if */
      declfuncvar(tok,str,tag,TRUE,FALSE,FALSE,fconst);
      break;
    case tSTOCK:
      /* This can be a stock function or a stock *global) variable; see the
       * comment above (for static functions/variables) for details.
       */
      fstatic=matchtoken(tSTATIC);
      fconst=matchtoken(tCONST);
      tag=sc_addtag(NULL);
      tok=lex(&val,&str);
      if (tok==tNATIVE || tok==tPUBLIC) {
        error(42);              /* invalid combination of class specifiers */
        break;
      } /* if */
      declfuncvar(tok,str,tag,FALSE,fstatic,TRUE,fconst);
      break;
    case tLABEL:
    case tSYMBOL:
    case tOPERATOR:
      lexpush();
      if (!newfunc(NULL,-1,FALSE,FALSE,FALSE))
        error(10);              /* illegal function or declaration */
      break;
    case tNATIVE:
      funcstub(TRUE);           /* create a dummy function */
      break;
    case tFORWARD:
      funcstub(FALSE);
      break;
    case '}':
      error(54);                /* unmatched closing brace */
      break;
    case '{':
      error(55);                /* start of function body without function header */
      break;
    default:
      if (freading)
        error(10);              /* illegal function or declaration */
    } /* switch */
  } /* while */
}

/*  dumplits
 *
 *  Dump the literal pool (strings etc.)
 *
 *  Global references: litidx (referred to only)
 */
static void dumplits(void)
{
  int j,k;

  k=0;
  while (k<litidx){
    /* should be in the data segment */
    assert(curseg==2);
    defstorage();
    j=16;       /* 16 values per line */
    while (j && k<litidx){
      outval(litq[k], FALSE);
      stgwrite(" ");
      k++;
      j--;
      if (j==0 || k>=litidx)
        stgwrite("\n");         /* force a newline after 10 dumps */
      /* Note: stgwrite() buffers a line until it is complete. It recognizes
       * the end of line as a sequence of "\n\0", so something like "\n\t"
       * so should not be passed to stgwrite().
       */
    } /* while */
  } /* while */
}

/*  dumpzero
 *
 *  Dump zero's for default initial values
 */
static void dumpzero(int count)
{
  int i;

  if (count<=0)
    return;
  assert(curseg==2);
  defstorage();
  i=0;
  while (count-- > 0) {
    outval(0, FALSE);
    i=(i+1) % 16;
    stgwrite((i==0 || count==0) ? "\n" : " ");
    if (i==0 && count>0)
      defstorage();
  } /* while */
}

static void declfuncvar(int tok,char *symname,int tag,int fpublic,int fstatic,int fstock,int fconst)
{
  char name[sNAMEMAX+1];

  if (tok!=tSYMBOL && tok!=tOPERATOR) {
    if (freading)
      error(20,symname);        /* invalid symbol name */
    return;
  } /* if */
  if (tok==tOPERATOR) {
    lexpush();
    if (!newfunc(NULL,tag,fpublic,fstatic,fstock))
      error(10);            /* illegal function or declaration */
  } else {
    assert(strlen(symname)<=sNAMEMAX);
    strcpy(name,symname);
    if (fconst || !newfunc(name,tag,fpublic,fstatic,fstock))
      declglb(name,tag,fpublic,fstatic,fstock,fconst);  /* if not a static function,
                                                         * try a static variable */
  } /* if */
}

/*  declglb     - declare global symbols
 *
 *  Declare a static (global) variable. Global variables are stored in
 *  the DATA segment.
 *
 *  global references: glb_declared     (altered)
 */
static void declglb(char *firstname,int firsttag,int fpublic,int fstatic,int stock,int fconst)
{
  int ident,tag,ispublic;
  int idxtag[sDIMEN_MAX];
  char name[sNAMEMAX+1];
  cell val,size,cidx;
  char *str;
  int dim[sDIMEN_MAX];
  int numdim,level;
  int filenum;
  symbol *sym;
  #if !defined NDEBUG
    cell glbdecl;
  #endif

  assert(!fpublic || !stock);           /* may not both be set */
  filenum=fcurrent;                     /* save file number at the start of the declaration */
  do {
    size=1;                             /* single size (no array) */
    numdim=0;                           /* no dimensions */
    ident=iVARIABLE;
    if (firstname!=NULL) {
      assert(strlen(firstname)<=sNAMEMAX);
      strcpy(name,firstname);           /* save symbol name */
      tag=firsttag;
      firstname=NULL;
    } else {
      tag=sc_addtag(NULL);
      if (lex(&val,&str)!=tSYMBOL)      /* read in (new) token */
        error(20,str);                  /* invalid symbol name */
      assert(strlen(str)<=sNAMEMAX);
      strcpy(name,str);                 /* save symbol name */
    } /* if */
    sym=findglb(name);
    if (sym==NULL)
      sym=findconst(name);
    if (sym!=NULL && (sym->usage & uDEFINE)!=0)
      error(21,name);                   /* symbol already defined */
    ispublic=fpublic;
    if (name[0]==PUBLIC_CHAR) {
      ispublic=TRUE;                    /* implicitly public variable */
      if (stock || fstatic)
        error(42);                      /* invalid combination of class specifiers */
    } /* if */
    while (matchtoken('[')) {
      ident=iARRAY;
      if (numdim == sDIMEN_MAX) {
        error(53);                      /* exceeding maximum number of dimensions */
        return;
      } /* if */
      if (numdim>0 && dim[numdim-1]==0)
        error(52);                      /* only last dimension may be variable length */
      size=needsub(&idxtag[numdim]);    /* get size; size==0 for "var[]" */
      #if INT_MAX < LONG_MAX
        if (size > INT_MAX)
          error(105);                   /* overflow, exceeding capacity */
      #endif
      dim[numdim++]=(int)size;
    } /* while */
    /* if this variable is never used (which can be detected only in the
     * second stage), shut off code generation; make an exception for public
     * variables
     */
    if (sc_status==statWRITE && sym!=NULL && (sym->usage & (uREAD | uWRITTEN | uPUBLIC))==0) {
      sc_status=statSKIP;
      cidx=code_idx;
      #if !defined NDEBUG
        glbdecl=glb_declared;
      #endif
    } /* if */
    defsymbol(name,ident,sGLOBAL,sizeof(cell)*glb_declared);
    begdseg();          /* real (initialized) data in data segment */
    litidx=0;           /* global initial data is dumped, so restart at zero */
    initials(ident,tag,&size,dim,numdim);/* stores values in the literal queue */
    assert(size>=litidx);
    if (numdim==1)
      dim[0]=(int)size;
    dumplits();         /* dump the literal queue */
    dumpzero((int)size-litidx);
    if (sym==NULL) {    /* define only if not yet defined */
      sym=addvariable(name,sizeof(cell)*glb_declared,ident,sGLOBAL,tag,
                      dim,numdim,idxtag);
    } else {            /* if declared but not yet defined, adjust the variable's address */
      sym->addr=sizeof(cell)*glb_declared;
      sym->usage|=uDEFINE;
    } /* if */
    if (ispublic)
      sym->usage|=uPUBLIC;
    if (fconst)
      sym->usage|=uCONST;
    if (stock)
      sym->usage|=uSTOCK;
    if (fstatic)
      sym->fnumber=filenum;
    if (ident==iARRAY)
      for (level=0; level<numdim; level++)
        symbolrange(level,dim[level]);
    if (sc_status==statSKIP) {
      sc_status=statWRITE;
      code_idx=cidx;
      assert(glb_declared==glbdecl);
    } else {
      glb_declared+=(int)size;  /* add total number of cells */
    } /* if */
  } while (matchtoken(',')); /* enddo */   /* more? */
  needtoken(tTERM);    /* if not comma, must be semicolumn */
}

/*  declloc     - declare local symbols
 *
 *  Declare local (automatic) variables. Since these variables are relative
 *  to the STACK, there is no switch to the DATA segment. These variables
 *  cannot be initialized either.
 *
 *  global references: declared   (altered)
 *                     funcstatus (referred to only)
 */
static int declloc(int fstatic)
{
  int ident,tag;
  int idxtag[sDIMEN_MAX];
  char name[sNAMEMAX+1];
  symbol *sym;
  cell val,size;
  char *str;
  value lval;
  int cur_lit=0;
  int dim[sDIMEN_MAX];
  int numdim,level;
  int fconst;

  fconst=matchtoken(tCONST);
  do {
    ident=iVARIABLE;
    size=1;
    numdim=0;                           /* no dimensions */
    tag=sc_addtag(NULL);
    if (lex(&val,&str)!=tSYMBOL)        /* read in (new) token */
      error(20,str);                    /* invalid symbol name */
    assert(strlen(str)<=sNAMEMAX);
    strcpy(name,str);                   /* save symbol name */
    if (name[0]==PUBLIC_CHAR)
      error(56,name);                   /* local variables cannot be public */
    /* Note: block locals may be named identical to locals at higher
     * compound blocks (as with standard C); so we must check (and add)
     * the "nesting level" of local variables to verify the
     * multi-definition of symbols.
     */
    if ((sym=findloc(name))!=NULL && sym->compound==ncmp)
      error(21,name);                   /* symbol already defined */
    /* Although valid, a local variable whose name is equal to that
     * of a global variable or to that of a local variable at a lower
     * level might indicate a bug.
     */
    if ((sym=findloc(name))!=NULL && sym->compound!=ncmp || findglb(name)!=NULL)
      error(219,name);                  /* variable shadows another symbol */
    while (matchtoken('[')){
      ident=iARRAY;
      if (numdim == sDIMEN_MAX) {
        error(53);                      /* exceeding maximum number of dimensions */
        return ident;
      } /* if */
      if (numdim>0 && dim[numdim-1]==0)
        error(52);                      /* only last dimension may be variable length */
      size=needsub(&idxtag[numdim]);    /* get size; size==0 for "var[]" */
      #if INT_MAX < LONG_MAX
        if (size > INT_MAX)
          error(105);                   /* overflow, exceeding capacity */
      #endif
      dim[numdim++]=(int)size;
    } /* while */
    if (ident==iARRAY || fstatic) {
      cur_lit=litidx;           /* save current index in the literal table */
      initials(ident,tag,&size,dim,numdim);
      if (size==0)
        return ident;           /* error message already given */
      if (numdim==1)
        dim[0]=(int)size;
    } /* if */
    /* reserve memory (on the stack) for the variable */
    if (fstatic) {
      /* write zeros for uninitialized fields */
      while (litidx<cur_lit+size)
        stowlit(0);
      sym=addvariable(name,(cur_lit+glb_declared)*sizeof(cell),ident,sSTATIC,
                      tag,dim,numdim,idxtag);
      defsymbol(name,ident,sSTATIC,(cur_lit+glb_declared)*sizeof(cell));
    } else {
      declared+=(int)size;      /* variables are put on stack, adjust "declared" */
      sym=addvariable(name,-declared*sizeof(cell),ident,sLOCAL,tag,
                      dim,numdim,idxtag);
      defsymbol(name,ident,sLOCAL,-declared*sizeof(cell));
      modstk(-(int)size*sizeof(cell));
    } /* if */
    /* now that we have reserved memory for the variable, we can proceed
     * to initialize it */
    assert(sym!=NULL);          /* we declared it, it must be there */
    sym->compound=ncmp;         /* for multiple declaration/shadowing check */
    if (fconst)
      sym->usage|=uCONST;
    if (ident==iARRAY)
      for (level=0; level<numdim; level++)
        symbolrange(level,dim[level]);
    if (!fstatic) {             /* static variables already initialized */
      if (ident==iVARIABLE) {
        /* simple variable, also supports initialization */
        int ctag = tag;         /* set to "tag" by default */
        int explicit_init=FALSE;/* is the variable explicitly initialized? */
        if (matchtoken('=')) {
          doexpr(FALSE,FALSE,FALSE,&ctag);
          explicit_init=TRUE;
        } else {
          const1(0);            /* uninitialized variable, set to zero */
        } /* if */
        /* now try to save the value (still in PRI) in the variable */
        lval.sym=sym;
        lval.ident=iVARIABLE;
        lval.constval=0;
        store(&lval);
        if (!matchtag(tag,ctag,TRUE))
          error(213);           /* tag mismatch */
        /* if the variable was not explicitly initialized, reset the
         * "uWRITTEN" flag that store() set */
        if (!explicit_init)
          sym->usage &= ~uWRITTEN;
      } else {
        /* an array */
        assert(cur_lit>=0 && cur_lit<=litidx && litidx<=litmax);
        /* if the array is not completely filled, set all values to zero first */
        assert(size>0 && size>=sym->dim.array.length);
        assert(numdim>1 || size==sym->dim.array.length);
        if (litidx-cur_lit < size)
          fillarray(sym,size*sizeof(cell),0);
        if (cur_lit<litidx) {
          /* check whether the complete array is set to a single value; if
           * it is, more compact code can be generated */
          cell first=litq[cur_lit];
          int i;
          for (i=cur_lit; i<litidx && litq[i]==first; i++)
            /* nothing */;
          if (i==litidx) {
            /* all values are the same */
            fillarray(sym,(litidx-cur_lit)*sizeof(cell),first);
            litidx=cur_lit;     /* reset literal table */
          } else {
            /* copy the literals to the array */
            const1((cur_lit+glb_declared)*sizeof(cell));
            copyarray(sym,(litidx-cur_lit)*sizeof(cell));
          } /* if */
        } /* if */
      } /* if */
    } /* if */
  } while (matchtoken(',')); /* enddo */   /* more? */
  needtoken(tTERM);    /* if not comma, must be semicolumn */
  return ident;
}

static cell calc_arraysize(int dim[],int numdim,int cur)
{
  if (cur==numdim)
    return 0;
  return dim[cur]+(dim[cur]*calc_arraysize(dim,numdim,cur+1));
}

/*  initials
 *
 *  Initialize global objects and local arrays.
 *    size==array cells (count), if 0 on input, the routine counts the number of elements
 *    tag==required tagname id (not the returned tag)
 *
 *  Global references: litidx (altered)
 */
static void initials(int ident,int tag,cell *size,int dim[],int numdim)
{
  int ctag;
  int curlit=litidx;
  int d;

  if (!matchtoken('=')) {
    if (ident==iARRAY && dim[numdim-1]==0) {
      /* declared as "myvar[];" which is senseless (note: this *does* make
       * sense in the case of a iREFARRAY, which is a function parameter)
       */
      error(9);         /* array has zero length -> invalid size */
    } /* if */
    if (numdim>1) {
      /* initialize the indirection tables */
      #if sDIMEN_MAX>2
        #error Array algorithms for more than 2 dimensions are not implemented
      #endif
      assert(numdim==2);
      *size=calc_arraysize(dim,numdim,0);
      for (d=0; d<dim[0]; d++)
        stowlit((dim[0]+d*(dim[1]-1)) * sizeof(cell));
    } /* if */
    return;
  } /* if */

  if (ident==iVARIABLE) {
    assert(*size==1);
    init(ident,&ctag);
    if (!matchtag(tag,ctag,TRUE))
      error(213);       /* tag mismatch */
  } else {
    assert(numdim>0);
    if (numdim==1) {
      *size=initvector(ident,tag,dim[0],FALSE);
    } else {
      cell offs,dsize;
      /* The simple algorithm below only works for arrays with one or two
       * dimensions. This should be some recursive algorithm.
       */
      #if sDIMEN_MAX>2
        #error Array algorithms for more than 2 dimensions are not implemented
      #endif
      assert(numdim==2);
      if (dim[numdim-1]!=0)
        *size=calc_arraysize(dim,numdim,0); /* set size to (known) full size */
      /* dump indirection tables */
      for (d=0; d<dim[0]; d++)
        stowlit(0);
      /* now dump individual vectors */
      needtoken('{');
      offs=dim[0];
      for (d=0; d<dim[0]; d++) {
        litq[curlit+d]=offs*sizeof(cell);
        dsize=initvector(ident,tag,dim[1],TRUE);
        offs+=dsize-1;
        if (d+1<dim[0])
          needtoken(',');
      } /* for */
      needtoken('}');
    } /* if */
  } /* if */

  if (*size==0)
    *size=litidx-curlit;                /* number of elements defined */
}

/*  initvector
 *  Initialize a single dimensional array
 */
static cell initvector(int ident,int tag,cell size,int fillzero)
{
  cell prev1=0,prev2=0;
  int ctag;
  int ellips=FALSE;
  int curlit=litidx;

  assert(ident==iARRAY || ident==iREFARRAY);
  if (matchtoken('{')) {
    do {
      if ((ellips=matchtoken(tELLIPS))!=0)
        break;
      prev2=prev1;
      prev1=init(ident,&ctag);
      if (!matchtag(tag,ctag,TRUE))
        error(213);             /* tag mismatch */
    } while (matchtoken(',')); /* do */
    needtoken('}');
  } else {
    init(ident,&ctag);
    if (!matchtag(tag,ctag,TRUE))
      error(213);               /* tagname mismatch */
  } /* if */
  /* fill up the literal queue with a series */
  if (ellips) {
    cell step=((litidx-curlit)==1) ? (cell)0 : prev1-prev2;
    if (size==0 || (litidx-curlit)==0)
      error(41);                /* invalid ellipsis, array size unknown */
    else if ((litidx-curlit)==(int)size)
      error(18);                /* initialisation data exceeds declared size */
    while ((litidx-curlit)<(int)size) {
      prev1+=step;
      stowlit(prev1);
    } /* while */
  } /* if */
  if (fillzero && size>0) {
    while ((litidx-curlit)<(int)size)
      stowlit(0);
  } /* if */
  if (size==0) {
    size=litidx-curlit;                 /* number of elements defined */
  } else if (litidx-curlit>(int)size) { /* e.g. "myvar[3]={1,2,3,4};" */
    error(18);                  /* initialisation data exceeds declared size */
    litidx=(int)size+curlit;    /* avoid overflow in memory moves */
  } /* if */
  return size;
}

/*  init
 *
 *  Evaluate one initializer.
 */
static cell init(int ident,int *tag)
{
  cell i = 0;

  if (matchtoken(tSTRING)){
    /* lex() automatically stores strings in the literal table (and
     * increases "litidx") */
    if (ident==iVARIABLE)
      error(6);         /* must be assigned to an array */
    *tag=0;
  } else if (constexpr(&i,tag)){
    stowlit(i);         /* store expression result in literal table */
  } /* if */
  return i;
}

/*  needsub
 *
 *  Get required array size
 */
static cell needsub(int *tag)
{
  cell val;

  *tag=0;
  if (matchtoken(']'))  /* we've already seen "[" */
    return 0;           /* null size (like "char msg[]") */
  constexpr(&val,tag);  /* get value (must be constant expression) */
  if (val<0) {
    error(9);           /* negative array size is invalid; assumed zero */
    val=0;
  } /* if */
  needtoken(']');
  return val;           /* return array size */
}

/*  decl_const  - declare a single constant
 *
 */
static void decl_const(int vclass)
{
  char constname[sNAMEMAX+1];
  cell val;
  char *str;
  int tag;

  tag=sc_addtag(NULL);
  if (lex(&val,&str)!=tSYMBOL)          /* read in (new) token */
    error(20,str);                      /* invalid symbol name */
  strcpy(constname,str);                /* save symbol name */
  needtoken('=');
  constexpr(&val,NULL);                 /* get value */
  needtoken(tTERM);
  /* add_constant() checks for duplicate definitions */
  add_constant(constname,val,vclass,tag);
}

/*  decl_enum   - declare enumerated constants
 *
 */
static void decl_enum(int vclass)
{
  char enumname[sNAMEMAX+1],constname[sNAMEMAX+1];
  cell val,value,size;
  char *str;
  int tag,tok;

  if (lex(&val,&str)==tSYMBOL) {       /* read in (new) token */
    strcpy(enumname,str);               /* save enum name (last constant) */
    tag=sc_addtag(enumname);
  } else {
    lexpush();                          /* analyze again */
    enumname[0]='\0';
    tag=0;
  } /* if */
  needtoken('{');
  /* go through all constants */
  value=0;                              /* default starting value */
  do {
    if (matchtoken('}')) {              /* quick exit if '}' follows ',' */
      lexpush();
      break;
    } /* if */
    tok=lex(&val,&str);                 /* read in (new) token */
    if (tok!=tSYMBOL && tok!=tLABEL)
      error(20,str);                    /* invalid symbol name */
    strcpy(constname,str);              /* save symbol name */
    size=1;                             /* default increment of 'val' */
    if (tok==tLABEL || matchtoken(':'))
      constexpr(&size,NULL);            /* get size */
    if (matchtoken('='))
      constexpr(&value,NULL);           /* get value */
    /* add_constant() checks whether a variable (global or local) or
     * a constant with the same name already exists */
    add_constant(constname,value,vclass,tag);
    value+=size;
  } while (matchtoken(','));
  needtoken('}');       /* terminates the constant list */
  matchtoken(';');      /* eat an optional ; */
  /* set the enum name to the last value plus one */
  if (strlen(enumname)>0)
    add_constant(enumname,value,vclass,tag);
}

/*
 *  Finds a function in the global symbol table or creates a new entry.
 *  It does some basic processing and error checking.
 */
SC_FUNC symbol *fetchfunc(char *name,int tag)
{
  symbol *sym;
  cell offset;

  offset=code_idx;
  if ((debug & sSYMBOLIC)!=0) {
    offset+=opcodes(1)+opargs(3)+nameincells(name);
    /* ^^^ The address for the symbol is the code address. But the "symbol"
     *     instruction itself generates code. Therefore the offset is
     *     pre-adjusted to the value it will have after the symbol instruction.
     */
  } /* if */
  if ((sym=findglb(name))!=0) {           /* already in symbol table? */
    if (sym->ident!=iFUNCTN)
      error(21,name);                     /* yes, but not as a function */
    else if ((sym->usage & uDEFINE)!=0)
      error(21,name);                     /* yes, and it's already defined */
    else if ((sym->usage & uNATIVE)!=0)
      error(21,name);                     /* yes, and it is an native */
    assert(sym->vclass==sGLOBAL);
    if ((sym->usage & uDEFINE)==0) {
      /* as long as the function stays undefined, update the address and the tag */
      sym->addr=offset;
      sym->tag=tag;
    } /* if */
  } else {
    /* don't set the "uDEFINE" flag; it may be a prototype */
    sym=addsym(name,offset,iFUNCTN,sGLOBAL,tag,0);
    /* assume no arguments */
    sym->dim.arglist=(arginfo*)malloc(1*sizeof(arginfo));
    sym->dim.arglist[0].ident=0;
    /* set library ID to NULL (only for native functions) */
    sym->x.lib=NULL;
  } /* if */
  return sym;
}

/* This routine adds symbolic information for each argument.
 */
static void define_args(void)
{
  symbol *sym;

  /* At this point, no local variables have been declared. All
   * local symbols are function arguments.
   */
  sym=loctab.next;
  while (sym!=NULL) {
    assert(sym->ident!=iLABEL);
    assert(sym->vclass==sLOCAL);
    defsymbol(sym->name,sym->ident,sLOCAL,sym->addr);
    if (sym->ident==iREFARRAY) {
      symbol *sub=sym;
      while (sub!=NULL) {
        symbolrange(sub->dim.array.level,sub->dim.array.length);
        sub=finddepend(sub);
      } /* while */
    } /* if */
    sym=sym->next;
  } /* while */
}

static int operatorname(char *name)
{
  int opertok;
  char *str;
  cell val;

  assert(name!=NULL);

  /* check the operator */
  opertok=lex(&val,&str);
  switch (opertok) {
  case '+':
  case '-':
  case '*':
  case '/':
  case '%':
  case '>':
  case '<':
  case '!':
    name[0]=(char)opertok;
    name[1]='\0';
    break;
  case tINC:
    strcpy(name,"++");
    break;
  case tDEC:
    strcpy(name,"--");
    break;
  case tlEQ:
    strcpy(name,"==");
    break;
  case tlNE:
    strcpy(name,"!=");
    break;
  case tlLE:
    strcpy(name,"<=");
    break;
  case tlGE:
    strcpy(name,">=");
    break;
  default:
    name[0]='\0';
    error(61);          /* operator cannot be redefined */
    return 0;
  } /* switch */

  return opertok;
}

static int operatoradjust(int opertok,symbol *sym,char *opername)
{
  int tags[2]={0,0};
  int count=0;
  arginfo *arg;
  char tmpname[sNAMEMAX+1];
  symbol *oldsym;

  if (opertok==0)
    return TRUE;

  assert(sym!=NULL && sym->ident==iFUNCTN && sym->dim.arglist!=NULL);
  /* count operators and save (first two) tags */
  while (arg=&sym->dim.arglist[count], arg->ident!=0) {
    if (count<2) {
      if (arg->numtags>1)
        error(65,count+1);  /* function argument may only have a single tag */
      else if (arg->numtags==1)
        tags[count]=arg->tags[0];
    } /* if */
    if (arg->ident!=iVARIABLE)
      error(66,arg->name);  /* must be non-reference argument */
    if (arg->hasdefault)
      error(59,arg->name);  /* arguments of an operator may not have a default value */
    count++;
  } /* while */

  /* for '!', '++' and '--', count should be 1
   * for '-', count may be 1 or 2
   * for all other (binary) operators, count must be 2
   */
  switch (opertok) {
  case '!':
  case tINC:
  case tDEC:
    if (count!=1)
      error(62);      /* number or placement of the operands does not fit the operator */
    break;
  case '-':
    if (count!=1 && count!=2)
      error(62);      /* number or placement of the operands does not fit the operator */
    break;
  default:
    if (count!=2)
      error(62);      /* number or placement of the operands does not fit the operator */
  } /* switch */

  if (tags[0]==0 && tags[1]==0)
    error(64);        /* cannot change predefined operators */

  /* change the operator name */
  assert(strlen(opername)>0);
  switch (count) {
  case 1:
    sprintf(tmpname,"%s%d",opername,tags[0]);
    break;
  case 2:
    sprintf(tmpname,"%d%s%d",tags[0],opername,tags[1]);
    break;
  } /* switch */
  if ((oldsym=findglb(tmpname))!=NULL) {
    int i;
    if ((oldsym->usage & uDEFINE)!=0) {
      char errname[2*sNAMEMAX+16];
      funcdisplayname(errname,tmpname);
      error(21,errname);        /* symbol already defined */
    } /* if */
    sym->usage|=oldsym->usage;  /* copy flags from the previous definition */
    sym->numrefers=oldsym->numrefers;
    for (i=0; i<sym->numrefers; i++)
      if (oldsym->refer[i]!=NULL)
        refer_symbol(sym,oldsym->refer[i]);
    delete_symbol(&glbtab,oldsym);
  } /* if */
  if ((debug & sSYMBOLIC)!=0)
    sym->addr += nameincells(tmpname) - nameincells(sym->name);
  strcpy(sym->name,tmpname);
  sym->hash=namehash(sym->name);/* calculate new hash */

  /* operators should return a value, except the "++" and "--" operators */
  if (opertok!=tINC && opertok!=tDEC)
    sym->usage |= uRETVALUE;

  return TRUE;
}

static int check_operatortag(int opertok,int resulttag)
{
  switch (opertok) {
  case '!':
  case '<':
  case '>':
  case tlEQ:
  case tlNE:
  case tlLE:
  case tlGE:
    if (resulttag!=sc_addtag("bool")) {
      error(63);                /* operator requires a "bool" result tag */
      return FALSE;
    } /* if */
  } /* switch */
  return TRUE;
}

static int parse_funcname(char *fname,int *tag1,int *tag2,char *opname)
{
  char *ptr;
  int unary;

  /* tags are only positive, so if the function name starts with a '-',
   * the operator is an unary '-' or '--' operator.
   */
  if (*fname=='-') {
    *tag1=0;
    unary=TRUE;
    ptr=fname;
  } else {
    *tag1=(int)strtol(fname,&ptr,10);
    unary= ptr==fname;
  } /* if */
  assert(!unary || *tag1==0);
  assert(*ptr!='\0');
  while (!isdigit(*ptr))
    *opname++ = *ptr++;
  *opname='\0';
  *tag2=(int)strtol(ptr,NULL,10);
  return unary;
}

SC_FUNC char *funcdisplayname(char *dest,char *funcname)
{
  int tags[2];
  char opname[10];
  constval *tagsym[2];
  int unary;

  if (isalpha(*funcname) || *funcname=='_' || *funcname==PUBLIC_CHAR || *funcname=='\0') {
    if (dest!=funcname)
      strcpy(dest,funcname);
    return dest;
  } /* if */

  unary=parse_funcname(funcname,&tags[0],&tags[1],opname);
  tagsym[1]=find_constval_byval(&tagname_tab,tags[1]);
  assert(tagsym[1]!=NULL);
  if (unary) {
    sprintf(dest,"operator%s(%s:)",opname,tagsym[1]->name);
  } else {
    tagsym[0]=find_constval_byval(&tagname_tab,tags[0]);
    assert(tagsym[0]!=NULL);
    sprintf(dest,"operator%s(%s:,%s:)",opname,tagsym[0]->name,tagsym[1]->name);
  } /* if */
  return dest;
}

static void funcstub(int native)
{
  int tok,tag;
  char *str;
  cell val;
  char symbolname[sNAMEMAX+1];
  symbol *sym;
  int opertok;

  opertok=0;
  lastst=0;
  litidx=0;                     /* clear the literal pool */
  assert(loctab.next==NULL);    /* local symbol table should be empty */

  tag=sc_addtag(NULL);
  tok=lex(&val,&str);
  if (native) {
    if (tok==tPUBLIC || tok==tSTOCK || tok==tSTATIC || tok==tSYMBOL && *str==PUBLIC_CHAR)
      error(42);                /* invalid combination of class specifiers */
  } else {
    if (tok==tPUBLIC || tok==tSTATIC)
      tok=lex(&val,&str);
  } /* if */
  if (tok==tOPERATOR) {
    opertok=operatorname(symbolname);
    if (opertok==0)
      return;                   /* error message already given */
    check_operatortag(opertok,tag);
  } else {
    if (tok!=tSYMBOL && freading) {
      error(10);                /* illegal function or declaration */
      return;
    } /* if */
    strcpy(symbolname,str);
  } /* if */
  needtoken('(');               /* only functions may be native/forward */

  sym=fetchfunc(symbolname,tag);/* get a pointer to the function entry */
  if (sym==NULL)
    return;
  if (native) {
    sym->usage=uNATIVE | uRETVALUE | uDEFINE;
    sym->x.lib=curlibrary;
  } /* if */

  declargs(sym);
  /* "declargs()" found the ")" */
  if (!operatoradjust(opertok,sym,symbolname))
    sym->usage &= ~uDEFINE;
  /* for a native operator, also need to specify an "exported" function name;
   * for a native function, this is optional
   */
  if (native) {
    if (opertok!=0) {
      needtoken('=');
      lexpush();        /* push back, for matchtoken() to retrieve again */
    } /* if */
    if (matchtoken('=')) {
      /* allow number or symbol */
      if (matchtoken(tSYMBOL)) {
        tokeninfo(&val,&str);
        if (strlen(str)>sEXPMAX) {
          error(220,str,sEXPMAX);
          str[sEXPMAX]='\0';
        } /* if */
        insert_alias(sym->name,str);
      } else {
        constexpr(&val,NULL);
        sym->addr=val;
        // ??? Must mark this address, so that it won't be generated again
        //     and it won't be written to the output file. At the moment,
        //     I have assumed that this syntax is only valid if val < 0.
        //     To properly mix "normal" native functions and indexed native
        //     functions, one should use negative indices anyway.
        //     Special code for a negative index in sym->addr exists in
        //     SC4.C (ffcall()) and in SC6.C (the loops for counting the
        //     number of native variables and for writing them).
      } /* if */
    } /* if */
  } /* if */
  needtoken(tTERM);

  litidx=0;                     /* clear the literal pool */
  delete_symbols(&loctab,0,TRUE,TRUE);/* clear local variables queue */
}

/*  newfunc    - begin a function
 *
 *  This routine is called from "parse" and tries to make a function
 *  out of the following text
 *
 *  Global references: funcstatus,lastst,litidx
 *                     rettype  (altered)
 *                     curfunc  (altered)
 *                     declared (altered)
 *                     glb_declared (altered)
 */
static int newfunc(char *firstname,int firsttag,int fpublic,int fstatic,int stock)
{
  symbol *sym;
  int argcnt,tok,tag,funcline;
  int opertok,opererror;
  char symbolname[sNAMEMAX+1];
  char *str;
  cell val,cidx,glbdecl;
  int filenum;

  opertok=0;
  lastst=0;             /* no statement yet */
  litidx=0;             /* clear the literal pool */
  cidx=0;               /* just to avoid compiler warnings */
  glbdecl=0;
  assert(loctab.next==NULL);    /* local symbol table should be empty */
  filenum=fcurrent;     /* save file number at the start of the declaration */

  if (firstname!=NULL) {
    assert(strlen(firstname)<=sNAMEMAX);
    strcpy(symbolname,firstname);       /* save symbol name */
    tag=firsttag;
  } else {
    tag= (firsttag>=0) ? firsttag : sc_addtag(NULL);
    tok=lex(&val,&str);
    assert(!fpublic);
    if (tok==tNATIVE || tok==tPUBLIC && stock)
      error(42);                /* invalid combination of class specifiers */
    if (tok==tOPERATOR) {
      opertok=operatorname(symbolname);
      if (opertok==0)
        return TRUE;            /* error message already given */
      check_operatortag(opertok,tag);
    } else {
      if (tok!=tSYMBOL && freading) {
        error(20,str);          /* invalid symbol name */
        return FALSE;
      } /* if */
      assert(strlen(str)<=sNAMEMAX);
      strcpy(symbolname,str);
    } /* if */
  } /* if */
  /* check whether this is a function or a variable declaration */
  if (!matchtoken('('))
    return FALSE;
  /* so it is a function, proceed */
  funcline=fline;               /* save line at which the function is defined */
  if (symbolname[0]==PUBLIC_CHAR) {
    fpublic=TRUE;               /* implicitly public function */
    if (stock)
      error(42);                /* invalid combination of class specifiers */
  } /* if */
  sym=fetchfunc(symbolname,tag);/* get a pointer to the function entry */
  if (fpublic)
    sym->usage|=uPUBLIC;
  if (fstatic)
    sym->fnumber=filenum;
  /* declare all arguments */
  argcnt=declargs(sym);
  opererror=!operatoradjust(opertok,sym,symbolname);
  if (strcmp(symbolname,uMAINFUNC)==0) {
    if (argcnt>0)
      error(5);         /* "main()" function may not have any arguments */
    sym->usage|=uREAD;  /* "main()" is the program's entry point: always used */
  } /* if */
  /* "declargs()" found the ")"; if a ";" appears after this, it was a
   * prototype */
  if (matchtoken(';')) {
    if (!needsemicolon)
      error(218);       /* old style prototypes used with optional semicolumns */
    delete_symbols(&loctab,0,TRUE,TRUE);  /* prototype is done; forget everything */
    return TRUE;
  } /* if */
  /* so it is not a prototype, proceed */
  /* if this is a function that is not referred to (this can only be detected
   * in the second stage), shut code generation off */
  if (sc_status==statWRITE && (sym->usage & uREAD)==0) {
    sc_status=statSKIP;
    cidx=code_idx;
    glbdecl=glb_declared;
  } /* if */
  begcseg();
  sym->usage|=uDEFINE;  /* set the definition flag */
  if (fpublic)
    sym->usage|=uREAD;  /* public functions are always "used" */
  if (stock)
    sym->usage|=uSTOCK;
  if (opertok!=0 && opererror)
    sym->usage &= ~uDEFINE;
  defsymbol(sym->name,iFUNCTN,sGLOBAL,
            code_idx+opcodes(1)+opargs(3)+nameincells(sym->name));
         /* ^^^ The address for the symbol is the code address. But the
          * "symbol" instruction itself generates code. Therefore the
          * offset is pre-adjusted to the value it will have after the
          * symbol instruction.
          */
  startfunc(sym->name); /* creates stack frame */
  if ((debug & sSYMBOLIC)!=0)
    setline(funcline,fcurrent);
  declared=0;           /* number of local cells */
  rettype=(sym->usage & uRETVALUE);      /* set "return type" variable */
  curfunc=sym;
  define_args();        /* add the symbolic info for the function arguments */
  statement(NULL);
  if ((rettype & uRETVALUE)!=0)
    sym->usage|=uRETVALUE;
  if ((lastst!=tRETURN) && (lastst!=tGOTO)){
    const1(0);
    ffret();
    if ((sym->usage & uRETVALUE)!=0)
      error(209);               /* function should return a value */
  } /* if */
  endfunc();
  if (litidx) {                 /* if there are literals defined */
    glb_declared+=litidx;
    begdseg();                  /* flip to DATA segment */
    dumplits();                 /* dump literal strings */
  } /* if */
  testsymbols(&loctab,0,TRUE,TRUE);     /* test for unused arguments and labels */
  delete_symbols(&loctab,0,TRUE,TRUE);  /* clear local variables queue */
  assert(loctab.next==NULL);
  curfunc=NULL;
  if (sc_status==statSKIP) {
    sc_status=statWRITE;
    code_idx=cidx;
    glb_declared=glbdecl;
  } /* if */
  return TRUE;
}

static int argcompare(arginfo *a1,arginfo *a2)
{
  int result,level;

  result= strcmp(a1->name,a2->name)==0;
  if (result)
    result= a1->ident==a2->ident;
  if (result)
    result= a1->usage==a2->usage;
  if (result)
    result= a1->numtags==a2->numtags;
  if (result) {
    int i;
    for (i=0; i<a1->numtags && result; i++)
      result= a1->tags[i]==a2->tags[i];
  } /* if */
  if (result)
    result= a1->hasdefault==a2->hasdefault;
  if (a1->hasdefault) {
    if (a1->ident==iREFARRAY) {
      if (result)
        result= a1->defvalue.array.size==a2->defvalue.array.size;
      if (result)
        result= a1->defvalue.array.arraysize==a2->defvalue.array.arraysize;
      /* also check the dimensions of both arrays */
      if (result)
        result= a1->numdim==a2->numdim;
      for (level=0; result && level<a1->numdim; level++)
        result= a1->dim[level]==a2->dim[level];
      /* ??? should also check contents of the default array (these troubles
       * go away in a true 2-pass compiler) */
    } else {
      if (result) {
        if ((a1->hasdefault & uSIZEOF)!=0)
          result= strcmp(a1->defvalue.symname,a2->defvalue.symname)==0;
        else
          result= a1->defvalue.val==a2->defvalue.val;
      } /* if */
    } /* if */
  } /* if */
  return result;
}

/*  declargs()
 *
 *  This routine adds an entry in the local symbol table for each argument
 *  found in the argument list. It returns the number of arguments.
 */
static int declargs(symbol *sym)
{
  #define MAXTAGS 16
  char *ptr;
  int argcnt,tok,tags[MAXTAGS],numtags;
  cell val;
  arginfo arg, *arglist;
  char name[sNAMEMAX+1];
  int ident,fpublic,fconst;
  int idx;

  /* the '(' parantheses has already been parsed */
  argcnt=0;                     /* zero aruments up to now */
  ident=iVARIABLE;
  numtags=0;
  fconst=FALSE;
  fpublic= (sym->usage & uPUBLIC)!=0;
  if (!matchtoken(')')){
    do {                        /* there are arguments; process them */
      /* any legal name increases argument count (and stack offset) */
      tok=lex(&val,&ptr);
      switch (tok) {
      case 0:
        /* nothing */
        break;
      case '&':
        if (ident!=iVARIABLE || numtags>0)
          error(1,"-identifier-","&");
        ident=iREFERENCE;
        break;
      case tCONST:
        if (ident!=iVARIABLE || numtags>0)
          error(1,"-identifier-","const");
        fconst=TRUE;
        break;
      case tLABEL:
        if (numtags>0)
          error(1,"-identifier-","-tagname-");
        tags[0]=sc_addtag(ptr);
        numtags=1;
        break;
      case '{':
        if (numtags>0)
          error(1,"-identifier-","-tagname-");
        numtags=0;
        while (numtags<MAXTAGS) {
          if (!needtoken(tSYMBOL))
            break;
          tokeninfo(&val,&ptr);
          tags[numtags++]=sc_addtag(ptr);
          if (matchtoken('}'))
            break;
          needtoken(',');
        } /* for */
        needtoken(':');
        tok=tLABEL;     /* for outer loop: flag that we have seen a tagname */
        break;
      case tSYMBOL:
        if (argcnt>=sMAXARGS)
          error(45);            /* too many function arguments */
        strcpy(name,ptr);       /* save symbol name */
        if (name[0]==PUBLIC_CHAR)
          error(56,name);       /* function arguments cannot be public */
        if (numtags==0)
          tags[numtags++]=0;    /* default tag */
        /* Stack layout:
         *   base + 0*sizeof(cell)  == previous "base"
         *   base + 1*sizeof(cell)  == function return address
         *   base + 2*sizeof(cell)  == number of arguments
         *   base + 3*sizeof(cell)  == first argument of the function
         * So the offset of each argument is "(argcnt+3) * sizeof(cell)".
         */
        doarg(name,ident,(argcnt+3)*sizeof(cell),tags,numtags,fpublic,fconst,&arg);
        if (fpublic && arg.hasdefault)
          error(59,name);       /* arguments of a public function may not have a default value */
        if ((sym->usage & uPROTOTYPED)==0) {
          /* redimension the argument list, add the entry */
          sym->dim.arglist=(arginfo*)realloc(sym->dim.arglist,(argcnt+2)*sizeof(arginfo));
          if (sym->dim.arglist==0)
            error(103);         /* insufficient memory */
          sym->dim.arglist[argcnt]=arg;
          sym->dim.arglist[argcnt+1].ident=0;   /* keep the list terminated */
        } else {
          /* check the argument with the earlier definition */
          if (!argcompare(&sym->dim.arglist[argcnt],&arg))
            error(25);          /* function definition does not match prototype */
          /* may need to free default array argument and the tag list */
          if (arg.ident==iREFARRAY && arg.hasdefault)
            free(arg.defvalue.array.data);
          free(arg.tags);
        } /* if */
        argcnt++;
        ident=iVARIABLE;
        numtags=0;
        fconst=FALSE;
        break;
      case tELLIPS:
        if (ident!=iVARIABLE)
          error(10);            /* illegal function or declaration */
        if (numtags==0)
          tags[numtags++]=0;    /* default tag */
        if ((sym->usage & uPROTOTYPED)==0) {
          /* redimension the argument list, add the entry iVARARGS */
          sym->dim.arglist=(arginfo*)realloc(sym->dim.arglist,(argcnt+2)*sizeof(arginfo));
          if (sym->dim.arglist==0)
            error(103);         /* insufficient memory */
          sym->dim.arglist[argcnt].ident=iVARARGS;
          sym->dim.arglist[argcnt+1].ident=0;   /* keep the list terminated */
          sym->dim.arglist[argcnt].numtags=numtags;
          sym->dim.arglist[argcnt].tags=(int*)malloc(numtags*sizeof tags[0]);
          if (sym->dim.arglist[argcnt].tags==NULL)
            error(103);         /* insufficient memory */
          memcpy(sym->dim.arglist[argcnt].tags,tags,sizeof tags[0]);
        } /* if */
        break;
      default:
        error(10);              /* illegal function or declaration */
      } /* switch */
    } while (tok=='&' || tok==tLABEL || tok==tCONST
             || tok!=tELLIPS && matchtoken(',')); /* more? */
    /* if the next token is not ",", it should be ")" */
    needtoken(')');
  } /* if */
  /* resolve any "sizeof" arguments (now that all arguments are known) */
  assert(sym->dim.arglist!=NULL);
  arglist=sym->dim.arglist;
  for (idx=0; idx<argcnt; idx++) {
    if ((arglist[idx].hasdefault & uSIZEOF)!=0) {
      int altidx;
      /* Find the argument with the name mentioned after the "sizeof". Note
       * that we cannot use findloc here because we need the arginfo struct,
       * not the symbol.
       */
      ptr=arglist[idx].defvalue.symname;
      for (altidx=0; altidx<argcnt && strcmp(ptr,arglist[altidx].name)!=0; altidx++)
        /* nothing */;
      if (altidx>=argcnt) {
        error(17,ptr);          /* undefined symbol */
      } else {
        assert(arglist[idx].defvalue.symname!=NULL);
        /* check the type of the argument whose size to take; for a iVARIABLE
         * or a iREFERENCE, this is always 1 (so the code is redundant)
         */
        assert(arglist[altidx].ident!=iVARARGS);
        if (arglist[altidx].ident!=iREFARRAY) {
          assert(arglist[altidx].ident==iVARIABLE || arglist[altidx].ident==iREFERENCE);
          error(223,ptr);       /* redundant sizeof */
        } /* if */
      } /* if */
    } /* if */
  } /* for */

  sym->usage|=uPROTOTYPED;
  errorset(sRESET);             /* reset error flag (clear the "panic mode")*/
  return argcnt;
}

/*  doarg       - declare one argument type
 *
 *  this routine is called from "declargs()" and adds an entry in the local
 *  symbol table for one argument.
 *
 *  "fpublic" indicates whether the function for this argument list is public.
 *  The arguments themselves are never public.
 */
static void doarg(char *name,int ident,int offset,int tags[],int numtags,
                  int fpublic,int fconst,arginfo *arg)
{
  symbol *argsym;
  cell size;
  int idxtag[sDIMEN_MAX];

  strcpy(arg->name,name);
  arg->hasdefault=FALSE;        /* preset (most common case) */
  arg->defvalue.val=0;          /* clear */
  arg->numdim=0;
  if (matchtoken('[')) {
    if (ident==iREFERENCE)
      error(67,name);           /* illegal declaration ("&name[]" is unsupported) */
    do {
      if (arg->numdim == sDIMEN_MAX) {
        error(53);              /* exceeding maximum number of dimensions */
        return;
      } /* if */
      /* there is no check for non-zero major dimensions here, only if the
       * array parameter has a default value, we enforce that all array
       * dimensions, except the last, are non-zero
       */
      size=needsub(&idxtag[arg->numdim]);/* may be zero here, it is a pointer anyway */
      #if INT_MAX < LONG_MAX
        if (size > INT_MAX)
          error(105);           /* overflow, exceeding capacity */
      #endif
      arg->dim[arg->numdim]=(int)size;
      arg->numdim+=1;
    } while (matchtoken('['));
    ident=iREFARRAY;            /* "reference to array" (is a pointer) */
    if (matchtoken('=')) {
      int level;
      lexpush();                /* initials() needs it again */
      assert(litidx==0);        /* at the start of a function, this is reset */
      assert(numtags>0);
      /* for the moment, when a default value is given for the array, all
       * dimension sizes, except the last, must be non-zero (function initials()
       * requires to know the major dimensions)
       */
      for (level=0; level < arg->numdim - 1; level++)
        if (arg->dim[level]==0)
          error(52);            /* only last dimension may be variable length */
      initials(ident,tags[0],&size,arg->dim,arg->numdim);
      assert(size>=litidx);
      /* allocate memory to hold the initial values */
      arg->defvalue.array.data=(cell *)malloc(litidx*sizeof(cell));
      if (arg->defvalue.array.data!=NULL) {
        int i;
        memcpy(arg->defvalue.array.data,litq,litidx*sizeof(cell));
        arg->hasdefault=TRUE;   /* argument has default value */
        arg->defvalue.array.size=litidx;
        arg->defvalue.array.addr=-1;
        /* calulate size to reserve on the heap */
        arg->defvalue.array.arraysize=1;
        for (i=0; i<arg->numdim; i++)
          arg->defvalue.array.arraysize*=arg->dim[i];
        if (arg->defvalue.array.arraysize < arg->defvalue.array.size)
          arg->defvalue.array.arraysize = arg->defvalue.array.size;
      } /* if */
      litidx=0;                 /* reset */
    } /* if */
  } else {
    if (matchtoken('=')) {
      assert(ident==iVARIABLE || ident==iREFERENCE);
      arg->hasdefault=TRUE;     /* argument has a default value */
      if (matchtoken(tSIZEOF)) {
        if (ident==iREFERENCE)
          error(66,name);       /* argument may not be a reference */
        if (needtoken(tSYMBOL)) {
          /* save the name of the argument whose size id to take */
          char *name;
          cell val;
          tokeninfo(&val,&name);
          if ((arg->defvalue.symname=strdup(name)) == NULL)
            error(103);         /* insufficient memory */
          if (ident==iVARIABLE) /* make sure we set this only if not a reference */
            arg->hasdefault |= uSIZEOF;
        } /* if */
      } else {
        constexpr(&arg->defvalue.val,NULL);
      } /* if */
    } /* if */
  } /* if */
  arg->ident=(char)ident;
  arg->usage=(char)(fconst ? uCONST : 0);
  arg->numtags=numtags;
  arg->tags=(int*)malloc(numtags*sizeof tags[0]);
  if (arg->tags==NULL)
    error(103);                 /* insufficient memory */
  memcpy(arg->tags,tags,numtags*sizeof tags[0]);
  argsym=findloc(name);
  if (argsym!=NULL) {
    error(21,name);             /* symbol already defined */
  } else {
    if ((argsym=findglb(name))!=NULL && argsym->ident!=iFUNCTN)
      error(219,name);          /* variable shadows another symbol */
    /* add details of type and address */
    assert(numtags>0);
    argsym=addvariable(name,offset,ident,sLOCAL,tags[0],
                       arg->dim,arg->numdim,idxtag);
    argsym->compound=0;
    if (ident==iREFERENCE)
      argsym->usage|=uREAD;     /* because references are passed back */
    if (fpublic)
      argsym->usage|=uREAD;     /* arguments of public functions are always "used" */
    if (fconst)
      argsym->usage|=uCONST;
  } /* if */
}

static int count_referrers(symbol *entry)
{
  int i,count;

  count=0;
  for (i=0; i<entry->numrefers; i++)
    if (entry->refer[i]!=NULL)
      count++;
  return count;
}

#if !defined SC_LIGHT
static void dump_referrers(symbol *root,FILE *log)
{
  char symname[2*sNAMEMAX+16];
  int i,first;
  symbol *sym,*ref;

  /* use multiple passes to print constants variables and functions in
   * separate sections
   */

  fprintf(log,"(1) Constants\n");
  for (sym=root->next; sym!=NULL; sym=sym->next) {
    if (sym->parent!=NULL)
      continue;                 /* hierarchical data type */
    assert(sym->ident==iCONSTEXPR || sym->ident==iVARIABLE
           || sym->ident==iARRAY || sym->ident==iFUNCTN);
    if (sym->ident!=iCONSTEXPR)
      continue;
    if ((sym->usage & uREAD)==0)
      continue;
    fprintf(log,"%s:\t",funcdisplayname(symname,sym->name));
    assert(sym->refer!=NULL);
    first=TRUE;
    for (i=0; i<sym->numrefers; i++) {
      if ((ref=sym->refer[i])!=NULL) {
        if (!first)
          fprintf(log,", ");
        fprintf(log,"%s",funcdisplayname(symname,ref->name));
        first=FALSE;
      } /* if */
    } /* for */
    fprintf(log,"\n");
  } /* for */

  fprintf(log,"\n(2) Variables\n");
  for (sym=root->next; sym!=NULL; sym=sym->next) {
    if (sym->parent!=NULL)
      continue;                 /* hierarchical data type */
    if (sym->ident!=iVARIABLE && sym->ident!=iARRAY)
      continue;
    fprintf(log,"%s:\t",funcdisplayname(symname,sym->name));
    assert(sym->refer!=NULL);
    first=TRUE;
    for (i=0; i<sym->numrefers; i++) {
      if ((ref=sym->refer[i])!=NULL) {
        if (!first)
          fprintf(log,", ");
        fprintf(log,"%s",funcdisplayname(symname,ref->name));
        first=FALSE;
      } /* if */
    } /* for */
    fprintf(log,"\n");
  } /* for */

  fprintf(log,"\n(3) Functions\n");
  for (sym=root->next; sym!=NULL; sym=sym->next) {
    if (sym->parent!=NULL)
      continue;                 /* hierarchical data type */
    if (sym->ident!=iFUNCTN)
      continue;
    if ((sym->usage & (uREAD | uNATIVE))==uNATIVE)
      continue;                 /* unused native function */
    fprintf(log,"%s:\t",funcdisplayname(symname,sym->name));
    assert(sym->refer!=NULL);
    first=TRUE;
    /* check whether this function is called from the outside */
    if ((sym->usage & uPUBLIC)!=0) {
      fprintf(log,"(public)");
      first=FALSE;
    } else if (strcmp(sym->name,uMAINFUNC)==0) {
      fprintf(log,"(entry)");
      first=FALSE;
    } /* if */
    for (i=0; i<sym->numrefers; i++) {
      if ((ref=sym->refer[i])!=NULL) {
        if (!first)
          fprintf(log,", ");
        fprintf(log,"%s",funcdisplayname(symname,ref->name));
        first=FALSE;
      } /* if */
    } /* for */
    fprintf(log,"\n");
  } /* for */
}
#endif

/* Every symbol has a referrer list, that contains the functions that use
 * the symbol. Now, if function "apple" is accessed by functions "banana" and
 * "citron", but neither function "banana" nor "citron" are used by anyone
 * else, then, by inference, function "apple" is not used either.
 */
static void reduce_referrers(symbol *root)
{
  int i,restart;
  symbol *sym,*ref;

  do {
    restart=0;
    for (sym=root->next; sym!=NULL; sym=sym->next) {
      if (sym->parent!=NULL)
        continue;                 /* hierarchical data type */
      if (sym->ident==iFUNCTN
          && (sym->usage & uNATIVE)==0
          && (sym->usage & uPUBLIC)==0 && strcmp(sym->name,uMAINFUNC)!=0
          && count_referrers(sym)==0)
      {
        sym->usage&=~(uREAD | uWRITTEN);  /* erase usage bits if there is no referrer */
        /* find all symbols that are referred by this symbol */
        for (ref=root->next; ref!=NULL; ref=ref->next) {
          if (ref->parent!=NULL)
            continue;             /* hierarchical data type */
          assert(ref->refer!=NULL);
          for (i=0; i<ref->numrefers && ref->refer[i]!=sym; i++)
            /* nothing */;
          if (i<ref->numrefers) {
            assert(ref->refer[i]==sym);
            ref->refer[i]=NULL;
            restart++;
          } /* if */
        } /* for */
      } else if ((sym->ident==iVARIABLE || sym->ident==iARRAY)
                 && (sym->usage & uPUBLIC)==0
                 && sym->parent==NULL
                 && count_referrers(sym)==0)
      {
        sym->usage&=~(uREAD | uWRITTEN);  /* erase usage bits if there is no referrer */
      } /* if */
    } /* for */
    /* after removing a symbol, check whether more can be removed */
  } while (restart>0);
}

/*  testsymbols - test for unused local or global variables
 *
 *  "Public" functions are excluded from the check, since these
 *  may be exported to other object modules.
 *  Labels are excluded from the check if the argument 'testlabs'
 *  is 0. Thus, labels are not tested until the end of the function.
 *  Constants may also be excluded (convenient for global constants).
 *
 *  When the nesting level drops below "level", the check stops.
 *
 *  The function returns whether there is an "entry" point for the file.
 *  This flag will only be 1 when browsing the global symbol table.
 */
static int testsymbols(symbol *root,int level,int testlabs,int testconst)
{
  char symname[2*sNAMEMAX+16];
  int entry=FALSE;

  symbol *sym=root->next;
  while (sym!=NULL && sym->compound>=level) {
    switch (sym->ident) {
    case iLABEL:
      if (testlabs) {
        if ((sym->usage & uDEFINE)==0)
          error(19,sym->name);            /* not a label: ... */
        else if ((sym->usage & uREAD)==0)
          error(203,sym->name);           /* symbol isn't used: ... */
      } /* if */
      break;
    case iFUNCTN:
      if ((sym->usage & (uDEFINE | uREAD | uNATIVE | uSTOCK))==uDEFINE) {
        funcdisplayname(symname,sym->name);
        if (strlen(symname)>0)
          error(203,symname);       /* symbol isn't used ... (and not native/stock) */
      } /* if */
      if ((sym->usage & uPUBLIC)!=0 || strcmp(sym->name,uMAINFUNC)==0)
        entry=TRUE;                 /* there is an entry point */
      break;
    case iCONSTEXPR:
      if (testconst && (sym->usage & uREAD)==0)
        error(203,sym->name);       /* symbol isn't used: ... */
      break;
    default:
      /* a variable */
      if (sym->parent!=NULL)
        break;                      /* hierarchical data type */
      if ((sym->usage & (uWRITTEN | uREAD | uSTOCK | uPUBLIC))==0)
        error(203,sym->name);       /* symbol isn't used (and not stock or public) */
      else if ((sym->usage & (uREAD | uSTOCK | uPUBLIC))==0)
        error(204,sym->name);       /* value assigned to symbol is never used */
#if 0 // ??? uWRITTEN bit is not set in all circumstances, more testing is needed
      else if ((sym->usage & (uWRITTEN | uPUBLIC | uCONST))==0 && sym->ident==iREFARRAY)
        error(214,sym->name);       /* make array argument "const" */
#endif
    } /* if */
    sym=sym->next;
  } /* while */

  return entry;
}

static constval *insert_constval(constval *prev,constval *next,char *name,cell val)
{
  constval *eq;

  if ((eq=(constval*)malloc(sizeof(constval)))==NULL)
    error(103);       /* insufficient memory (fatal error) */
  memset(eq,0,sizeof(constval));
  strcpy(eq->name,name);
  eq->value=val;
  eq->next=next;
  prev->next=eq;
  return eq;
}

SC_FUNC constval *append_constval(constval *table,char *name,cell val)
{
  constval *eq,*prev;

  /* find the end of the constant table */
  for (prev=table, eq=table->next; eq!=NULL; prev=eq, eq=eq->next)
    /* nothing */;
  return insert_constval(prev,NULL,name,val);
}

SC_FUNC constval *find_constval(constval *table,char *name)
{
  constval *ptr = table->next;

  while (ptr!=NULL) {
    if (strcmp(name,ptr->name)==0)
      return ptr;
    ptr=ptr->next;
  } /* while */
  return NULL;
}

static constval *find_constval_byval(constval *table,cell val)
{
  constval *ptr = table->next;

  while (ptr!=NULL) {
    if (ptr->value==val)
      return ptr;
    ptr=ptr->next;
  } /* while */
  return NULL;
}

#if 0
static int delete_constval(constval *table,char *name)
{
  constval *prev = table;
  constval *cur = prev->next;

  while (cur!=NULL) {
    if (strcmp(name,cur->name)==0) {
      prev->next=cur->next;
      free(cur);
      return TRUE;
    } /* if */
    prev=cur;
    cur=cur->next;
  } /* while */
  return FALSE;
}
#endif

SC_FUNC void delete_consttable(constval *table)
{
  constval *eq=table->next, *next;

  while (eq!=NULL) {
    next=eq->next;
    free(eq);
    eq=next;
  } /* while */
  memset(table,0,sizeof(constval));
}

/*  add_constant
 *
 *  Adds a symbol to the #define symbol table.
 */
SC_FUNC void add_constant(char *name,cell val,int vclass,int tag)
{
  symbol *sym;

  /* Test whether a global or local symbol with the same name exists. Since
   * constants are stored in the symbols table, this also finds previously
   * defind constants. */
  sym=findglb(name);
  if (!sym)
    sym=findloc(name);
  if (sym) {
    /* silently ignore redefinitions of constants with the same value */
    if (sym->ident==iCONSTEXPR) {
      if (sym->addr!=val)
        error(201,name);   /* redefinition of constant (different value) */
    } else {
      error(21,name);      /* symbol already defined */
    } /* if */
    return;
  } /* if */

  /* constant doesn't exist yet, an entry must be created */
  sym=addsym(name,val,iCONSTEXPR,vclass,tag,uDEFINE);
  if (sc_status == statIDLE)
    sym->usage |= uPREDEF;
}

/*  statement           - The Statement Parser
 *
 *  This routine is called whenever the parser needs to know what statement
 *  it encounters (i.e. whenever program syntax requires a statement).
 *
 *  Global references: declared, ncmp
 */
static void statement(int *lastindent)
{
  int tok;
  cell val;
  char *st;

  if (!freading) {
    error(36);                  /* empty statement */
    return;
  } /* if */
  errorset(sRESET);

  tok=lex(&val,&st);
  if (tok!='{')
    setline(fline,fcurrent);
  /* lex() has set stmtindent */
  if (lastindent!=NULL && tok!=tLABEL) {
    if (*lastindent>=0 && *lastindent!=stmtindent && !indent_nowarn && sc_tabsize>0)
      error(217);               /* loose indentation */
    *lastindent=stmtindent;
    indent_nowarn=FALSE;        /* if warning was blocked, re-enable it */
  } /* if */
  switch (tok) {
  case 0:
    /* nothing */
    break;
  case tNEW:
    declloc(FALSE);
    lastst=tNEW;
    break;
  case tSTATIC:
    declloc(TRUE);
    lastst=tNEW;
    break;
  case '{':
    if (!matchtoken('}'))       /* {} is the empty statement */
      compound();
    /* "last statement" does not change */
    break;
  case ';':
    error(36);                  /* empty statement */
    break;
  case tIF:
    doif();
    lastst=tIF;
    break;
  case tWHILE:
    dowhile();
    lastst=tWHILE;
    break;
  case tDO:
    dodo();
    lastst=tDO;
    break;
  case tFOR:
    dofor();
    lastst=tFOR;
    break;
  case tSWITCH:
    doswitch();
    lastst=tSWITCH;
    break;
  case tCASE:
  case tDEFAULT:
    error(14);     /* not in switch */
    break;
  case tGOTO:
    dogoto();
    lastst=tGOTO;
    break;
  case tLABEL:
    dolabel();
    lastst=tLABEL;
    break;
  case tRETURN:
    doreturn();
    lastst=tRETURN;
    break;
  case tBREAK:
    dobreak();
    lastst=tBREAK;
    break;
  case tCONTINUE:
    docont();
    lastst=tCONTINUE;
    break;
  case tEXIT:
    doexit();
    lastst=tEXIT;
    break;
  case tASSERT:
    doassert();
    lastst=tASSERT;
    break;
  case tSLEEP:
    dosleep();
    lastst=tSLEEP;
    break;
  case tCONST:
    decl_const(sLOCAL);
    break;
  case tENUM:
    decl_enum(sLOCAL);
    break;
  default:          /* non-empty expression */
    lexpush();      /* analyze token later */
    doexpr(TRUE,TRUE,TRUE,NULL);
    needtoken(tTERM);
    lastst=tEXPR;
  } /* switch */
}

static void compound(void)
{
  cell save_decl;
  int indent=-1;

  save_decl=declared;
  ncmp+=1;                      /* increase compound statement level */
  while (matchtoken('}')==0){   /* repeat until compound statement is closed */
    if (!freading){
      needtoken('}');           /* gives error: "expected token }" */
      break;
    } else {
      statement(&indent);       /* do a statement */
    } /* if */
  } /* while */
  if ((lastst!=tRETURN) && (lastst!=tGOTO))
    modstk((int)(declared-save_decl)*sizeof(cell));  /* delete local variable space */
  testsymbols(&loctab,ncmp,FALSE,TRUE);     /* look for unused block locals */
  declared=save_decl;
  delete_symbols(&loctab,ncmp,FALSE,TRUE);  /* erase local symbols, but retain
                                             * block local labels (within the
                                             * function) */
  ncmp-=1;                      /* decrease compound statement level */
}

/*  doexpr
 *
 *  Global references: stgidx   (referred to only)
 */
static void doexpr(int comma,int chkeffect,int allowarray,int *tag)
{
  int constant,index,ident;
  int localstaging=FALSE;
  cell val;

  if (!staging) {
    stgset(TRUE);               /* start stage-buffering */
    localstaging=TRUE;
    assert(stgidx==0);
  } /* if */
  index=stgidx;
  errorset(sEXPRMARK);
  do {
    sideeffect=FALSE;
    ident=expression(&constant,&val,tag);
    endexpr();
    if (!allowarray && (ident==iARRAY || ident==iREFARRAY))
      error(33,"-unknown-");    /* array must be indexed */
    if (chkeffect && !sideeffect)
      error(215);               /* expression has no effect */
  } while (comma && matchtoken(',')); /* more? */
  errorset(sEXPRRELEASE);
  if (localstaging) {
    stgout(index);
    stgset(FALSE);              /* stop staging */
  } /* if */
}

/*  constexpr
 */
SC_FUNC int constexpr(cell *val,int *tag)
{
  int constant,index;
  cell cidx;

  stgset(TRUE);         /* start stage-buffering */
  stgget(&index,&cidx); /* mark position in code generator */
  errorset(sEXPRMARK);
  expression(&constant,val,tag);
  stgdel(index,cidx);   /* scratch generated code */
  stgset(FALSE);        /* stop stage-buffering */
  if (constant==0)
    error(8);           /* must be constant expression */
  errorset(sEXPRRELEASE);
  return constant;
}

/*  test
 *
 *  In the case a "simple assignment" operator ("=") is used within a test,
 *  the warning "possibly unintended assignment" is displayed. This routine
 *  sets the global variable "intest" to true, it is restored upon termination.
 *  In the case the assignment was intended, use parantheses around the
 *  expression to avoid the warning; primary() sets "intest" to 0.
 *
 *  Global references: intest   (altered, but restored upon termination)
 */
static void test(int label,int parens,int invert)
{
  int index,tok;
  cell cidx;
  value lval;
  int localstaging=FALSE;

  if (!staging) {
    stgset(TRUE);               /* start staging */
    localstaging=TRUE;
    #if !defined NDEBUG
      stgget(&index,&cidx);     /* should start at zero if started locally */
      assert(index==0);
    #endif
  } /* if */

  pushstk((stkitem)intest);
  intest=1;
  if (parens)
    needtoken('(');
  do {
    stgget(&index,&cidx);       /* mark position (of last expression) in
                                 * code generator */
    if (hier14(&lval))
      rvalue(&lval);
    tok=matchtoken(',');
  } while (tok); /* do */
  if (parens)
    needtoken(')');
  if (lval.ident==iARRAY || lval.ident==iREFARRAY) {
    char *ptr=(lval.sym->name!=NULL) ? lval.sym->name : "-unknown-";
    error(33,ptr);              /* array must be indexed */
  } /* if */
  if (lval.ident==iCONSTEXPR) { /* constant expression */
    intest=(int)(long)popstk(); /* restore stack */
    stgdel(index,cidx);
    if (lval.constval) {        /* code always executed */
      error(206);               /* redundant test: always non-zero */
    } else {
      error(205);               /* redundant code: never executed */
      jumplabel(label);
    } /* if */
    if (localstaging) {
      stgout(0);                /* write "jumplabel" code */
      stgset(FALSE);            /* stop staging */
    } /* if */
    return;
  } /* if */
  if (lval.tag!=0 && lval.tag!=sc_addtag("bool"))
    if (check_userop(lneg,lval.tag,0,1,NULL,&lval.tag))
      invert= !invert;          /* user-defined ! operator inverted result */
  if (invert)
    jmp_ne0(label);             /* jump to label if true (different from 0) */
  else
    jmp_eq0(label);             /* jump to label if false (equal to 0) */
  intest=(int)(long)popstk();   /* double typecast to avoid warning with Microsoft C */
  if (localstaging) {
    stgout(0);                  /* output queue from the very beginning (see
                                 * assert() when localstaging is set to TRUE) */
    stgset(FALSE);              /* stop staging */
  } /* if */
}

static void doif(void)
{
  int flab1,flab2;
  int ifindent;

  ifindent=stmtindent;  /* save the indent of the "if" instruction */
  flab1=getlabel();     /* get label number for false branch */
  test(flab1,TRUE,FALSE); /* get expression and branch to flab1 if false */
  statement(NULL);      /* if true, do a statement */
  if (matchtoken(tELSE)==0){  /* if...else ? */
    setlabel(flab1);    /* no, simple if..., print false label */
  } else {
    /* to avoid the "dangling else" error, we want a warning if the "else"
     * has a lower indent than the matching "if" */
    if (stmtindent<ifindent && sc_tabsize>0)
      error(217);       /* loose indentation */
    flab2=getlabel();
    if ((lastst!=tRETURN) && (lastst!=tGOTO))
      jumplabel(flab2);
    setlabel(flab1);    /* print false label */
    statement(NULL);    /* do "else" clause */
    setlabel(flab2);    /* print true label */
  } /* endif */
}

static void dowhile(void)
{
  int wq[wqSIZE];         /* allocate local queue */

  addwhile(wq);           /* add entry to queue for "break" */
  setlabel(wq[wqLOOP]);   /* loop label */
  /* The debugger uses the "line" opcode to be able to "break" out of
   * a loop. To make sure that each loop has a line opcode, even for the
   * tiniest loop, set it below the top of the loop */
  setline(fline,fcurrent);
  test(wq[wqEXIT],TRUE,FALSE); /* branch to wq[wqEXIT] if false */
  statement(NULL);        /* if so, do a statement */
  jumplabel(wq[wqLOOP]);  /* and loop to "while" start */
  setlabel(wq[wqEXIT]);   /* exit label */
  delwhile();             /* delete queue entry */
}

/*
 *  Note that "continue" will in this case not jump to the top of the loop, but
 *  to the end: just before the TRUE-or-FALSE testing code.
 */
static void dodo(void)
{
  int wq[wqSIZE],top;

  addwhile(wq);           /* see "dowhile" for more info */
  top=getlabel();         /* make a label first */
  setlabel(top);          /* loop label */
  statement(NULL);
  needtoken(tWHILE);
  setlabel(wq[wqLOOP]);   /* "continue" always jumps to WQLOOP. */
  setline(fline,fcurrent);
  test(wq[wqEXIT],TRUE,FALSE);
  jumplabel(top);
  setlabel(wq[wqEXIT]);
  delwhile();
  needtoken(tTERM);
}

static void dofor(void)
{
  int wq[wqSIZE],skiplab;
  cell save_decl;
  int save_ncmp,index;
  int *ptr;

  save_decl=declared;
  save_ncmp=ncmp;

  addwhile(wq);
  skiplab=getlabel();
  needtoken('(');
  if (matchtoken(';')==0) {
    /* new variable declarations are allowed here */
    if (matchtoken(tNEW)) {
      /* The variable in expr1 of the for loop is at a
       * 'compound statement' level of it own.
       */
      ncmp++;
      declloc(FALSE); /* declare local variable */
    } else {
      doexpr(TRUE,TRUE,TRUE,NULL);  /* expression 1 */
      needtoken(';');
    } /* if */
  } /* if */
  /* Adjust the "declared" field in the "while queue", in case that
   * local variables were declared in the first expression of the
   * "for" loop. These are deleted in separately, so a "break" or a "continue"
   * must ignore these fields.
   */
  ptr=readwhile();
  assert(ptr!=NULL);
  ptr[wqBRK]=(int)declared;
  ptr[wqCONT]=(int)declared;
  jumplabel(skiplab);               /* skip expression 3 1st time */
  setlabel(wq[wqLOOP]);             /* "continue" goes to this label: expr3 */
  setline(fline,fcurrent);
  /* Expressions 2 and 3 are reversed in the generated code: expression 3
   * precedes expression 2. When parsing, the code is buffered and marks for
   * the start of each expression are insterted in the buffer.
   */
  assert(!staging);
  stgset(TRUE);                     /* start staging */
  assert(stgidx==0);
  index=stgidx;
  stgmark(sSTARTREORDER);
  stgmark(sEXPRSTART+(char)0);      /* mark start of 2nd expression in stage */
  setlabel(skiplab);                /* jump to this point after 1st expression */
  if (matchtoken(';')==0) {
    test(wq[wqEXIT],FALSE,FALSE);   /* expression 2 (jump to wq[wqEXIT] if false) */
    needtoken(';');
  } /* if */
  stgmark(sEXPRSTART+(char)1);      /* mark start of 3th expression in stage */
  if (matchtoken(')')==0) {
    doexpr(TRUE,TRUE,TRUE,NULL);    /* expression 3 */
    needtoken(')');
  } /* if */
  stgmark(sENDREORDER);             /* mark end of reversed evaluation */
  stgout(index);
  stgset(FALSE);                    /* stop staging */
  statement(NULL);
  jumplabel(wq[wqLOOP]);
  setlabel(wq[wqEXIT]);
  delwhile();

  assert(ncmp>=save_ncmp);
  if (ncmp>save_ncmp) {
    /* Clean up the space and the symbol table for the local
     * variable in "expr1".
     */
    modstk((int)(declared-save_decl)*sizeof(cell));
    declared=save_decl;
    delete_symbols(&loctab,ncmp,TRUE,TRUE);
    ncmp=save_ncmp;     /* reset 'compound statement' nesting level */
  } /* if */
}

/* The switch statement is incompatible with its C sibling:
 * 1. the cases are not drop through
 * 2. only one instruction may appear below each case, use a compound
 *    instruction to execute multiple instructions
 * 3. the "case" keyword accepts a comma separated list of values to
 *    match, it also accepts a range using the syntax "1 .. 4"
 *
 * SWITCH param
 *   PRI = expression result
 *   param = table offset (code segment)
 *
 */
static void doswitch(void)
{
  int lbl_table,lbl_exit,lbl_case;
  int tok,swdefault,casecount;
  cell val;
  char *str;
  constval caselist = { "", 0, NULL};   /* case list starts empty */
  constval *cse,*csp;
  char labelname[sNAMEMAX+1];

  needtoken('(');
  doexpr(TRUE,FALSE,FALSE,NULL);/* evaluate switch expression */
  needtoken(')');
  /* generate the code for the switch statement, the label is the address
   * of the case table (to be generated later).
   */
  lbl_table=getlabel();
  lbl_case=0;                   /* just to avoid a compiler warning */
  ffswitch(lbl_table);

  needtoken('{');
  lbl_exit=getlabel();          /* get label number for jumping out of switch */
  swdefault=FALSE;
  casecount=0;
  do {
    tok=lex(&val,&str);         /* read in (new) token */
    switch (tok) {
    case tCASE:
      if (swdefault!=FALSE)
        error(15);        /* "default" case must be last in switch statement */
      lbl_case=getlabel();
      sc_allowtags=FALSE; /* do not allow tagnames here */
      do {
        casecount++;

        /* ??? enforce/document that, in a switch, a statement cannot start
         *     with a label. Then, you can search for:
         *     * the first semicolon (marks the end of a statement)
         *     * an opening brace (marks the start of a compound statement)
         *     and search for the right-most colon before that statement
         *     Now, by replacing the ':' by a special COLON token, you can
         *     parse all expressions until that special token.
         */

        constexpr(&val,NULL);
        /* Search the insertion point (the table is kept in sorted order, so
         * that advanced abstract machines can sift the case table with a
         * binary search). Check for duplicate case values at the same time.
         */
        for (csp=&caselist, cse=caselist.next;
             cse!=NULL && cse->value<val;
             csp=cse, cse=cse->next)
          /* nothing */;
        if (cse!=NULL && cse->value==val)
          error(40,val);        /* duplicate "case" label */
        /* Since the label is stored as a string in the "constval", the
         * size of an identifier must be at least 8, as there are 8
         * hexadecimal digits in a 32-bit number.
         */
        #if sNAMEMAX < 8
          #error Length of identifier (sNAMEMAX) too small.
        #endif
        assert(csp!=NULL);
        assert(csp->next==cse);
        insert_constval(csp,cse,itoh(lbl_case),val);
        if (matchtoken(tDBLDOT)) {
          cell end;
          constexpr(&end,NULL);
          if (end<=val)
            error(50);          /* invalid range */
          while (++val<=end) {
            casecount++;
            /* find the new insertion point */
            for (csp=&caselist, cse=caselist.next;
                 cse!=NULL && cse->value<val;
                 csp=cse, cse=cse->next)
              /* nothing */;
            if (cse!=NULL && cse->value==val)
              error(40,val);    /* duplicate "case" label */
            assert(csp!=NULL);
            assert(csp->next==cse);
            insert_constval(csp,cse,itoh(lbl_case),val);
          } /* if */
        } /* if */
      } while (matchtoken(','));
      needtoken(':');           /* ':' ends the case */
      sc_allowtags=TRUE;  /* reset */
      setlabel(lbl_case);
      statement(NULL);
      jumplabel(lbl_exit);
      break;
    case tDEFAULT:
      if (swdefault!=FALSE)
        error(16);         /* multiple defaults in switch */
      lbl_case=getlabel();
      setlabel(lbl_case);
      needtoken(':');
      swdefault=TRUE;
      statement(NULL);
      /* Jump to lbl_exit, even thouh this is the last clause in the
       * switch, because the jump table is generated between the last
       * clause of the switch and the exit label.
       */
      jumplabel(lbl_exit);
      break;
    case '}':
      /* nothing, but avoid dropping into "default" */
      break;
    default:
      error(2);
      indent_nowarn=TRUE; /* disable this check */
      tok='}';          /* break out of the loop after an error */
    } /* switch */
  } while (tok!='}');

  #if !defined NDEBUG
    /* verify that the case table is sorted (unfortunatly, duplicates can
     * occur; there really shouldn't be duplicate cases, but the compiler
     * may not crash or drop into an assertion for a user error). */
    for (cse=caselist.next; cse!=NULL && cse->next!=NULL; cse=cse->next)
      assert(cse->value <= cse->next->value);
  #endif
  /* generate the table here, before lbl_exit (general jump target) */
  setlabel(lbl_table);
  assert(swdefault==FALSE || swdefault==TRUE);
  if (swdefault==FALSE) {
    /* store lbl_exit as the "none-matched" label in the switch table */
    strcpy(labelname,itoh(lbl_exit));
  } else {
    /* lbl_case holds the label of the "default" clause */
    strcpy(labelname,itoh(lbl_case));
  } /* if */
  ffcase(casecount,labelname,TRUE);
  /* generate the rest of the table */
  for (cse=caselist.next; cse!=NULL; cse=cse->next)
    ffcase(cse->value,cse->name,FALSE);

  setlabel(lbl_exit);
  delete_consttable(&caselist); /* clear list of case labels */
}

static void doassert(void)
{
  int flab1,index;
  cell cidx;
  value lval;

  if ((debug & sCHKBOUNDS)!=0) {
    flab1=getlabel();           /* get label number for "OK" branch */
    test(flab1,FALSE,TRUE);     /* get expression and branch to flab1 if true */
    setline(fline,fcurrent);    /* make sure we abort on the correct line number */
    ffabort(xASSERTION);
    setlabel(flab1);
  } else {
    stgset(TRUE);               /* start staging */
    stgget(&index,&cidx);       /* mark position in code generator */
    do {
      if (hier14(&lval))
        rvalue(&lval);
      stgdel(index,cidx);       /* just scrap the code */
    } while (matchtoken(','));
    stgset(FALSE);              /* stop staging */
  } /* if */
  needtoken(tTERM);
}

static void dogoto(void)
{
  char *st;
  cell val;
  symbol *sym;

  if (lex(&val,&st)==tSYMBOL) {
    sym=fetchlab(st);
    jumplabel((int)sym->addr);
    sym->usage|=uREAD;  /* set "uREAD" bit */
  } else {
    error(20,st);       /* illegal symbol name */
  } /* if */
  needtoken(tTERM);
}

static void dolabel(void)
{
  char *st;
  cell val;
  symbol *sym;

  tokeninfo(&val,&st);  /* retrieve label name again */
  if (find_constval(&tagname_tab,st)!=NULL)
    error(221,st);      /* label name shadows tagname */
  sym=fetchlab(st);
  setlabel((int)sym->addr);
  /* since one can jump around variable declarations or out of compound
   * blocks, the stack must be manually adjusted
   */
  setstk(-declared*sizeof(cell));
  sym->usage|=uDEFINE;  /* label is now defined */
}

/*  fetchlab
 *
 *  Finds a label from the (local) symbol table or adds one to it.
 *  Labels are local in scope.
 *
 *  Note: The "_usage" bit is set to zero. The routines that call "fetchlab()"
 *        must set this bit accordingly.
 */
static symbol *fetchlab(char *name)
{
  symbol *sym;

  sym=findloc(name);            /* labels are local in scope */
  if (sym){
    if (sym->ident!=iLABEL)
      error(19,sym->name);      /* not a label: ... */
  } else {
    sym=addsym(name,getlabel(),iLABEL,sLOCAL,0,0);
    sym->x.declared=(int)declared;
    sym->compound=ncmp;
  } /* if */
  return sym;
}

/*  doreturn
 *
 *  Global references: rettype  (altered)
 */
static void doreturn(void)
{
  int tag;
  if (matchtoken(tTERM)==0){
    if ((rettype & uRETNONE)!=0)
      error(208);                       /* mix "return;" and "return value;" */
    doexpr(TRUE,FALSE,FALSE,&tag);
    needtoken(tTERM);
    rettype|=uRETVALUE;                 /* function returns a value */
    /* check tagname with function tagname */
    assert(curfunc!=NULL);
    if (!matchtag(curfunc->tag,tag,TRUE))
      error(213);                       /* tagname mismatch */
  } else {
    /* this return statement contains no expression */
    const1(0);
    if ((rettype & uRETVALUE)!=0)
      error(209);                       /* function should return a value */
    rettype|=uRETNONE;                  /* function does not return anything */
  } /* if */
  modstk((int)declared*sizeof(cell));   /* end of function, remove *all*
                                         * local variables */
  ffret();
}

static void dobreak(void)
{
  int *ptr;

  ptr=readwhile();      /* readwhile() gives an error if not in loop */
  needtoken(tTERM);
  if (ptr==NULL)
    return;
  modstk(((int)declared-ptr[wqBRK])*sizeof(cell));
  jumplabel(ptr[wqEXIT]);
}

static void docont(void)
{
  int *ptr;

  ptr=readwhile();      /* readwhile() gives an error if not in loop */
  needtoken(tTERM);
  if (ptr==NULL)
    return;
  modstk(((int)declared-ptr[wqCONT])*sizeof(cell));
  jumplabel(ptr[wqLOOP]);
}

static void exporttag(int tag)
{
  /* find the tag by value in the table, then set the top bit to mark it
   * "public"
   */
  if (tag!=0) {
    constval *ptr;
    assert((tag & PUBLICTAG)==0);
    for (ptr=tagname_tab.next; ptr!=NULL && tag!=(int)(ptr->value & TAGMASK); ptr=ptr->next)
      /* nothing */;
    if (ptr!=NULL)
      ptr->value |= PUBLICTAG;
  } /* if */
}

static void doexit(void)
{
  int tag=0;

  if (matchtoken(tTERM)==0){
    doexpr(TRUE,FALSE,FALSE,&tag);
    needtoken(tTERM);
  } else {
    const1(0);
  } /* if */
  const2(tag);
  exporttag(tag);
  ffabort(xEXIT);
}

static void dosleep(void)
{
  int tag=0;

  if (matchtoken(tTERM)==0){
    doexpr(TRUE,FALSE,FALSE,&tag);
    needtoken(tTERM);
  } else {
    const1(0);
  } /* if */
  const2(tag);
  exporttag(tag);
  ffabort(xSLEEP);
}

static void addwhile(int *ptr)
{
  int k;

  ptr[wqBRK]=(int)declared;     /* stack pointer (for "break") */
  ptr[wqCONT]=(int)declared;    /* for "continue", possibly adjusted later */
  ptr[wqLOOP]=getlabel();
  ptr[wqEXIT]=getlabel();
  if (wqptr>=(wq+wqTABSZ-wqSIZE))
    error(102,"loop table");    /* loop table overflow (too many active loops)*/
  k=0;
  while (k<wqSIZE){     /* copy "ptr" to while queue table */
    *wqptr=*ptr;
    wqptr+=1;
    ptr+=1;
    k+=1;
  } /* while */
}

static void delwhile(void)
{
  if (wqptr>wq)
    wqptr-=wqSIZE;
}

static int *readwhile(void)
{
  if (wqptr<=wq){
    error(24);          /* out of context */
    return NULL;
  } else {
    return (wqptr-wqSIZE);
  } /* if */
}

