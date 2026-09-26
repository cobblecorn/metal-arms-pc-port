/* Since some of these routines go further than those of standard C,
 * they cannot always be implemented with portable C functions. In
 * other words, these routines must be ported to other environments.
 *
 *  Copyright (c) ITB CompuPhase, 1997-2002
 *  This file may be freely used. No warranties of any kind.
 *
 *  Version: $Id: Amxcons.c,v 1.24 2002-02-13 11:49:38+01 thiadmer Exp thiadmer $
 */
#if defined __WIN32__ || defined _WIN32 || defined __MSDOS__
  #include <conio.h>
#endif
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "amx.h"

#if defined __MSDOS__
  #define EOL_CHAR       '\r'
#endif
#if defined __WIN32__ || defined _WIN32
  #define EOL_CHAR       '\r'
#endif
#if !defined EOL_CHAR
  /* if not a "known" operating system, assume Unix */
  #define EOL_CHAR     '\n'
#endif

#if defined NO_DEF_FUNCS
  int amx_printf(char *,...);
  int amx_putchar(int);
  int amx_fflush(void *);
  int amx_getch(void);
  char *amx_gets(char *,int);
#else
  #define amx_printf    printf
  #define amx_putchar   putchar
  #define amx_fflush    fflush
  #define amx_getch     getchar
  #define amx_gets(s,n) fgets(s,n,stdin)
#endif

static int printstring(AMX *amx,cell *cstr,cell *params,int num);

static int dochar(AMX *amx,char ch,cell param)
{
  cell *cptr;

  switch (ch) {
  case '%':
    amx_putchar(ch);
    return 0;
  case 'c':
    amx_GetAddr(amx,param,&cptr);
    amx_putchar((int)*cptr);
    return 1;
  case 'd':
    amx_GetAddr(amx,param,&cptr);
    amx_printf("%ld",(long)*cptr);
    return 1;
#if defined FLOATPOINT
  case 'f': /* 32-bit floating point number */
    amx_GetAddr(amx,param,&cptr);
    amx_printf("%f",*(float*)cptr);
    return 1;
#endif
#if defined FIXEDPOINT
  #define FIXEDMULT 1000
  case 'r': /* 32-bit fixed point number */
    amx_GetAddr(amx,param,&cptr);
    amx_printf("%ld.%03d",(long)(*cptr/FIXEDMULT), (int)((*cptr%FIXEDMULT+FIXEDMULT)%FIXEDMULT));
    return 1;
#endif
  case 's':
    amx_GetAddr(amx,param,&cptr);
    printstring(amx,cptr,NULL,0);
    return 1;
  } /* switch */
  /* error in the string format, try to repair */
  amx_putchar(ch);
  return 0;
}

static int printstring(AMX *amx,cell *cstr,cell *params,int num)
{
  int i;
  int informat=0,paramidx=0;

  /* check whether this is a packed string */
  if ((ucell)*cstr>UCHAR_MAX) {
    int j=sizeof(cell)-sizeof(char);
    char c;
    /* the string is packed */
    i=0;
    for ( ; ; ) {
      c=(char)((ucell)cstr[i] >> 8*j);
      if (c==0)
        break;
      if (informat) {
        assert(params!=NULL);
        paramidx+=dochar(amx,c,params[paramidx]);
        informat=0;
      } else if (params!=NULL && c=='%') {
        informat=1;
      } else {
        amx_putchar(c);
      } /* if */
      if (j==0)
        i++;
      j=(j+sizeof(cell)-sizeof(char)) % sizeof(cell);
    } /* for */
  } else {
    /* the string is unpacked */
    for (i=0; cstr[i]!=0; i++) {
      if (informat) {
        assert(params!=NULL);
        paramidx+=dochar(amx,(char)cstr[i],params[paramidx]);
        informat=0;
      } else if (params!=NULL && (int)cstr[i]=='%') {
        if (paramidx<num)
          informat=1;
        else
          amx_RaiseError(amx, AMX_ERR_NATIVE);
      } else {
        amx_putchar((int)cstr[i]);
      } /* if */
    } /* for */
  } /* if */
  return paramidx;
}

static cell AMX_NATIVE_CALL _print(AMX *amx,cell *params)
{
  cell *cstr;

  /* do the colour codes with ANSI strings */
  if (params[2]>=0)
    amx_printf("\x1b[%dm",(int)params[2]+30);
  if (params[3]>=0)
    amx_printf("\x1b[%dm",(int)params[3]+40);

  amx_GetAddr(amx,params[1],&cstr);
  printstring(amx,cstr,NULL,0);

  /* reset the colours */
  if (params[2]>=0 || params[3]>=0)
    amx_printf("\x1b[37;40m");

  amx_fflush(stdout);
  return 0;
}

