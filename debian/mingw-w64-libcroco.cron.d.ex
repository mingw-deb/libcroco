#
# Regular cron jobs for the mingw-w64-libcroco package
#
0 4	* * *	root	[ -x /usr/bin/mingw-w64-libcroco_maintenance ] && /usr/bin/mingw-w64-libcroco_maintenance
