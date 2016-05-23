#!/bin/bash
# This script makes a backup of the /opt/gamecq/ directory to a central server.

set -e
(
	backup_server=jupiter.palestar.com
	hostname=`hostname -s`
	date=`date +%Y-%m-%d`
	backup_dir=./backups/$hostname/
	echo "Backing up to $backup_server:$backup_dir..."

	cd /opt/gamecq/darkspace/

	# backup the user_storage directory first
	echo "Backing up user storage..."
	tar czf  darkspace_user_storage.$date.tar.gz user_storage/
	scp -r darkspace_user_storage.$date.tar.gz "$backup_server:$backup_dir"
	rm -f darkspace_user_storage.$date.tar.gz
	
	# backup the persistent universe
	echo "Backing up server storage..."
	tar czf darkspace_storage.$date.tar.gz storage/
 	scp -r darkspace_storage.$date.tar.gz "$backup_server:$backup_dir"
	rm -f darkspace_storage.$date.tar.gz
	
	cd /opt/gamecq/
	
	# backup configurations
	echo "Backing up server configs..."
	find . -name '*.ini' | tar -czf config.$date.tar.gz --files-from -
 	scp -r config.$date.tar.gz "$backup_server:$backup_dir"
	rm -f config.$date.tar.gz

	# backup database
	echo "Backing up mysql..."
	mysqldump --user backup --password=A387367dSh!# --all-databases | gzip > mysql-dump.$date.sql.gz
	scp -r mysql-dump.$date.sql.gz "$backup_server:$backup_dir"
	rm -f mysql-dump.$date.sql.gz
	
) 200>/tmp/BackupCron.lock

