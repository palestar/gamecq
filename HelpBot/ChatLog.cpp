// ChatLog.cpp: implementation of the CChatLog class.
//
//////////////////////////////////////////////////////////////////////

#include "ChatLog.h"
#include "File\FileDisk.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CChatLog::CChatLog()
{

}

CChatLog::~CChatLog()
{

}

void CChatLog::write(String text)
{
	FileDisk fd;
	if( fd.open( "BotLog.html", FileDisk::AccessType::READ_WRITE ) )
	{
		if( fd.size() == 0 )
			fd.write("<BODY bgcolor=\"#000000\" text=\"#FFFFFF\">\r\n", 41);
		fd.setPosition( fd.size() );
		if( !( text.endsWith("\r\n") || text.endsWith("\n\r") ) )
		{
			text.append('\r');
			text.append('\n');
		}
		text.append('<');
		text.append('b');
		text.append('r');
		text.append('>');
		fd.write( text, text.length() );
		fd.close();
	}
	
}
