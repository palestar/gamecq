/*
	Database.h

	Abstract Interface for accessing a database
	(c)2004 Palestar Inc, Richard Lyle
*/

#ifndef DATABASE_H
#define DATABASE_H

#include "Factory/FactoryTypes.h"
#include "Standard/CharString.h"
#include "GCQDBDll.h"

#include <stdlib.h>			// strtod / strtol

//----------------------------------------------------------------------------

class DLL Database : public Widget
{
public:
	DECLARE_WIDGET_CLASS();

	// Construction
	Database();
	virtual ~Database();
	
	// Types
	typedef void *					QueryHandle;

	struct Row;
	struct Query;

	struct Data
	{
		Data();
		Data( const Row * pRow, int nField );
		Data( const Data & copy );

		operator				const char *() const;
		operator				unsigned long() const;
		operator				long() const;
		operator				unsigned int() const;
		operator				int() const;
		operator				unsigned short() const;
		operator				short() const;
		operator				float() const;
		operator				double() const;

		// Data
		CharString				m_sData;
	};

	struct Row
	{
		Row();
		Row( const Query * pQuery, int nRow );
		Row( const Row & copy );
		~Row();

		CharString				data( int f ) const;								// get the data for field f

		// Helpers
		Data					operator[]( int n ) const;							// returns field n
		Data					operator[]( const char * pName ) const;				// returns field by name

		// Data
		const Query *			m_pQuery;
		int						m_nRow;
	};

	struct Query
	{
		Query();
		Query( Database * pDB, QueryHandle hQuery );
		Query( const Query & copy );
		~Query();

		bool					success() const;							// result status
		int						fields() const;								// number of fields returned
		int						rows() const;								// number of rows returned

		CharString				field( int f ) const;						// get name of field f
		CharString				data( int n, int f ) const;					// get data for row n and field f

		// Helpers
		Row						operator[]( int n ) const;					// get row n	
		// depreciated
		int						size() const;

		// Data
		Database *				m_pDB;
		QueryHandle				m_hResult;
	};

	// Accessors
	virtual bool			connected() const = 0;							// is a connection currently open
	virtual bool			success() const = 0;							// returns true if last query was successful
	virtual dword			insertId() const = 0;							// ID for last row inserted by query

	virtual dword			fields(QueryHandle hQuery) const = 0;				// number of fields returned
	virtual dword			rows(QueryHandle hQuery) const = 0;					// number of rows returned by the last query
	virtual CharString		field(QueryHandle hQuery, int f ) const = 0;		// get field f
	virtual CharString		data(QueryHandle hQuery, int n, int f ) const = 0;	// get data for row n, field f
	
	// Mutators
	virtual void			execute( const char * pSQL ) = 0;				// sends an SQL query, waits for no results
	virtual QueryHandle		createQuery( const char * pSQL ) = 0;
	virtual QueryHandle		copyQuery( QueryHandle hQuery ) = 0;
	virtual void			deleteQuery( QueryHandle hQuery ) = 0;

	// Mutators
	virtual bool			open( const char * pName,						// name of the database
								const char * pAddress,						// IP address
								unsigned int nPort,							// port
								const char * pUID,							// user 
								const char * pPW ) = 0;						// password
	virtual void			close() = 0;

	// Helpers
	Query					query( const char * pSQL );						// send and get the results of an SQL query
	Query					query( const wchar * pSQL );					// wide character support
	void					execute( const wchar * pSQL );

	// Static
	static Database *		create( const char * pClass );					// create a database object by class name
	static void				grab( Database * pDB );
	static void				release( Database * pDB );						// destroy a database object - this is provided to make sure the right memory allocator is used
																			// to destroy the created database object
};

//----------------------------------------------------------------------------

inline Database::Data::Data()
{}

inline Database::Data::Data( const Row * pRow, int nField )
{
	if ( pRow != NULL )
		m_sData = pRow->data( nField );
}

inline Database::Data::Data( const Data & copy ) : m_sData( copy.m_sData )
{}

inline Database::Data::operator const char *() const
{
	return m_sData;
}

inline Database::Data::operator unsigned long() const
{
	return (unsigned long)strtoul( *this, NULL, 10 );
}

inline Database::Data::operator long() const
{
	return (long)strtol( *this, NULL, 10 );
}

