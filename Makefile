all : glookoo
	@echo done

glookoo : glookoo.cxx
	g++ -DHAVE_CONFIG_H -g glookoo.cxx -o glookoo -lgloox -lreadline -lpthread

clean :
	@rm *.log
	@rm -r autom4te.cache
