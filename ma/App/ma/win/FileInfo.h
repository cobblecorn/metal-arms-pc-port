/**
 * FileInfo.h 1.3 - Interface for the CFileInfo and CFileInfoArray classes |
 * The classes contained in this file allow to gather recursively file information
 * through directories.
 *
 * Codeguru & friends
 * Coded by Antonio Tejada Lacaci. 1999
 * atejada@espanet.com
 * CRC32 code by Floor A.C. Naaijkens 
 * 
 * Keep this comment if you redistribute this file. And credit where credit's due!
 */

#if !defined(AFX_FILEINFO_H__4C620721_CFC7_11D2_AA42_A54123628E22__INCLUDED_)
#define AFX_FILEINFO_H__4C620721_CFC7_11D2_AA42_A54123628E22__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <afxtempl.h>

// class Stores information about a file in a way like <c CFindFile> does
class CFileInfo 
{  
public:
	/** @access Public members */
	CFileInfo();
	/**
    * @cmember Copy constructor
    * @parm CFileInfo to copy member variables from.
    */
	CFileInfo(const CFileInfo& finf);
	
	/**
    * @cmember Destructor
    */
	~CFileInfo();
	
	/**
    * @cmember Initializes CFileInfo member variables.
    * @parm Values to init member variables.
    * @parm Path of the file the CFileInfo refers to.
    * @parmopt User defined parameter.
    */
	void Create(const WIN32_FIND_DATA* pwfd, const CString strPath, LPARAM lParam = NULL);
	
	/**
    * @cmember Initializes CFileInfo member variables.
    * @parm Absolute path for file or directory
    * @parmopt User defined parameter.
    */
	void Create(const CString strFilePath, LPARAM lParam = NULL);
	
	/**
    * @cmember Calcs 32bit checksum of file(i.e. sum of all the DWORDS of the file, 
    * truncated to 32bit).
    * @parmopt Number of maximum bytes read for checksum calculation. This number is 
    * up - rounded to a multiple of 4 bytes(DWORD). If 0 or bigger than uhFileSize, checksum
    * for all the file is calculated.
    * @parmopt Force recalculation of checksum(otherwise if checksum has already
    * been calculated, it isn't calculated again and previous calculated value is returned).
    * @parmopt Flag to allow calling application to abort the calculation of 
    * checksum(for multithreaded applications).
    * @parmopt Pointer to counter of bytes whose checksum has been calculated. 
    * This value is updated while checksum is being calculated, so calling application
    * can view the progress of checksum calc(for multithreaded applications).
    * Maximum value for pulCount is uhFileSize.
    */
	DWORD GetChecksum(const ULONGLONG uhUpto = 0, const BOOL bRecalc = FALSE, 
					  const volatile BOOL* pbAbort = NULL, volatile ULONG* pulCount = NULL);
	
		/**
		* @cmember Calcs 32bit CRC of file contents(i.e. CRC of all the DWORDS of the file).
		* @parmopt Number of maximum bytes read for CRC calculation. This number is 
		* up - rounded to a multiple of 4 bytes(DWORD). If 0 or bigger than uhFileSize, CRC
		* for all the file is calculated.
		* @parmopt Force recalculation of CRC(otherwise if CRC has already
		* been calculated, it isn't calculated again and previous calculated value is returned).
		* @parmopt pbAbort Flag to allow calling application to abort the calculation of 
		* CRC(for multithreaded applications).
		* @parmopt Pointer to counter of bytes whose CRC has been calculated. 
		* This value is updated while CRC is being calculated, so calling application
		* can view the progress of CRC calc(for multithreaded applications).
		* Maximum value for pulCount is uhFileSize.
    */
	DWORD GetCRC(const ULONGLONG dhUpto = 0, const BOOL bRecalc = FALSE,
				 const volatile BOOL* pbAbort = NULL, volatile ULONG* pulCount = NULL);
	
