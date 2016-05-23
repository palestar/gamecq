/*
	DatabaseMYSQL.h

	Concrete implementation of the Database interface using the MYSQL / MYSQLPP API's
	(c)2004 Palestar Inc, Richard Lyle
*/

#ifndef DATABASEMYSQL_H
#define DATABASEMYSQL_H

#include "Database.h"

//----------------------------------------------------------------------------

struct st_mysql;			// forward declare
class DatabaseMYSQL : public Database
{
public:
	DECLARE_WIDGET_CLASS();

	// Construction
	DatabaseMYSQL();
	virtual ~DatabaseMYSQL();

	// Database interface
	bool			connected() const;								// is a connection currently open
	bool			success() const;								// returns true if last query was successful
	dword			insertId() const;								// ID for last row inserted by query

	dword			fields(QueryHandle hQuery) const;				// number of fields returned
	dword			rows(QueryHandle hQuery) const;					// number of rows returned by the last query
	CharString		field(QueryHandle hQuery, int f ) const;		// get field f
	CharString		data(QueryHandle hQuery, int n, int f ) const;	// get data for row n, field f

	void			execute( const char * pSQL );					// sends an SQL query, waits for no results

	QueryHandle		createQuery( const char * pSQL );
	QueryHandle		copyQuery( QueryHandle hQuery );
	void			deleteQuery( QueryHandle hQuery );

	bool			open( const char * pName, 
						const char * pAddress, unsigned int nPort,
						const char * pUID, const char * pPW );
	void			close();

private:
	st_mysql *		m_pDB;	
	bool			m_bConnected;
	bool			m_bSuccess;
};



#endif

//----------------------------------------------------------------------------
//EOF
