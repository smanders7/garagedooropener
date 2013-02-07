#!/bin/sh

##
## Purpose: Startup Script for garagedoord
## File:    garagedoord.sh
## Author:  smanders
## Date:    Feb 6, 2013
## License: GPLv2
##

### BEGIN INIT INFO
# Provides:        garagedoord.sh
# Required-Start:  $network $remote_fs $syslog
# Required-Stop:   $network $remote_fs $syslog
# Default-Start:   2 3 4 5
# Default-Stop:
# Short-Description: Start garagedoord daemon
### END INIT INFO

PATH=/sbin:/bin:/usr/sbin:/usr/bin

. /lib/lsb/init-functions

DAEMON=/usr/sbin/garagedoord
PIDFILE=/var/run/garagedoord.pid

test -x $DAEMON || exit 5

LOCKFILE=/var/lock/garagedoord

case $1 in
	start)
		log_daemon_msg "Starting garagedoor server" "garagedoord"
  		start-stop-daemon --start --quiet --pidfile $PIDFILE --startas $DAEMON -- -p $PIDFILE
		status=$?
		log_end_msg $status
  		;;
	stop)
		log_daemon_msg "Stopping garagedoord server" "garagedoord"
  		start-stop-daemon --stop --quiet --pidfile $PIDFILE
		log_end_msg $?
		rm -f $PIDFILE
  		;;
	restart|force-reload)
		$0 stop && sleep 2 && $0 start
  		;;
	try-restart)
		if $0 status >/dev/null; then
			$0 restart
		else
			exit 0
		fi
		;;
	reload)
		exit 3
		;;
	status)
		status_of_proc $DAEMON "garagedoord server"
		;;
	*)
		echo "Usage: $0 {start|stop|restart|try-restart|force-reload|status}"
		exit 2
		;;
esac