	/** @cmember File size in bytes as a DWORD value. */
	DWORD GetLength(void) const 
	{
		return (DWORD) m_uhFileSize;
	};
	/** @cmember File size in bytes as an ULONGLONG value. */
	ULONGLONG GetLength64(void) const 
	{
		return m_uhFileSize;
	};
	
	/** Get File split info(equivalent to CFindFile members) */
	
	/** 
    * @cmember Gets the file drive 
    * @rdesc Returns C: for C:\WINDOWS\WIN.INI 
    */
	CString GetFileDrive(void) const;
	/** 
    * @cmember Gets the file dir 
    * @rdesc Returns \WINDOWS\ for C:\WINDOWS\WIN.INI 
    */
	CString GetFileDir(void) const;
	/** @cmember returns WIN for C:\WINDOWS\WIN.INI */
	CString GetFileTitle(void) const;
	/** @cmember returns .INI for C:\WINDOWS\WIN.INI */
	CString GetFileExt(void) const;
	/** @cmember returns C:\WINDOWS\ for C:\WINDOWS\WIN.INI */
	CString GetFileRoot(void) const 
	{
		return GetFileDrive() + GetFileDir();
	};
	/** @cmember returns WIN.INI for C:\WINDOWS\WIN.INI */
	CString GetFileName(void) const 
	{
		return GetFileTitle() + GetFileExt();
	};
	/** @cmember returns C:\WINDOWS\WIN.INI for C:\WINDOWS\WIN.INI */
	const CString& GetFilePath(void) const { return m_strFilePath; }
	
	/* Get File times info(equivalent to CFindFile members) */
	/** @cmember returns creation time */
	const CTime& GetCreationTime(void) const 
	{
		return m_timCreation;
	};
	/** @cmember returns last access time */
	const CTime& GetLastAccessTime(void) const 
	{
		return m_timLastAccess;
	};
	/** @cmember returns las write time */
	const CTime& GetLastWriteTime(void) const 
	{
		return m_timLastWrite;
	};
	
	/* Get File attributes info(equivalent to CFindFile members) */
	/** @cmember returns file attributes */
	DWORD GetAttributes(void) const 
	{
		return m_dwAttributes;
	};
	/** @cmember returns TRUE if the file is a directory */
	BOOL IsDirectory(void) const 
	{
		return m_dwAttributes & FILE_ATTRIBUTE_DIRECTORY;
	};
	/** @cmember Returns TRUE if the file has archive bit set */
	BOOL IsArchived(void) const 
	{
		return m_dwAttributes & FILE_ATTRIBUTE_ARCHIVE;
	};
	/** @cmember Returns TRUE if the file is read - only */
	BOOL IsReadOnly(void) const 
	{
		return m_dwAttributes & FILE_ATTRIBUTE_READONLY;
	};
	/** @cmember Returns TRUE if the file is compressed */
	BOOL IsCompressed(void) const 
	{
		return m_dwAttributes & FILE_ATTRIBUTE_COMPRESSED;
	};
	/** @cmember Returns TRUE if the file is a system file */
	BOOL IsSystem(void) const 
	{
		return m_dwAttributes & FILE_ATTRIBUTE_SYSTEM;
	};
	/** @cmember Returns TRUE if the file is hidden */
	BOOL IsHidden(void) const 
	{
		return m_dwAttributes & FILE_ATTRIBUTE_HIDDEN;
	};
	/** @cmember Returns TRUE if the file is temporary */
	BOOL IsTemporary(void) const 
	{
		return m_dwAttributes & FILE_ATTRIBUTE_TEMPORARY;
	};
	/** @cmember Returns TRUE if the file is a normal file */
	BOOL IsNormal(void) const 
	{
		return m_dwAttributes == 0;
	};
	
	LPARAM m_lParam;        /** User - defined parameter */
private:
	/** @access Private members */
	
