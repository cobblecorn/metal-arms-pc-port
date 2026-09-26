//////////////////////////////////////////////////////////////////////////////////////
// CSVFile.cpp - Comma separated value spreadsheet parser 
//               Built from code originally written by Yap Chun Wei
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

#include "stdafx.h"
#include "CSVFile.h"
#include "UnicodeFile.h"

// Open spreadsheet for reading and writing
CCSVFile::CCSVFile(CString File, bool Backup) :
m_sFile(File), m_dTotalRows(0), m_dTotalColumns(0), m_dCurrentRow(1),
m_bAppend(false), m_bBackup(Backup), m_bTransaction(false), m_bUnicode(false)
{
	m_sSeparator = ",";

	if (Open())
	{
		if ((m_bBackup) && (m_bAppend))
		{
			CString sTempString;
			CString sBackupString;
			sTempString = m_sFile;
			sBackupString.Format("%s.bak", m_sFile);
			m_sFile = sBackupString;
			if (!Commit())
			{
				m_bBackup = false;
			}
			m_sFile = sTempString;
		}
	}
}

// Perform some cleanup functions
CCSVFile::~CCSVFile()
{
}

// Add header row to spreadsheet
bool CCSVFile::AddHeaders(CArray<CStringW, CStringW &> &FieldNames, bool replace)
{
	if (m_bAppend) // Append to old Sheet
	{
		if (replace) // Replacing header row rather than adding new columns
		{
			if (!AddRow(FieldNames, 1, true))
			{
				return false;
			}
			else
			{
				return true;
			}
		}

		if (ReadRow(m_atempArray, 1)) // Add new columns
		{
			m_atempArray.Append(FieldNames);
			if (!AddRow(m_atempArray, 1, true))
			{
				m_sLastError = "Problems with adding headers\n";
				return false;
			}

			// Update largest number of columns if necessary
			if (m_atempArray.GetSize() > m_dTotalColumns)
			{
				m_dTotalColumns = m_atempArray.GetSize();
			}
			return true;
		}
		return false;				
	}
	else // New Sheet
	{
		m_dTotalColumns = FieldNames.GetSize();
		if (!AddRow(FieldNames, 1, true))
		{
			return false;
		}
		else
		{
			m_dTotalRows = 1;
			return true;
		}
	}
}

// Clear text delimited file content
bool CCSVFile::DeleteSheet()
{
	m_aRows.RemoveAll();
	m_aFieldNames.RemoveAll();
	m_dTotalColumns = 0;
	m_dTotalRows = 0;
	if (!m_bTransaction)
	{
		Commit();			
	}
	m_bAppend = false; // Set flag to new sheet
	return true;		
}

// Insert or replace a row into spreadsheet. 
// Default is add new row.
bool CCSVFile::AddRow(CArray<CStringW, CStringW &> &RowValues, long row, bool replace)
{
	long tempRow;
	
	if (row == 1)
	{
		// Update header row
		m_aFieldNames.RemoveAll();
		m_aFieldNames.Copy(RowValues);
	}

	// Update largest number of columns if necessary
	if (RowValues.GetSize() > m_dTotalColumns)
	{
		m_dTotalColumns = RowValues.GetSize();
	}

	// Convert row values
	CStringW sTempString;
	CStringW sTempSql;
	for (int i = 0; i < RowValues.GetSize(); i++)
	{
		if (i != RowValues.GetSize()-1) // Not last column
		{
			sTempSql.Format(L"\"%s\"%s", RowValues.GetAt(i), m_sSeparator);
			sTempString += sTempSql;
		}
		else // Last column
		{
			sTempSql.Format(L"\"%s\"", RowValues.GetAt(i));
			sTempString += sTempSql;
		}
	}
	
	if (row)
	{
		if (row <= m_dTotalRows) // Not adding new rows
		{
			if (replace) // Replacing row
			{
				m_aRows.SetAt(row-1, sTempString);
			}
			else // Inserting row
			{
				m_aRows.InsertAt(row-1, sTempString);
				m_dTotalRows++;
			}

			if (!m_bTransaction)
			{
				Commit();
			}
			return true;
		}
		else // Adding new rows
		{
			// Insert null rows until specified row
			m_dCurrentRow = m_dTotalRows;
			sTempSql.Empty();
			CStringW nullString;
			for (int i = 1; i <= m_dTotalColumns; i++)
			{
				if (i != m_dTotalColumns)
				{
					nullString.Format(L"\"\"%s", m_sSeparator);
					sTempSql += nullString;
				}
				else
				{
					sTempSql += L"\"\"";
				}
			}
			for (int j = m_dTotalRows + 1; j < row; j++)
			{
				m_dCurrentRow++;
				m_aRows.Add(sTempSql);
			}
		}
	}
	else
	{
		tempRow = m_dCurrentRow;
		m_dCurrentRow = m_dTotalRows;
	}

	// Insert new row
	m_dCurrentRow++;
	m_aRows.Add(sTempString);
	
	if (row > m_dTotalRows)
	{
		m_dTotalRows = row;
	}
	else if (!row)
	{
		m_dTotalRows = m_dCurrentRow;
		m_dCurrentRow = tempRow;
	}
	if (!m_bTransaction)
	{
		Commit();
	}
	return true;
}

