/*  Small compiler  - maintenance of name list
 *
 *  Name list (aliases)
 *
 *  Copyright (c) ITB CompuPhase, 2001-2002
 *  This file may be freely used. No warranties of any kind.
 *
 *  Version: $Id: sclist.c,v 1.3 2002-02-13 11:52:39+01 thiadmer Exp thiadmer $
 */
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "sc.h"

static namealias alias_tab = { "", "", NULL};  /* alias table */

SC_FUNC namealias *insert_alias(char *name,char *alias)
{
  namealias *cur;

  assert(name!=NULL);
  assert(strlen(name)<=sNAMEMAX);
  assert(alias!=NULL);
  assert(strlen(alias)<=sEXPMAX);
  if ((cur=(namealias*)malloc(sizeof(namealias)))==NULL)
    error(103);       /* insufficient memory (fatal error) */
  memset(cur,0,sizeof(namealias));
  strcpy(cur->name,name);
  strcpy(cur->alias,alias);
  cur->next=alias_tab.next;
  alias_tab.next=cur;
  return cur;
}

SC_FUNC namealias *find_alias(char *name)
{
  namealias *cur = alias_tab.next;

  assert(name!=NULL);
  assert(strlen(name)<=sNAMEMAX);
  while (cur!=NULL) {
    if (strcmp(name,cur->name)==0)
      return cur;
    cur=cur->next;
  } /* while */
  return NULL;
}

SC_FUNC int lookup_alias(char *target,char *name)
{
  namealias *cur = find_alias(name);
  if (cur!=NULL) {
    assert(strlen(cur->alias)<=sEXPMAX);
    strcpy(target,cur->alias);
  } /* if */
  return cur!=NULL;
}

SC_FUNC void delete_aliastable(void)
{
  namealias *cur=alias_tab.next, *next;

  while (cur!=NULL) {
    next=cur->next;
    free(cur);
    cur=next;
  } /* while */
  memset(&alias_tab,0,sizeof(namealias));
}

