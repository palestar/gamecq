/*
	UploadThread.h
	(c)2005 Palestar Inc, Richard Lyle
*/

#ifndef UPLOADTHREAD_H
#define UPLOADTHREAD_H

#include "Network/MirrorClient.h"
#include "Debug/Log.h"
#include "Debug/SafeThread.h"

//----------------------------------------------------------------------------

class UploadThread : public SafeThread, public MirrorClient	
{
public:
	// Construction
	UploadThread( const char * pPath, const char * pAddress, int nPort,
		const char * pUID, const char * pPW ) : m_bDone( false ), m_bError( false ), 
		m_sPath( pPath ), m_sAddress( pAddress ), m_nPort( nPort ), 
		m_sUID( pUID ), m_sPW( pPW )
	{}

	// Thread interface
	int run()
	{
		if (!m_sPath.endsWith( "\\" ) && !m_sPath.endsWith( "/" ) )
			m_sPath += "\\";

		LOG_STATUS( "UploadThread", CharString().format("Starting upload, path = %s, server = %s:%d", m_sPath, m_sAddress, m_nPort) );

		m_bError = true;
		if ( open( m_sAddress, m_nPort, m_sPath, NULL, false ) )
		{
			if (! login( m_sUID, m_sPW ) )
				LOG_STATUS( "UploadThread", "Failed to login..." );

			dword nJob = syncronize( true );
			if ( nJob == 0 || waitJob( nJob, 86400 * 1000 ) > 0 )
				m_bError = false;		// job completed successfully!
		}

		// close our connection
		close();

		if ( m_bError )
			LOG_STATUS( "UploadThread", "Upload ended with an error!" );
		else
			LOG_STATUS( "UploadThread", "Upload ended successfully!" );
		m_bDone = true;

		return 0;
	}

	// MirrorClient interface
	void onStatus( const char * status )
	{
		LOG_STATUS( "UploadThread", status );
	}
	void onError( const char * error )
	{
		LOG_STATUS( "UploadThread", error );
	}
	int onFileConflict( const char * pFile )
	{
		return USE_LOCAL;
	}
	bool onFileDownload( const char * pFile )
	{
		return false;
	}
	bool onFileUpload( const char * pFile )
	{
		LOG_STATUS( "UploadThread", CharString().format( "Uploading %s", pFile ) );
		return true;
	}

	// Accessors
	bool done() const
	{
		return m_bDone;
	}
	bool error() const
	{
		return m_bError;
	}

private:
	bool					m_bDone;
	bool					m_bError;

	CharString				m_sPath;
	CharString				m_sAddress;
	int						m_nPort;
	CharString				m_sUID;
	CharString				m_sPW;
};




#endif

//----------------------------------------------------------------------------
//EOF