// Replace or add a cell into Excel spreadsheet using header row or column alphabet. 
// Default is add cell into new row.
// Set Auto to false if want to force column to be used as header name
bool CCSVFile::AddCell(CStringW CellValue, CString column, long row, bool Auto)
{
	short columnIndex = CalculateColumnNumber(column, Auto);
	if (columnIndex == 0)
	{
		return false;
	}

	if (AddCell(CellValue, columnIndex, row))
	{
		return true;
	}
	return false;
}

// Replace or add a cell into spreadsheet using column number
// Default is add cell into new row.
bool CCSVFile::AddCell(CStringW CellValue, short column, long row)
{
	CStringW sTempString;
	CStringW sTempSql;

	if (column == 0)
	{
		m_sLastError = "Column cannot be zero\n";
		return false;
	}

	long tempRow;

	// Update largest number of columns if necessary
	if (column > m_dTotalColumns)
	{
		m_dTotalColumns = column;
	}

	if (row)
	{
		if (row <= m_dTotalRows)
		{
			ReadRow(m_atempArray, row);
	
			// Change desired row
			m_atempArray.SetAtGrow(column-1, CellValue);

			if (row == 1)
			{
				// Update header row
				m_aFieldNames.RemoveAll();
				m_aFieldNames.Copy(m_atempArray);
			}	

			if (!AddRow(m_atempArray, row, true))
			{
				return false;
			}

			if (!m_bTransaction)
			{
				Commit();
			}
			return true;
		}
		else
		{
			// Insert null rows until specified row
			m_dCurrentRow = m_dTotalRows;
			sTempSql.Empty();
			CStringW nullString;
			for (int i = 1; i <= m_dTotalColumns; i++)
			{
				if (i != m_dTotalColumns)
				{
					nullString.Format(L"\"\"%s", m_sSeparator);
					sTempSql += nullString;
				}
				else
				{
					sTempSql += L"\"\"";
				}
			}
			for (int j = m_dTotalRows + 1; j < row; j++)
			{
				m_dCurrentRow++;
				m_aRows.Add(sTempSql);
			}
		}
	}
	else
	{
		tempRow = m_dCurrentRow;
		m_dCurrentRow = m_dTotalRows;
	}

	// Insert cell
	m_dCurrentRow++;
	sTempString.Empty();
	for (int j = 1; j <= m_dTotalColumns; j++)
	{
		if (j != m_dTotalColumns) // Not last column
		{
			if (j != column)
			{
				sTempSql.Format(L"\"\"%s", m_sSeparator);
				sTempString += sTempSql;
			}
			else
			{
				sTempSql.Format(L"\"%s\"%s", CellValue, m_sSeparator);
				sTempString += sTempSql;
			}
		}
		else // Last column
		{
			if (j != column)
			{
				sTempString += L"\"\"";
			}
			else
			{
				sTempSql.Format(L"\"%s\"", CellValue);
				sTempString += sTempSql;
			}
		}
	}	

	m_aRows.Add(sTempString);
	
	if (row > m_dTotalRows)
	{
		m_dTotalRows = row;
	}
	else if (!row)
	{
		m_dTotalRows = m_dCurrentRow;
		m_dCurrentRow = tempRow;
	}
	if (!m_bTransaction)
	{
		Commit();
	}
	return true;
}