static cell AMX_NATIVE_CALL _printf(AMX *amx,cell *params)
{
  cell *cstr;

  amx_GetAddr(amx,params[1],&cstr);
  printstring(amx,cstr,params+2,(int)(params[0]/sizeof(cell))-1);
  amx_fflush(stdout);
  return 0;
}

#if defined __BORLANDC__ || defined __WATCOMC__
  #pragma argsused
#endif
static cell AMX_NATIVE_CALL _getchar(AMX *amx,cell *params)
{
  int c;

  c=amx_getch();
  if (params[1]) {
    amx_putchar('\n');
    amx_fflush(stdout);
  } /* if */
  return c;
}

static cell AMX_NATIVE_CALL _getstring(AMX *amx,cell *params)
{
  int c,chars;
  char *str;
  cell *cptr;

  if (params[2]<=0)
    return 0;
  str=(char *)malloc((int)params[2]);
  if (str==NULL)
    return 0;

  chars=0;
  c=amx_getch();
  while (c!=EOF && c!=EOL_CHAR && chars<params[2]) {
    str[chars]=(char)c;
    amx_putchar(c);
    amx_fflush(stdout);
    chars++;            /* one more character done */
    c=amx_getch();
  } /* while */

  if (c==EOL_CHAR)
    amx_putchar('\n');
  str[chars]='\0';

  amx_GetAddr(amx,params[1],&cptr);
  amx_SetString(cptr,str,(int)params[3]);

  free(str);
  return chars;
}

static void acceptchar(int c,int *num)
{
  switch (c) {
  case '\b':
    amx_putchar('\b');
    *num-=1;
    #if defined __BORLANDC__ || defined __WATCOMC__
      /* the backspace key does not erase the
       * character, so do this explicitly */
      amx_putchar(' ');     /* erase */
      amx_putchar('\b');    /* go back */
    #endif
    break;
  case EOL_CHAR:
    amx_putchar('\n');
    *num+=1;
    break;
  default:
    amx_putchar(c);
    *num+=1;
  } /* switch */
  amx_fflush(stdout);
}

static int inlist(AMX *amx,int c,cell *params,int num)
{
  int i, key;

  for (i=0; i<num; i++) {
    if (i==0) {
      /* first key is passed by value, others are passed by reference */
      key = (int)params[i];
    } else {
      cell *cptr;
      amx_GetAddr(amx,params[i],&cptr);
      key=(int)*cptr;
    } /* if */
    if (c==key || c==-key)
      return key;
  } /* for */
  return 0;
}

static cell AMX_NATIVE_CALL _getvalue(AMX *amx,cell *params)
{
  cell value;
  int base,sign,c,d;
  int chars,n;

  base=(int)params[1];
  if (base<2 || base>36)
    return 0;

  chars=0;
  value=0;
  sign=1;       /* to avoid a compiler warning (Microsoft Visual C/C++ 6.0) */

  c=amx_getch();
  while (c!=EOF) {
    /* check for sign (if any) */
    if (chars==0) {
      if (c=='-') {
        sign=-1;
        acceptchar(c,&chars);
        c=amx_getch();
      } else {
        sign=1;
      } /* if */
    } /* if */

    /* check end of input */
    #if defined LINUX
      if (c=='\n' && inlist(amx,'\r',params+2,(int)params[0]/sizeof(cell)-1)!=0)
        c='\r';
    #endif
    if ((chars>1 || chars>0 && sign>0)
        && (n=inlist(amx,c,params+2,(int)params[0]/sizeof(cell)-1))!=0)
    {
      if (n>0)
        acceptchar(c,&chars);
      break;
    } /* if */

    /* get value */
    d=base;     /* by default, do not accept the character */
    if (c>='0' && c<='9') {
      d=c-'0';
    } else if (c>='a' && c<='z') {
      d=c-'a'+10;
    } else if (c>='A' && c<='Z') {
      d=c-'A'+10;
    } else if (c=='\b') {
      if (chars>0) {
        value/=base;
        acceptchar(c,&chars);
      } /* if */
    } /* if */
    if (d<base) {
      acceptchar(c,&chars);
      value=value*base + d;
    } /* if */
    c=amx_getch();
  } /* while */
  return sign*value;
}

AMX_NATIVE_INFO console_Natives[] = {
  { "getchar",   _getchar },
  { "getstring", _getstring },
  { "getvalue",  _getvalue },
  { "print",     _print },
  { "printf",    _printf },
  { NULL, NULL }        /* terminator */
};

