#!/bin/bash
# This script should be added to the crontab and run every minute to ensure the ProcessServer is running.

export DISPLAY=:0 #needed if you are running a simple gui app.

set -e
(
	cd /opt/gamecq/
	# Lock so no more than one script can be running at once..
	flock -x -w 60 200
	ulimit -c unlimited
	ulimit -s 2048
	ulimit -a
	# Turn off malloc checking
	export MALLOC_CHECK_=0
	# Clean up ipc files
	rm -f /tmp/*.ipc
	# update any files ending with .upd first, since it may have exited due to an update..
	echo "$(date): Updating files..."
	find -type f -name '*.upd' | while read f; do mv "$f" "${f%.upd}"; done
	# Now start the ProcessServer
	echo "$(date): Starting ProcessServer..."
	./ProcessServer
	echo "$(date): ProcessServer has exited..."
) 200>/tmp/ProcessServerCron.lock