	CString m_strFilePath;  /** Full filepath of file(directory + filename) */
	DWORD m_dwAttributes;   /** File attributes of file(as returned by FindFile() */
	ULONGLONG m_uhFileSize; /** File of size.(COM states LONGLONG as hyper, so "uh" means unsigned hyper) */
	CTime m_timCreation;    /** Creation time */
	CTime m_timLastAccess;  /** Last Access time */
	CTime m_timLastWrite;   /** Last write time */
	
	DWORD m_dwChecksum;     /** Checksum calculated for the first m_uhChecksumBytes bytes */
	DWORD m_dwCRC;          /** CRC calculated for the first m_uhCRCBytes bytes */
	DWORD m_uhCRCBytes;     /** Number of file bytes with CRC calc'ed(4 multiple or filesize) */
	DWORD m_uhChecksumBytes;/** Number of file bytes with Checksum calc'ed(4 multiple or filesize) */
}; 

// class Allows to retrieve <c CFileInfo>s from files/directories in a directory
class CFileInfoArray : public CArray<CFileInfo, CFileInfo&> 
{
public:
	CFileInfoArray();
	
	
	// Default values for <md CFileInfoArray.lAddParam>
	enum 
	{ 
		AP_NOSORT = 0,			// Insert <c CFileInfo>s in a unordered manner
		AP_SORTASCENDING = 0,	// Insert <c CFileInfo>s in a ascending order
		AP_SORTDESCENDING = 1,	// Insert <c CFileInfo>s in a descending number

		AP_SORTBYSIZE = 2,		// AP_SORTBYSIZE | Insert <c CFileInfo>s ordered by uhFileSize(presumes array is previously ordered by uhFileSize).
		AP_SORTBYNAME = 4,		// AP_SORTBYNAME | Insert <c CFileInfo>s ordered by strFilePath(presumes array is previously ordered by strFilePath)

		AP_IGNORE_CASE = 8,		// only use if sorting by name and you don't want case to factor in to the comparsion
		AP_COMPARE_FILENAME_ONLY = 16 // use GetFileName instead of GetFilePath when comparing strings
	};
	
	/*
    * Adds a file or all contained in a directory to the CFileInfoArray.
    * Only "static" data for CFileInfo is filled(by default CRC and checksum are NOT 
    * calculated when inserting CFileInfos).
	*
	* Returns the number of <c CFileInfo>s added to the array
    *
	* Name of the directory, ended in backslash.
    * Mask of files to add in case that strDirName is a directory
    * Whether to recurse or not subdirectories
    * Parameter to pass to protected member function AddFileInfo
    * Whether to add or not CFileInfos for directories
    * Pointer to a variable to signal abort of directory retrieval (multithreaded apps).
    * Pointer to a variable incremented each time a CFileInfo is added to the array (multithreaded apps).
    */
	int AddDir( const CString strDirName, const CString strMask, const BOOL bRecurse, 
				LPARAM lAddParam = AP_NOSORT, const BOOL bIncludeDirs = FALSE, 
				const volatile BOOL* pbAbort = NULL, volatile ULONG* pulCount = NULL );
	
	/*
	* Adds a single file or directory to the CFileInfoArray. 
	* In case of directory, files contained in the directory are NOT added to the array.
	*
	* Returns the position in the array where the <c CFileInfo> was added(-1 if <c CFileInfo> wasn't added)
	*
	* Name of the file or directory to add. NOT ended with backslash.
	* Parameter to pass to protected member function AddFileInfo.
    */
	int AddFile( const CString strFilePath, LPARAM lAddParam );

	// removes all CFileInfos of files that have extensions in sExtMask
	//
	// returns the number of CFileInfos remaining in the list
	//
	// sExtMask should be a list of extensions in the following format: *.exe;*.bat;*.txt
	int RemoveExtFiles( const CString sExtMask );
	
