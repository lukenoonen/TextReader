//=============== Copyright Luke Noonen, All rights reserved. ===============//
//
// Purpose: Defines the CTextReader class and its component classes,
// CTextBlock, CTextLine, and CTextItem, as well as UTIL functions
// used for extracting selected data types from a CTextItem instance
//
//===========================================================================//

#include "TextReader.h"

//-----------------------------------------------------------------------------
// Purpose:
// Output: true on successful data extraction, false on failure
//-----------------------------------------------------------------------------
bool UTIL_GetValue( const CTextItem *pTextItem, CTextBlock *&pValue )
{
	if (!pTextItem->GetTextBlock())
		return false;

	pValue = pTextItem->GetTextBlock();
	return true;
}

//-----------------------------------------------------------------------------
// Purpose:
// Output: true on successful data extraction, false on failure
//-----------------------------------------------------------------------------
bool UTIL_GetValue( const CTextItem *pTextItem, CTextLine *&pValue )
{
	if (!pTextItem->GetTextLine())
		return false;

	pValue = pTextItem->GetTextLine();
	return true;
}

//-----------------------------------------------------------------------------
// Purpose:
// Output: true on successful data extraction, false on failure
// Note: dynamically allocates and copies string, be sure to delete
//-----------------------------------------------------------------------------
bool UTIL_GetValue( const CTextItem *pTextItem, char *&sValue )
{
	if (!pTextItem->GetString())
		return false;

	sValue = _strdup( pTextItem->GetString() );
	return true;
}

//-----------------------------------------------------------------------------
// Purpose:
// Output: true on successful data extraction, false on failure
// Note: just copies pointer, be sure not to use after CTextReader is out of
//       scope
//-----------------------------------------------------------------------------
bool UTIL_GetValue( const CTextItem *pTextItem, const char *&sValue )
{
	if (!pTextItem->GetString())
		return false;

	sValue = pTextItem->GetString();
	return true;
}

//-----------------------------------------------------------------------------
// Purpose:
// Output: true on successful data extraction, false on failure
// Note: "true" and a non-zero integer value is interpreted as true, "false"
//       and 0 are interpreted as false
//-----------------------------------------------------------------------------
bool UTIL_GetValue( const CTextItem *pTextItem, bool &bValue )
{
	if (!pTextItem->GetString())
		return false;

	const char *sString = pTextItem->GetString();
	if (strcmp( sString, "true" ) == 0)
		bValue = true;
	else if (strcmp( sString, "false" ) == 0)
		bValue = false;
	else
		bValue = (bool)atoi( sString );

	return true;
}
//-----------------------------------------------------------------------------
// Purpose:
// Output: true on successful data extraction, false on failure
//-----------------------------------------------------------------------------
bool UTIL_GetValue( const CTextItem *pTextItem, int &iValue )
{
	if (!pTextItem->GetString())
		return false;

	iValue = atoi( pTextItem->GetString() );
	return true;
}