// Read a row from spreadsheet. 
// Default is read the next row
bool CCSVFile::ReadRow(CArray<CStringW, CStringW &> &RowValues, long row)
{
	CStringW sTempString;
	CStringW sTempSql;

	// Check if row entered is more than number of rows in sheet
	if (row <= m_aRows.GetSize())
	{
		if (row != 0)
		{
			m_dCurrentRow = row;
		}
		else if (m_dCurrentRow > m_aRows.GetSize())
		{
			return false;
		}
		// Read the desired row
		RowValues.RemoveAll();
		sTempString = m_aRows.GetAt(m_dCurrentRow-1);
		m_dCurrentRow++;

		// Search for separator to split row
		int separatorPosition;
		sTempSql.Format(L"\"%s\"", m_sSeparator);
		separatorPosition = sTempString.Find(sTempSql); // If separator is "?"
		if (separatorPosition != -1)
		{
			// Save columns
			int nCount = 0;
			int stringStartingPosition = 0;
			while (separatorPosition != -1)
			{
				nCount = separatorPosition - stringStartingPosition;
				RowValues.Add(sTempString.Mid(stringStartingPosition, nCount));
				stringStartingPosition = separatorPosition + sTempSql.GetLength();
				separatorPosition = sTempString.Find(sTempSql, stringStartingPosition);
			}
			nCount = sTempString.GetLength() - stringStartingPosition;
			RowValues.Add(sTempString.Mid(stringStartingPosition, nCount));

			// Remove quotes from first column
			sTempString = RowValues.GetAt(0);
			sTempString.Delete(0, 1);
			RowValues.SetAt(0, sTempString);
			
			// Remove quotes from last column
			sTempString = RowValues.GetAt(RowValues.GetSize()-1);
			sTempString.Delete(sTempString.GetLength()-1, 1);
			RowValues.SetAt(RowValues.GetSize()-1, sTempString);

			return true;
		}
		else
		{
			// Save columns
			separatorPosition = sTempString.Find(m_sSeparator); // if separator is ?
			if (separatorPosition != -1)
			{
				int nCount = 0;
				int stringStartingPosition = 0;
				while (separatorPosition != -1)
				{
					nCount = separatorPosition - stringStartingPosition;
					RowValues.Add(sTempString.Mid(stringStartingPosition, nCount));
					stringStartingPosition = separatorPosition + m_sSeparator.GetLength();
					separatorPosition = sTempString.Find(m_sSeparator, stringStartingPosition);
				}
				nCount = sTempString.GetLength() - stringStartingPosition;
				RowValues.Add(sTempString.Mid(stringStartingPosition, nCount));
				return true;
			}
			else	// Treat spreadsheet as having one column
			{
				// Remove opening and ending quotes if any
				int quoteBegPos = sTempString.Find(L'\"');
				int quoteEndPos = sTempString.ReverseFind(L'\"');
				if ((quoteBegPos == 0) && (quoteEndPos == sTempString.GetLength()-1))
				{
					sTempString.Delete(0, 1);
					sTempString.Delete(sTempString.GetLength()-1, 1);
				}

				RowValues.Add(sTempString);

				return true;
			}
		}
	}
	m_sLastError = "Desired row is greater than total number of rows in spreadsheet\n";
	return false;
}

// Read a column from Excel spreadsheet using header row or column alphabet. 
// Set Auto to false if want to force column to be used as header name
bool CCSVFile::ReadColumn(CArray<CStringW, CStringW &> &ColumnValues, CString column, bool Auto)
{
	short columnIndex = CalculateColumnNumber(column, Auto);
	if (columnIndex == 0)
	{
		return false;
	}

	if (ReadColumn(ColumnValues, columnIndex))
	{
		return true;
	}
	return false;
}

