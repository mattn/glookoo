all : glookoo
	@echo done

glookoo : glookoo.cxx
	g++ -g glookoo.cxx -o glookoo -lgloox -lreadline -lpthread

clean :
	@rm *.log
	@rm -r autom4te.cache
