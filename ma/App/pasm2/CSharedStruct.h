//////////////////////////////////////////////////////////////////////////////////////
// CSharedStruct.h - got this code off of www.codeguru.com
//
// Author: Michael Starich   
//////////////////////////////////////////////////////////////////////////////////////
// THIS CODE IS PROPRIETARY PROPERTY OF SWINGIN' APE STUDIOS, INC.
// Copyright (c) 2002
//
// The contents of this file may not be disclosed to third
// parties, copied or duplicated in any form, in whole or in part,
// without the prior written permission of Swingin' Ape Studios, Inc.
//////////////////////////////////////////////////////////////////////////////////////
// Modification History:
//
// Date     Who         Description
// -------- ----------  --------------------------------------------------------------
// 01/03/02 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#if !defined(AFX_CSharedStruct_H__86467BA6_5AFA_11D3_863D_00A0244A9CA7__INCLUDED_)
#define AFX_CSharedStruct_H__86467BA6_5AFA_11D3_863D_00A0244A9CA7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Aclapi.h"

template <class StructType>
class CSharedStruct
{
private:
   HANDLE m_hFileMapping;
   char   m_hSharedName[MAX_PATH];
   DWORD  m_dwMaxDataSize;
   StructType *m_pvData;

public:
   CSharedStruct();
   CSharedStruct(char *Name);
   ~CSharedStruct();

   VOID Release();
   BOOL Acquire(char *Name);

   StructType *operator->();

   BOOL m_bCreated;
};


template <class StructType>
StructType *CSharedStruct<StructType>::operator->()
{
   return m_pvData;
}

template <class StructType>
CSharedStruct<StructType>::CSharedStruct()
{
   m_hFileMapping = NULL;
   m_bCreated = FALSE;
}

template <class StructType>
CSharedStruct<StructType>::~CSharedStruct()
{
	Release();
}


template <class StructType>
CSharedStruct<StructType>::CSharedStruct(char *Name)
{
	//CSharedStruct();
	m_hFileMapping = NULL;
	m_bCreated = FALSE;
	Acquire(Name);
}

template <class StructType>
VOID CSharedStruct<StructType>::Release()
{
   if (m_pvData)
   {
      UnmapViewOfFile(m_pvData);
      m_pvData = NULL;
   }

   if (m_hFileMapping)
   {
      CloseHandle(m_hFileMapping);
	  m_hFileMapping = NULL;
   }
   m_bCreated = FALSE;
}

template <class StructType>
BOOL CSharedStruct<StructType>::Acquire( char *Name ) {

   m_dwMaxDataSize = 0;
   m_hFileMapping = CreateFileMapping( INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(StructType), Name );
   int err = GetLastError();
   if( m_hFileMapping == NULL ) {      
      return FALSE;
   }

   // OK, if we're the first person to create this
   // file mapping object
   // we want to clear the ACL, so
   // anyone else can access this object
   //
   // If we don't do this, and a service creates the file mapping
   // User processes won't be able to access it.

   // So, we set the DACL to NULL, which effectively
   // grants Everyone All Access

   // More complicated ACLs are left as an exercise for the reader
   // (because I sure as hell can't figure them out!)

   SetNamedSecurityInfo( Name, SE_KERNEL_OBJECT, DACL_SECURITY_INFORMATION, 0, 0, (PACL) NULL, NULL );

   m_dwMaxDataSize = sizeof( StructType );
   strncpy( m_hSharedName, Name, MAX_PATH - 1 );
   m_pvData = (StructType *)MapViewOfFile( m_hFileMapping, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0 );
   if( m_pvData == NULL ) {
      CloseHandle( m_hFileMapping );
	  return FALSE;
   }

   if( err != ERROR_ALREADY_EXISTS ) {
		m_bCreated = TRUE;
   }
	
   return TRUE;
}

#endif