#ifndef TEXTREADER_H
#define TEXTREADER_H

#include <vector>
#include <string>

class CTextReader;
class CTextBlock;
class CTextLine;
class CTextItem;

bool UTIL_GetValue( const CTextItem *pTextItem, CTextBlock *&pValue );
bool UTIL_GetValue( const CTextItem *pTextItem, CTextLine *&pValue );
bool UTIL_GetValue( const CTextItem *pTextItem, char *&sValue );
bool UTIL_GetValue( const CTextItem *pTextItem, const char *&sValue );
bool UTIL_GetValue( const CTextItem *pTextItem, bool &bValue );
bool UTIL_GetValue( const CTextItem *pTextItem, int &iValue );
bool UTIL_GetValue( const CTextItem *pTextItem, float &fValue );

class CTextReader
{
public:
	CTextReader( const char *sText );
	~CTextReader();

	bool Success( void ) const;

	CTextBlock *GetTextBlock( void ) const;

private:
	char *m_sText;
	CTextBlock *m_pTextBlock;
};

class CTextBlock
{
public:
	CTextBlock( char *sTextBlock );
	~CTextBlock();

	bool Success( void ) const;

	unsigned int GetTextLineCount( void ) const;
	CTextLine *GetTextLine( unsigned int uiIndex ) const;

	CTextLine *GetTextLine( const char *sKey ) const;

	template <class T> bool GetValue( T &tValue, unsigned int uiIndex, const char *sKey ) const;

private:
	bool m_bSuccess;
	std::vector<CTextLine *> m_pTextLines;
};

class CTextLine
{
public:
	CTextLine( char *sTextLine );
	~CTextLine();

	bool Success( void ) const;

	unsigned int GetTextItemCount( void ) const;
	CTextItem *GetTextItem( unsigned int uiIndex ) const;

	bool IsKey( const char *sKey ) const;

	template <class T> bool GetValue( T &tValue, unsigned int uiIndex ) const;

private:
	std::vector<CTextItem *> m_pTextItems;
};

class CTextItem
{
public:
	CTextItem( char *sTextItem, unsigned char ucActiveData );
	~CTextItem();

	bool Success( void ) const;

	CTextBlock *GetTextBlock( void ) const;
	CTextLine *GetTextLine( void ) const;
	const char *GetString( void ) const;

	template <class T> bool GetValue( T &tValue ) const;

private:
	unsigned char m_ucActiveData; // TODO: maybe make this an enum
	union
	{
		CTextBlock *pTextBlock;
		CTextLine *pTextLine;
		const char *sString;
	} m_uData;
};

template <class T> bool CTextBlock::GetValue( T &tValue, unsigned int uiIndex, const char *sKey ) const
{
	CTextLine *pTextLine = GetTextLine( sKey );
	return pTextLine && pTextLine->GetValue( tValue, uiIndex );
}

template <class T> bool CTextLine::GetValue( T &tValue, unsigned int uiIndex ) const
{
	CTextItem *pTextItem = GetTextItem( uiIndex );
	return pTextItem && pTextItem->GetValue( tValue );
}

template <class T> bool CTextItem::GetValue( T &tValue ) const
{
	return UTIL_GetValue( this, tValue );
}

#endif // TEXTREADER_H