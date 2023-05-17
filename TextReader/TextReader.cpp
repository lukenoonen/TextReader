//=============== Copyright Luke Noonen, All rights reserved. ===============//
//
// Purpose: Defines the CTextReader class and its component classes,
// CTextBlock, CTextLine, and CTextItem, as well as UTIL functions
// used for extracting selected data types from a CTextItem instance
//
//===========================================================================//

#include "TextReader.h"
#include <stack>

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
bool UTIL_GetValue( const CTextItem *pTextItem, unsigned int &uiValue )
{
	if (!pTextItem->GetString())
		return false;

	uiValue = (unsigned int)atoi( pTextItem->GetString() );
	return true;
}

//-----------------------------------------------------------------------------
// Purpose:
// Output: true on successful data extraction, false on failure
//-----------------------------------------------------------------------------
bool UTIL_GetValue( const CTextItem *pTextItem, char &cValue )
{
	if (!pTextItem->GetString())
		return false;

	cValue = (char)atoi( pTextItem->GetString() );
	return true;
}

//-----------------------------------------------------------------------------
// Purpose:
// Output: true on successful data extraction, false on failure
//-----------------------------------------------------------------------------
bool UTIL_GetValue( const CTextItem *pTextItem, unsigned char &ucValue )
{
	if (!pTextItem->GetString())
		return false;

	ucValue = (char)atoi( pTextItem->GetString() );
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

	if (!ProcessText( m_sText ))
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
// Purpose: Using an adapted LL(1) algorithm, process the text into blocks,
// lines, and items
// Output: true on successful read, false otherwise
// Note: Will populate the CTextBlock data structure, but won't delete on fail
//-----------------------------------------------------------------------------
bool CTextReader::ProcessText( char *sText )
{
	std::stack<char> sStack;
	std::stack<CTextBlock *> pTextBlocks;
	std::stack<CTextLine *> pTextLines;

	sStack.push( 'B' );
	pTextBlocks.push( new CTextBlock() );
	m_pTextBlock = pTextBlocks.top();

	while (!sStack.empty())
	{
		char cTop = sStack.top();
		sText = sText + strspn( sText, " \t\n" );

		switch (cTop)
		{
		case ';':
		case '{':
		case '}':
		case '[':
		case ']':
		{
			if (cTop == *sText)
			{
				switch (cTop)
				{
				case ';':
				case ']':
				{
					*sText = '\0';
					pTextLines.pop();
					break;
				}
				case '}':
				{
					pTextBlocks.pop();
					break;
				}
				}
				sStack.pop();
				sText++;
			}
			else
			{
				return false;
			}
			break;
		}
		case '*':
		{
			char *sSearch = strpbrk( sText, ";{}[] \t\n" );
			if (sText < sSearch)
			{
				pTextLines.top()->AddTextItem( new CTextItem( sText ) );

				sStack.pop();
				sText = sSearch;
				switch (*sText)
				{
				case ' ':
				case '\t':
				case '\n':
				{
					*sText++ = '\0';
					break;
				}
				}
			}
			else
			{
				return false;
			}
			break;
		}
		case 'B':
		{
			switch (*sText)
			{
			case '\0':
			{
				sStack.pop();
				break;
			}
			case ']':
			case ';':
			{
				return false;
			}
			case '}':
			{
				sStack.pop();
				break;
			}
			default:
			{
				pTextLines.push( new CTextLine() );
				pTextBlocks.top()->AddTextLine( pTextLines.top() );

				sStack.pop();
				sStack.push( 'B' );
				sStack.push( ';' );
				sStack.push( 'L' );
				break;
			}
			}
			break;
		}
		case 'L':
		{
			switch (*sText)
			{
			case '}':
			case ']':
			case ';':
			{
				return false;
			}
			default:
			{
				sStack.pop();
				sStack.push( 'I' );
				break;
			}
			}
			break;
		}
		case 'I':
		{
			switch (*sText)
			{
			case '}':
			{
				return false;
			}
			case ']':
			case ';':
			{
				sStack.pop();
				break;
			}
			case '{':
			{
				CTextLine *pTextLine = pTextLines.top();
				pTextBlocks.push( new CTextBlock() );
				pTextLine->AddTextItem( new CTextItem( pTextBlocks.top() ) );

				sStack.pop();
				sStack.push( 'I' );
				sStack.push( '}' );
				sStack.push( 'B' );
				sStack.push( '{' );
				break;
			}
			case '[':
			{
				CTextLine *pTextLine = pTextLines.top();
				pTextLines.push( new CTextLine() );
				pTextLine->AddTextItem( new CTextItem( pTextLines.top() ) );

				sStack.pop();
				sStack.push( 'I' );
				sStack.push( ']' );
				sStack.push( 'L' );
				sStack.push( '[' );
				break;
			}
			default:
			{
				sStack.pop();
				sStack.push( 'I' );
				sStack.push( '*' );
				break;
			}
			}
			break;
		}
		}
	}

	if (*sText != '\0')
		return false;

	return true;
}

CTextBlock::CTextBlock()
{

}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CTextBlock::~CTextBlock()
{
	for (unsigned int i = 0; i < m_pTextLines.size(); i++)
		delete m_pTextLines[i];
}

void CTextBlock::AddTextLine( CTextLine *pTextLine )
{
	m_pTextLines.push_back( pTextLine );
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

CTextLine::CTextLine()
{

}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CTextLine::~CTextLine()
{
	for (unsigned int i = 0; i < m_pTextItems.size(); i++)
		delete m_pTextItems[i];
}

void CTextLine::AddTextItem( CTextItem *pTextItem )
{
	m_pTextItems.push_back( pTextItem );
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

CTextItem::CTextItem( CTextBlock *pTextBlock )
{
	m_uData.pTextBlock = pTextBlock;
	m_ucActiveData = 0;
}

CTextItem::CTextItem( CTextLine *pTextLine )
{
	m_uData.pTextLine = pTextLine;
	m_ucActiveData = 1;
}

CTextItem::CTextItem( const char *sString )
{
	m_uData.sString = sString;
	m_ucActiveData = 2;
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