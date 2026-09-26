// ItemInfo.h: interface for the CItemInfo class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ITEMINFO_H__0104E656_C0A0_11D1_B974_425F59000000__INCLUDED_)
#define AFX_ITEMINFO_H__0104E656_C0A0_11D1_B974_425F59000000__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

class CItemInfo  
{
public:
	CItemInfo( int nItem) : m_nItem( nItem ) {};
	virtual ~CItemInfo() {};
	
	int GetItem() { return m_nItem; }

private:

	int m_nItem;
};

#endif // !defined(AFX_ITEMINFO_H__0104E656_C0A0_11D1_B974_425F59000000__INCLUDED_)
