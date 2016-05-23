/*
	DatabaseMYSQL.cpp
	(c)2004 Palestar Inc, Richard Lyle
*/

#define GCQDB_DLL
#include "DatabaseMYSQL.h"

#include <stdio.h>
#include <time.h>

#if defined(_WIN32)
#include <winsock2.h>
#endif

#include "mysql.h"
#include "errmsg.h"

#pragma warning( disable:4996 )		// 'localtime' was declared deprecated

//---------------------------------------------------------------------------------------------------

struct QueryResult
{
	MYSQL_RES *		pResult;
	int				nUsers;
};

//---------------------------------------------------------------------------------------------------

IMPLEMENT_FACTORY( DatabaseMYSQL, Database );

DatabaseMYSQL::DatabaseMYSQL() : m_pDB( NULL ), m_bConnected( false ), m_bSuccess( true )
{}

DatabaseMYSQL::~DatabaseMYSQL()
{
	close();
}

bool DatabaseMYSQL::connected() const
{
	return m_bConnected;
}

bool DatabaseMYSQL::success() const
{
	return m_bSuccess;
}

dword DatabaseMYSQL::insertId() const
{
	return m_pDB != NULL ? (dword)m_pDB->insert_id : -1;
}

dword DatabaseMYSQL::fields(QueryHandle hQuery) const
{
	QueryResult * pQuery = (QueryResult *)hQuery;
	if ( pQuery == NULL || pQuery->pResult == NULL )
		return -1;
	return mysql_num_fields( pQuery->pResult );
}

dword DatabaseMYSQL::rows(QueryHandle hQuery) const
{
	QueryResult * pQuery = (QueryResult *)hQuery;
	if ( pQuery == NULL || pQuery->pResult == NULL )
		return -1;
	return (dword)mysql_num_rows( pQuery->pResult );
}

CharString DatabaseMYSQL::field(QueryHandle hQuery, int f ) const
{
	QueryResult * pQuery = (QueryResult *)hQuery;
	if ( pQuery == NULL || pQuery->pResult == NULL )
		return "";
	MYSQL_FIELD * pField = mysql_fetch_field_direct( pQuery->pResult, f );
	return pField != NULL ? CharString( pField->name ) : "";
}

CharString DatabaseMYSQL::data(QueryHandle hQuery, int n, int f ) const
{
	QueryResult * pQuery = (QueryResult *)hQuery;
	if ( pQuery == NULL || pQuery->pResult == NULL )
		return "";
	if ( n < 0 || n >= (int)rows( hQuery ) )
		return "";
	if ( f < 0 || f >= (int)fields( hQuery ) )
		return "";

	mysql_data_seek( pQuery->pResult, n );
	MYSQL_ROW row = mysql_fetch_row( pQuery->pResult );
	return CharString( row[f] ? row[f] : "" );
}

//----------------------------------------------------------------------------

void DatabaseMYSQL::execute( const char * pSQL )
{
	if ( m_pDB != NULL )
		m_bSuccess = mysql_query( m_pDB, pSQL ) == 0;
}

Database::QueryHandle DatabaseMYSQL::createQuery( const char * pSQL )
{
	if ( m_pDB == NULL )
		return NULL;

	if ( (m_bSuccess = mysql_query( m_pDB, pSQL ) == 0) )
	{
		QueryResult * pQuery = new QueryResult;
		pQuery->pResult = mysql_store_result( m_pDB );
		pQuery->nUsers = 1;

		return (QueryHandle)pQuery;
	}

	LOG_ERROR( "DatabaseMYSQL", CharString().format( "Query Failed: %s (%s)(%d)", pSQL, mysql_error( m_pDB ), mysql_errno( m_pDB ) ) );
	m_bConnected = false;
	return NULL;
}

Database::QueryHandle DatabaseMYSQL::copyQuery( QueryHandle hQuery )
{
	if ( hQuery == NULL )
		return NULL;
	((QueryResult *)hQuery)->nUsers++;
	return hQuery;
}

void DatabaseMYSQL::deleteQuery( QueryHandle hQuery )
{
	QueryResult * pResult = (QueryResult *)hQuery;
	pResult->nUsers--;

	if ( pResult->nUsers <= 0 )
	{
		mysql_free_result( pResult->pResult );
		delete pResult;
	}
}

bool DatabaseMYSQL::open( const char * pName, const char * pAddress, 
								unsigned int nPort, const char * pUID, const char * pPW )
{
	close();
	
	m_pDB = new MYSQL();
	mysql_init( m_pDB );

	if (! mysql_real_connect( m_pDB, pAddress, pUID, pPW, pName, nPort, NULL, 0 ) )
		return false;
	m_bConnected = true;

	return true;
}

void DatabaseMYSQL::close()
{
	if ( m_pDB != NULL )
	{
		mysql_close( m_pDB );

		delete m_pDB;
		m_pDB = NULL;
	}
}

//----------------------------------------------------------------------------
//EOF