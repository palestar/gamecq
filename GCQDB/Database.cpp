/*
	Database.cpp

	Database implementation
	(c)2004 Palestar Inc, Richard Lyle
*/

#define GCQDB_DLL
#include "Database.h"

//----------------------------------------------------------------------------

IMPLEMENT_ABSTRACT_FACTORY( Database, Widget );

Database::Database()
{}

Database::~Database()
{}

Database * Database::create( const char * pClass )
{
	Database * pDB = WidgetCast<Database>( Factory::createNamedWidget( pClass ) );
	if ( pDB != NULL )
		pDB->grabReference();

	return pDB;
}

void Database::grab( Database * pDB )
{
	if ( pDB != NULL )
		pDB->grabReference();
}

void Database::release( Database * pDB )
{
	if ( pDB != NULL )
		pDB->releaseReference();
}

//----------------------------------------------------------------------------
//EOF