	// builds a CMap for quickly looking up files
	//
	// if lAddParam & AP_IGNORE_CASE then strings will be inserted into rMap in lowercase format
	// if lAddParam & AP_COMPARE_FILENAME_ONLY then GetFileName will be used instead of GetFilePath
	static void BuildMapFromFileInfoArray( CMapStringToPtr &rMap, const CFileInfoArray &rFileInfo, LPARAM lAddParam );
	
protected:
	/*
    * Called by AddXXXX to add a CFileInfo to the array.
	* Can be overriden to:
    * 1. Add only desired CFileInfos( filter )
    * 2. Fill user param lParam
    * 3. Change sort order/criteria
    *
	* Returns the position in the array where the CFileInfo was added or -1 if the CFileInfo wasn't added to the array.
    * Default implementation sorts by lAddParam values and adds all CFileInfos (no filtering)
    *
	* CFileInfo to insert in the array.
    * Parameter passed from AddDir function.
    */
	virtual int AddFileInfo(CFileInfo& finf, LPARAM lAddParam);
};


/** EXAMPLES OF USAGE **

@ex This code adds all files in root directory and its subdirectories(but not directories themselves) to the array and TRACEs them: |

CFileInfoArray fia;

fia.AddDir(
   "C:\\",                                     // Directory
   "*.*",                                      // Filemask (all files)
   TRUE,                                       // Recurse subdirs
   fia::AP_SORTBYNAME | fia::AP_SORTASCENDING, // Sort by name and ascending
   FALSE                                       // Do not add array entries for directories (only for files)
);
TRACE("Dumping directory contents\n");
for (int i = 0; i < fia.GetSize(); i++)
TRACE(fia[i].GetFilePath() + "\n");

@ex You can also call AddDir multiple times. The example shows files in root directories(but not subdirectories) of C:\\ and D:\\: |

CFileInfoArray fia;

// Note both AddDir use the same sorting order and direction
fia.AddDir("C:\\", "*.*", FALSE, fia::AP_SORTBYNAME | fia::AP_SORTASCENDING, FALSE);
fia.AddDir("D:\\", "*.*", FALSE, fia::AP_SORTBYNAME | fia::AP_SORTASCENDING, FALSE);
TRACE("Dumping directory contents for C:\\ and D:\\ \n");
for (int i = 0; i < fia.GetSize(); i++)
TRACE(fia[i].GetFilePath() + "\n");


@ex Or you can add individual files: |

CFileInfoArray fin;

// Note both AddDir and AddFile must use the same sorting order and direction
fia.AddDir("C:\\WINDOWS\\", "*.*", FALSE, fia::AP_SORTBYNAME | fia::AP_SORTDESCENDING, FALSE);
fia.AddFile("C:\\AUTOEXEC.BAT", fia::AP_SORTBYNAME | fia::SORTDESCENDING);
TRACE("Dumping directory contents for C:\\WINDOWS\\ and file C:\\AUTOEXEC.BAT\n");
for (int i = 0; i < fia.GetSize(); i++)
TRACE(fia[i].GetFilePath() + "\n");

@ex And mix directories with individual files:  |

CFileInfoArray fin;

// Note both AddDir and AddFile must use the same sorting order and direction
// Note also the list of filemasks *.EXE and *.COM
fia.AddDir("C:\\WINDOWS\\", "*.EXE;*.COM", FALSE, fia::AP_SORTBYNAME | fia::AP_SORTDESCENDING, FALSE);
fia.AddFile("C:\\AUTOEXEC.BAT", fia::AP_SORTBYNAME | fia::SORTDESCENDING);
// Note no trailing bar for next AddFile (we want to insert an entry for the directory
// itself, not for the files inside the directory)
fia.AddFile("C:\\PROGRAM FILES", fia::AP_SORTBYNAME | fia::SORTDESCENDING);
TRACE("Dumping directory contents for C:\\WINDOWS\\, file C:\\AUTOEXEC.BAT and "
" directory \"C:\\PROGRAM FILES\" \n");
for (int i = 0; i < fia.GetSize(); i++)
TRACE(fia[i].GetFilePath + "\n");

*/

#endif // !defined(AFX_FILEINFO_H__4C620721_CFC7_11D2_AA42_A54123628E22__INCLUDED_)
