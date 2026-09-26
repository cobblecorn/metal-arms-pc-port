//////////////////////////////////////////////////////////////////////////////////////
// CSVFile.h - Comma separated value spreadsheet parser 
//             Built from code originally written by Yap Chun Wei
//
// Author: Russell A. Foushee   
//////////////////////////////////////////////////////////////////////////////////////
// THIS CODE IS PROPRIETARY PROPERTY OF SWINGIN' APE STUDIOS, INC.
// Copyright (c) 2003
//
// The contents of this file may not be disclosed to third
// parties, copied or duplicated in any form, in whole or in part,
// without the prior written permission of Swingin' Ape Studios, Inc.
//////////////////////////////////////////////////////////////////////////////////////
// Modification History:
//
// Date     Who         Description
// -------- ----------  --------------------------------------------------------------
// 02/10/03 Foushee     Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _CSVFILE_H_
#define _CSVFILE_H_ 1

#include <afxtempl.h>

class CCSVFile
{
public:
	CCSVFile(CString File, bool Backup = true); // Open spreadsheet for reading and writing
	~CCSVFile(); // Perform some cleanup functions
	bool AddHeaders(CArray<CStringW, CStringW &> &FieldNames, bool replace = false); // Add header row to spreadsheet
	bool DeleteSheet(); // Clear text delimited file content
	bool AddRow(CArray<CStringW, CStringW &> &RowValues, long row = 0, bool replace = false); // Insert or replace a row into spreadsheet. Default is add new row. 
	bool AddCell(CStringW CellValue, CString column, long row = 0, bool Auto = true); // Replace or add a cell into Excel spreadsheet using header row or column alphabet. Default is add cell into new row. Set Auto to false if want to force column to be used as header name
	bool AddCell(CStringW CellValue, short column, long row = 0); // Replace or add a cell into spreadsheet using column number. Default is add cell into new row. 
	bool ReadRow(CArray<CStringW, CStringW &> &RowValues, long row = 0); // Read a row from spreadsheet. Default is read the next row
	bool ReadColumn(CArray<CStringW, CStringW &> &ColumnValues, CString column, bool Auto = true); // Read a column from Excel spreadsheet using header row or column alphabet. Set Auto to false if want to force column to be used as header name
	bool ReadColumn(CArray<CStringW, CStringW &> &ColumnValues, short column); // Read a column from spreadsheet using column number
	bool ReadCell (CStringW &CellValue, CString column, long row = 0, bool Auto = true); // Read a cell from Excel spreadsheet using header row or column alphabet. Default is read the next cell in next row. Set Auto to false if want to force column to be used as header name
	bool ReadCell (CStringW &CellValue, short column, long row = 0); // Read a cell from spreadsheet using column number. Default is read the next cell in next row.
	void BeginTransaction(); // Begin transaction
	bool Commit(); // Save changes to spreadsheet
	bool RollBack(); // Undo changes to spreadsheet
	inline void GetFieldNames (CArray<CStringW, CStringW &> &FieldNames) {FieldNames.RemoveAll(); FieldNames.Copy(m_aFieldNames);} // Get the header row from spreadsheet
	inline long GetTotalRows() {return m_dTotalRows;} // Get total number of rows in  spreadsheet
	inline short GetTotalColumns() {return m_dTotalColumns;} // Get total number of columns in  spreadsheet
	inline long GetCurrentRow() {return m_dCurrentRow;} // Get the currently selected row in  spreadsheet
	inline bool GetBackupStatus() {return m_bBackup;} // Get status of backup. True if backup is successful, False if spreadsheet is not backup
	inline bool GetTransactionStatus() {return m_bTransaction;} // Get status of Transaction. True if Transaction is started, False if Transaction is not started or has error in starting
	inline CString GetLastError() {return m_sLastError;} // Get last error message

private:
	bool Open(); // Open a text delimited file for reading or writing
	short CalculateColumnNumber(CString column, bool Auto); // Convert Excel column in alphabet into column number

	bool m_bAppend; // Internal flag to denote newly created spreadsheet or previously created spreadsheet
	bool m_bBackup; // Internal flag to denote status of Backup
	bool m_bTransaction; // Internal flag to denote status of Transaction
	bool m_bUnicode; // Internal flag to indicate this file is a Unicode Encoded File

	long m_dCurrentRow; // Index of current row, starting from 1
	long m_dTotalRows; // Total number of rows in spreadsheet
	short m_dTotalColumns; // Total number of columns in Excel spreadsheet. Largest number of columns in text delimited spreadsheet

	CString m_sFile; // CSV file name
	CStringW m_sSeparator; // Separator in text delimited spreadsheet
	CString m_sLastError; // Last error message

	CArray<CStringW, CStringW &>m_atempArray; // Temporary array for use by functions
	CArray<CStringW, CStringW &>m_aFieldNames; // Header row in spreadsheet
	CArray<CStringW, CStringW &> m_aRows; // Content of all the rows in spreadsheet
};

#endif