all : glookoo
	@echo done

glookoo : glookoo.cxx
	g++ -DHAVE_GLOOX_MESSAGE_H -g glookoo.cxx -o glookoo -lgloox -lreadline -lpthread -lws2_32 -lsecur32 -lcrypt32

clean :
	@rm *.log
	@rm -r autom4te.cache
