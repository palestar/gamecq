CoderGuru's NT service app wizard created the following files for you:

	NTService.h
	NTService.cpp
		CNTService
			This class implements the core functionality of
			the service.
			You might frequently watch it's home page at
			http://www.codeguru.com/misc/nt_service.shtml
			for bug fixes and/or further information.

	main.cpp
		main()
			Implements the startup procedure.

	Service.h
	Service.cpp
		CService	
			This is your CNTService derived class.
			Have a look at these TODO comments lurking around
			in these two files.

	NTServiceEventLogMsg.mc
	NTServiceEventLogMsg.rc
	NTServiceEventLogMsg.h
	MSG00000.bin
		Message catalogue and resource
			The MC file declares a message catalogue and compiles
			to the BIN, RC and the H files.
			You need these files for a correct message logging.
			The method CNTService::AddToMessageLog() uses this
			catalogue. Messages generated in this way can be read
			with the event-viewer utility found in the
			"Administrative Tools" program folder.

If you find any bug (and have a fix for it?!), please contact me at
	Joerg.Koenig@rhein-neckar.de	(private site)
	J.Koenig@adg.de					(company site)