// Read a column from spreadsheet using column number
bool CCSVFile::ReadColumn(CArray<CStringW, CStringW &> &ColumnValues, short column)
{
	if (column == 0)
	{
		m_sLastError = "Column cannot be zero\n";
		return false;
	}

	int tempRow = m_dCurrentRow;
	m_dCurrentRow = 1;
	ColumnValues.RemoveAll();
	for (int i = 1; i <= m_aRows.GetSize(); i++)
	{
		// Read each row
		if (ReadRow(m_atempArray, i))
		{
			// Get value of cell in desired column
			if (column <= m_atempArray.GetSize())
			{
				ColumnValues.Add(m_atempArray.GetAt(column-1));
			}
			else
			{
				CStringW nullString;
				nullString = L"";
				ColumnValues.Add(nullString);
			}
		}
		else
		{
			m_dCurrentRow = tempRow;
			m_sLastError = "Error reading row\n";
			return false;
		}
	}
	m_dCurrentRow = tempRow;
	return true;
}

// Read a cell from Excel spreadsheet using header row or column alphabet. 
// Default is read the next cell in next row. 
// Set Auto to false if want to force column to be used as header name
bool CCSVFile::ReadCell (CStringW &CellValue, CString column, long row, bool Auto)
{
	short columnIndex = CalculateColumnNumber(column, Auto);
	if (columnIndex == 0)
	{
		return false;
	}

	if (ReadCell(CellValue, columnIndex, row))
	{
		return true;
	}
	return false;
}

// Read a cell from spreadsheet using column number. 
// Default is read the next cell in next row.
bool CCSVFile::ReadCell (CStringW &CellValue, short column, long row)
{
	if (column == 0)
	{
		m_sLastError = "Column cannot be zero\n";
		return false;
	}

	int tempRow = m_dCurrentRow;
	if (row)
	{
		m_dCurrentRow = row;
	}
	if (ReadRow(m_atempArray, m_dCurrentRow))
	{
		// Get value of cell in desired column
		if (column <= m_atempArray.GetSize())
		{
			CellValue = m_atempArray.GetAt(column-1);
		}
		else
		{
			CellValue.Empty();
			m_dCurrentRow = tempRow;
			return false;
		}
		m_dCurrentRow = tempRow;
		return true;
	}
	m_dCurrentRow = tempRow;
	m_sLastError = "Error reading row\n";
	return false;
}

// Begin transaction
void CCSVFile::BeginTransaction()
{
	m_bTransaction = true;
}

// Save changes to spreadsheet
bool CCSVFile::Commit()
{
	if( m_bUnicode ) {
		//first, see if this is a unicode file...
		CUnicodeFile oUnicodeFile;
		if(oUnicodeFile.Open( m_sFile, UNICODEFILE_OPENMODE_WRITE ) ) {
			m_aRows.RemoveAll();
			// Read and store all rows in memory
			CStringW sTempStringW;
			for (int i = 0; i < m_aRows.GetSize(); i++)
			{
				sTempStringW = m_aRows.GetAt(i); //convert this wide string into whatever format we are running under (ANSI or UNICODE)
				sTempStringW += L"\n";
				oUnicodeFile.WriteString(sTempStringW);
			}
			m_bTransaction = false;
			return true;
		}
		return false;
	} else {
		try
		{
			CFile *File = NULL;
			File = new CFile(m_sFile, CFile::modeCreate | CFile::modeWrite  | CFile::shareDenyNone);
			if (File != NULL)
			{
				CArchive *Archive = NULL;
				Archive = new CArchive(File, CArchive::store);
				if (Archive != NULL)
				{
					CString sTempString;
					for (int i = 0; i < m_aRows.GetSize(); i++)
					{
						sTempString = m_aRows.GetAt(i); //convert this wide string into whatever format we are running under (ANSI or UNICODE)
						Archive->WriteString(sTempString);
						Archive->WriteString( _T( "\r\n" ) );
					}
					delete Archive;
					delete File;
					m_bTransaction = false;
					return true;
				}
				delete File;
			}
		}
		catch(...)
		{
		}
		m_sLastError = "Error writing file\n";
		return false;
	}
}