//-----------------------------------------------------------------------------
// Purpose:
// Output: true on successful data extraction, false on failure
//-----------------------------------------------------------------------------
bool UTIL_GetValue( const CTextItem *pTextItem, float &fValue )
{
	if (!pTextItem->GetString())
		return false;

	fValue = (float)atof( pTextItem->GetString() );
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CTextReader::CTextReader()
{
	m_pTextBlock = NULL;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CTextReader::~CTextReader()
{
	if (m_pTextBlock)
	{
		delete[] m_sText;
		delete m_pTextBlock;
	}
}

//-----------------------------------------------------------------------------
// Purpose:
// Output: true upon successful interpreation of text, false otherwise
//-----------------------------------------------------------------------------
bool CTextReader::ReadText( const char *sText )
{
	if (m_pTextBlock)
	{
		delete[] m_sText;
		delete m_pTextBlock;
	}

	m_sText = _strdup( sText );
	m_pTextBlock = new CTextBlock( m_sText );

	if (!m_pTextBlock->Success())
	{
		delete[] m_sText;
		delete m_pTextBlock;
		m_pTextBlock = NULL;
		return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output: the CTextBlock instance
// Note: If read has failed or nothing has been read, will return NULL
//-----------------------------------------------------------------------------
CTextBlock *CTextReader::GetTextBlock( void ) const
{
	return m_pTextBlock;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CTextBlock::CTextBlock( char *sTextBlock )
{
	m_bSuccess = false;

	char *sChar = sTextBlock;
	while (true)
	{
		sChar = strpbrk( sChar, ";{[\"" );
		if (!sChar)
		{
			if (*(sTextBlock + strspn( sTextBlock, " \t\n" )))
				return;
			else
				break;
		}

		if (*sChar == ';')
		{
			*sChar++ = '\0';
			m_pTextLines.push_back( new CTextLine( sTextBlock ) );
			sTextBlock = sChar;
		}
		else
		{
			switch (*sChar++)
			{
			case '{':
			{
				sChar = strchr( sChar, '}' );
				break;
			}
			case '[':
			{
				sChar = strchr( sChar, ']' );
				break;
			}
			case '\"':
			{
				sChar = strchr( sChar, '\"' );
				break;
			}
			}

			if (!sChar)
				return;

			sChar++;
		}
	}

	m_bSuccess = true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CTextBlock::~CTextBlock()
{
	for (unsigned int i = 0; i < m_pTextLines.size(); i++)
		delete m_pTextLines[i];
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output: true if text was successfully interpreted, false otherwise
//-----------------------------------------------------------------------------
bool CTextBlock::Success( void ) const
{
	if (!m_bSuccess)
		return false;

	for (unsigned int i = 0; i < m_pTextLines.size(); i++)
	{
		if (!m_pTextLines[i]->Success())
			return false;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output: The number of CTextLine instances
//-----------------------------------------------------------------------------
unsigned int CTextBlock::GetTextLineCount( void ) const
{
	return (unsigned int)m_pTextLines.size();
}

//-----------------------------------------------------------------------------
// Purpose:
// Output: the corresponding CTextLine instance for the given index, or NULL
//         if index is invalid
//-----------------------------------------------------------------------------
CTextLine *CTextBlock::GetTextLine( unsigned int uiIndex ) const
{
	if (uiIndex >= m_pTextLines.size())
		return NULL;

	return m_pTextLines[uiIndex];
}

//-----------------------------------------------------------------------------
// Purpose:
// Output: the corresponding CTextLine instance for the given key, or NULL if
//         key does not match any
//-----------------------------------------------------------------------------
CTextLine *CTextBlock::GetTextLine( const char *sKey ) const
{
	for (unsigned int i = 0; i < m_pTextLines.size(); i++)
	{
		CTextLine *pTextLine = m_pTextLines[i];
		if (pTextLine->IsKey( sKey ))
			return pTextLine;
	}

	return NULL;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CTextLine::CTextLine( char *sTextLine )
{
	while (true)
	{
		if (!sTextLine)
			break;

		if (!*sTextLine)
			break;

		char *sChar = sTextLine + strspn( sTextLine, " \t\n" );
		if (!*sChar)
			break;

		char *sCharEnd;
		unsigned char ucActiveData = 2;
		switch (*sChar)
		{
		case '{':
		{
			ucActiveData = 0;
			sCharEnd = strchr( ++sChar, '}' );
			break;
		}
		case '[':
		{
			ucActiveData = 1;
			sCharEnd = strchr( ++sChar, ']' );
			break;
		}
		case '\"':
		{
			sCharEnd = strchr( ++sChar, '\"' );
			break;
		}
		default:
		{
			sCharEnd = strpbrk( sChar, " \t\n" );
			break;
		}
		}

		if (sCharEnd)
			*sCharEnd++ = '\0';

		m_pTextItems.push_back( new CTextItem( sChar, ucActiveData ) );

		sTextLine = sCharEnd;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CTextLine::~CTextLine()
{
	for (unsigned int i = 0; i < m_pTextItems.size(); i++)
		delete m_pTextItems[i];
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output: true if text was successfully interpreted, false otherwise
//-----------------------------------------------------------------------------
bool CTextLine::Success( void ) const
{
	for (unsigned int i = 0; i < m_pTextItems.size(); i++)
	{
		if (!m_pTextItems[i]->Success())
			return false;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output: The number of CTextLine instances
//-----------------------------------------------------------------------------
unsigned int CTextLine::GetTextItemCount( void ) const
{
	return (unsigned int)m_pTextItems.size();
}

//-----------------------------------------------------------------------------
// Purpose:
// Output: the corresponding CTextItem instance for the given index, or NULL 
//         if index is invalid
//-----------------------------------------------------------------------------

CTextItem *CTextLine::GetTextItem( unsigned int uiIndex ) const
{
	if (uiIndex >= m_pTextItems.size())
		return NULL;

	return m_pTextItems[uiIndex];
}

//-----------------------------------------------------------------------------
// Purpose:
// Output: true if the string value of the CTextItem instance at index zero is
//         equal to the key string input, false otherwise
//-----------------------------------------------------------------------------
bool CTextLine::IsKey( const char *sKey ) const
{
	const char *sCompareKey;
	return GetValue( sCompareKey, 0 ) && strcmp( sKey, sCompareKey ) == 0;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CTextItem::CTextItem( char *sTextItem, unsigned char ucActiveData )
{
	m_ucActiveData = ucActiveData;
	switch (m_ucActiveData)
	{
	case 0:
		m_uData.pTextBlock = new CTextBlock( sTextItem );
		break;
	case 1:
		m_uData.pTextLine = new CTextLine( sTextItem );
		break;
	case 2:
		m_uData.sString = sTextItem;
		break;
	}
}
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CTextItem::~CTextItem()
{
	switch (m_ucActiveData)
	{
	case 0:
		delete m_uData.pTextBlock;
		break;
	case 1:
		delete m_uData.pTextLine;
		break;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output: true if text was successfully interpreted, false otherwise
//-----------------------------------------------------------------------------
bool CTextItem::Success( void ) const
{
	switch (m_ucActiveData)
	{
	case 0:
		return m_uData.pTextBlock->Success();
	case 1:
		return m_uData.pTextLine->Success();
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output: CTextBlock if it is the active union member, NULL otherwise
//-----------------------------------------------------------------------------
CTextBlock *CTextItem::GetTextBlock( void ) const
{
	if (m_ucActiveData == 0)
		return m_uData.pTextBlock;

	return NULL;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output: CTextItem if it is the active union member, NULL otherwise
//-----------------------------------------------------------------------------
CTextLine *CTextItem::GetTextLine( void ) const
{
	if (m_ucActiveData == 1)
		return m_uData.pTextLine;

	return NULL;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output: string if it is the active union member, NULL otherwise
//-----------------------------------------------------------------------------
const char *CTextItem::GetString( void ) const
{
	if (m_ucActiveData == 2)
		return m_uData.sString;

	return NULL;
}