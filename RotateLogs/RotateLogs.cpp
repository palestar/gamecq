// RotateLogs.cpp : Defines the entry point for the console application.
//

#include "Standard/Time.h"
#include "File/Path.h"
#include "File/FindFile.h"
#include "File/Stream.h"
#include "File/FileDisk.h"

#include <stdio.h>

int main(int argc, char* argv[])
{
	int depth = 30;

	CharString directory( "." PATH_SEPERATOR );
	if ( argc == 3 )
	{
		// get the depth
		depth = atoi( argv[1] );
		// get the directory
		directory = Path( argv[2] ).directory();
	}

	// find all files that end with .log
	FindFile ff( directory + "*.log" );
	for(int i=0;i<ff.fileCount();i++)
	{
		// remove the log extention
		CharString name( ff.file(i) );
		name[ name.reverseFind('.') ] = 0;

		// skip archives of log files
		if ( name.reverseFind( '.' ) >= 0 )
			continue;

		// remove the oldest log file
		FileDisk::deleteFile( CharString().format("%s%s.%3.3d.log", directory.cstr(), name.cstr(), depth) );

		// rotate the files
		for(int j=depth;j>0;j--)
		{
			CharString oldName;
			oldName.format( "%s%s.%3.3d.log", directory.cstr(), name.cstr(), j - 1 );
			CharString newName;
			newName.format( "%s%s.%3.3d.log", directory.cstr(), name.cstr(), j );

			FileDisk::renameFile( oldName, newName );
		}

		CharString oldName;
		oldName.format( "%s%s", directory.cstr(), ff.file(i) );
		CharString newName;
		newName.format( "%s%s.000.log", directory.cstr(), name.cstr() );

		FileDisk::copyFile( oldName, newName );

		// open and reshorten the log file
		FileDisk::saveTextFile( oldName, "" );
		//LogHelper::log( oldName, "Log file start" );
		//LogHelper::log( newName, "Log file end" );
	}

	return 0;
}