// Undo changes to spreadsheet
bool CCSVFile::RollBack()
{
	if (Open())
	{
		m_bTransaction = false;
		return true;
	}
	m_sLastError = "Error in returning to previous state\n";
	return false;
}

// Open a text delimited file for reading
bool CCSVFile::Open()
{
	//first, see if this is a unicode file...
	CUnicodeFile oUnicodeFile;
	if(oUnicodeFile.Open( m_sFile, UNICODEFILE_OPENMODE_READ ) ) {
		m_bUnicode = true;

		m_aRows.RemoveAll();
		// Read and store all rows in memory
		CStringW sTempStringW;
		while( oUnicodeFile.ReadString(sTempStringW) ) {
			m_aRows.Add(sTempStringW);
		}
		ReadRow(m_aFieldNames, 1); // Get field names i.e header row

		// Get total number of rows
		m_dTotalRows = m_aRows.GetSize();

		// Get the largest number of columns
		for (int i = 0; i < m_aRows.GetSize(); i++)
		{
			ReadRow(m_atempArray, i);
			if (m_atempArray.GetSize() > m_dTotalColumns)
			{
				m_dTotalColumns = m_atempArray.GetSize();
			}
		}

		if (m_dTotalColumns != 0)
		{
			m_bAppend = true;
		}
		return true;
	}
	else {
		try
		{
			CFile *File = NULL;
			File = new CFile(m_sFile, CFile::modeRead | CFile::shareDenyNone);
			if (File != NULL)
			{
				CArchive *Archive = NULL;
				Archive = new CArchive(File, CArchive::load);
				if (Archive != NULL)
				{
					m_aRows.RemoveAll();
					// Read and store all rows in memory
					CString sTempString;
					CStringW sTempStringW;
					while( Archive->ReadString(sTempString) ) {
						sTempStringW = sTempString;
						m_aRows.Add(sTempStringW);
					}
					ReadRow(m_aFieldNames, 1); // Get field names i.e header row
					delete Archive;
					delete File;

					// Get total number of rows
					m_dTotalRows = m_aRows.GetSize();

					// Get the largest number of columns
					for (int i = 0; i < m_aRows.GetSize(); i++)
					{
						ReadRow(m_atempArray, i);
						if (m_atempArray.GetSize() > m_dTotalColumns)
						{
							m_dTotalColumns = m_atempArray.GetSize();
						}
					}

					if (m_dTotalColumns != 0)
					{
						m_bAppend = true;
					}
					return true;
				}
				delete File;
			}
		}
		catch(...)
		{
		}
	}
	m_sLastError = "Error in opening file\n";
	return false;
}

// Convert Excel column in alphabet into column number
short CCSVFile::CalculateColumnNumber(CString column, bool Auto)
{
	CStringW sWcolumn = column;

	if (Auto)
	{
		int firstLetter, secondLetter;
		sWcolumn.MakeUpper();

		if (sWcolumn.GetLength() == 1)
		{
			firstLetter = sWcolumn.GetAt(0);
			return (firstLetter - 65 + 1); // 65 is A in ascii
		}
		else if (sWcolumn.GetLength() == 2)
		{
			firstLetter = sWcolumn.GetAt(0);
			secondLetter = sWcolumn.GetAt(1);
			return ((firstLetter - 65 + 1)*26 + (secondLetter - 65 + 1)); // 65 is A in ascii
		}
	}

	// Check if it is a valid field name
	for (int i = 0; i < m_aFieldNames.GetSize(); i++)
	{
		if (!sWcolumn.Compare(m_aFieldNames.GetAt(i)))
		{
			return (i + 1);
		}
	}
	m_sLastError = "Invalid field name or column alphabet\n";
	return 0;	
}