inline Database::Data::operator unsigned int() const
{
	return (unsigned int)strtoul( *this, NULL, 10 );
}

inline Database::Data::operator int() const
{
	return (int)strtol( *this, NULL, 10 );
}

inline Database::Data::operator unsigned short() const
{
	return (unsigned short)strtoul( *this, NULL, 10 );
}

inline Database::Data::operator short() const
{
	return (short)strtol( *this, NULL, 10 );
}

inline Database::Data::operator float() const
{
	return (float)strtod( *this, NULL );
}

inline Database::Data::operator double() const
{
	return strtod( *this, NULL );
}

//----------------------------------------------------------------------------

inline Database::Row::Row() : m_pQuery( NULL ), m_nRow( -1 )
{}

inline Database::Row::Row( const Query * pQuery, int nRow  ) : m_pQuery( pQuery ), m_nRow( nRow )
{
	if ( m_pQuery != NULL && m_pQuery->m_pDB != NULL )
		Database::grab( m_pQuery->m_pDB );
}

inline Database::Row::Row( const Row & copy ) : m_pQuery( copy.m_pQuery ), m_nRow( copy.m_nRow )
{
	if ( m_pQuery != NULL && m_pQuery->m_pDB != NULL )
		Database::grab( m_pQuery->m_pDB );
}

inline Database::Row::~Row()
{
	if ( m_pQuery != NULL && m_pQuery->m_pDB != NULL )
		Database::release( m_pQuery->m_pDB );
}

inline CharString Database::Row::data( int f ) const
{
	if ( m_pQuery != NULL )
		return m_pQuery->data( m_nRow, f );
	return "";
}

inline Database::Data Database::Row::operator[]( int n ) const
{
	return Data( this, n );
}

inline Database::Data Database::Row::operator[]( const char * pName ) const
{
	if ( m_pQuery != NULL )
	{
		for(int i=0;i<m_pQuery->fields();i++)
			if ( stricmp<char>( pName, m_pQuery->field( i ) ) == 0 )
				return operator[]( i );
	}

	// field not found, return Data object with invalid field index
	return Data( this, -1 );
}

//----------------------------------------------------------------------------

inline Database::Query::Query() : m_pDB( NULL ), m_hResult( NULL )
{}

inline Database::Query::Query( Database * pDB, QueryHandle hQuery ) : m_pDB( pDB ), m_hResult( hQuery )
{
	if ( m_pDB != NULL )
		Database::grab( m_pDB );
}

inline Database::Query::Query( const Query & copy ) : m_pDB( copy.m_pDB ), m_hResult( NULL )
{
	// use the database object to copy the result object
	if ( m_pDB != NULL && m_hResult != NULL )
		m_hResult = m_pDB->copyQuery( copy.m_hResult );
	if ( m_pDB != NULL )
		Database::grab( m_pDB );
}

inline Database::Query::~Query()
{
	// delete the result object
	if ( m_pDB != NULL && m_hResult != NULL )
		m_pDB->deleteQuery( m_hResult );
	if ( m_pDB != NULL )
		Database::release( m_pDB );
}

inline bool Database::Query::success() const
{
	return m_pDB != NULL ? m_pDB->success() : false;
}

inline int Database::Query::fields() const
{
	return m_pDB != NULL ? m_pDB->fields( m_hResult ) : 0;
}

inline int Database::Query::rows() const
{
	return m_pDB != NULL ? m_pDB->rows( m_hResult ) : 0;
}

inline CharString Database::Query::field( int f ) const
{
	return m_pDB != NULL ? m_pDB->field( m_hResult, f ) : "";
}

inline CharString Database::Query::data( int n, int f ) const
{
	return m_pDB != NULL ? m_pDB->data( m_hResult, n, f ) : "";
}

inline Database::Row Database::Query::operator[]( int n ) const
{
	return Row( this, n );
}

inline int Database::Query::size() const
{
	return rows();
}

//----------------------------------------------------------------------------

inline Database::Query Database::query( const char * pSQL )
{
	return Query( this, createQuery( pSQL ) );
}

inline Database::Query Database::query( const wchar * pSQL )
{
	return Query( this, createQuery( CharString( pSQL ) ) );
}

inline void Database::execute( const wchar * pSQL )
{
	execute( CharString( pSQL ) );
}

//----------------------------------------------------------------------------




#endif

//----------------------------------------------------------------------------
//EOF
