Glookoo
=======

REQUIRES:
=========

- readline
- gloox

COMPILE GLOOX:
---------------

	# wget http://camaya.net/download/gloox-1.0-beta7.tar.bz2
	# tar xjvf gloox-1.0-beta7.tar.bz2
	# cd gloox-1.0-beta7
	# vim config.h.win
	
	comment out HAVE_WINDNS_H
	// #define HAVE_WINDNS_H 1
	
	# cd src
	# gcc -I. -c *.cpp
	# ar cr libgloox.a *.o

COMPILE READLINE:
------------------

	# make clean GCE-inlined

COMPILE GLOOKOO:
----------------

	# gcc -Ic:/mingw/include/pthread -o glookoo.exe -I. glookoo.cxx libgloox.a c:/mingw/lib/readline.lib -lstdc++ -lws2_32 -lcrypt32 -lsecur32 -lpthreadGCE2

USAGE:
------

	Usage: glookoo [-c config-file] [-s server] [-j jid] [-p passwd] [-u user]

	# cat config-file
	server=my-server
	jid=my@example.comment
	passwd=mypassword
	user=your@example.com

	# glookoo -c config-file

or 

	# glookoo -s my-server -j my@example.com -p mypassword -u your@example.com
