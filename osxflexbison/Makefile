all: descr.tab.c lex.yy.c
	#g++ -c descr.tab.cpp descr.tab.h lex.yy.cc
	#ar rcs descr.lib descr.tab.o lex.yy.o

descr.tab.c: ../descr.y
	bison -o ../descr.tab.cpp --defines=../descr.tab.h ../descr.y

lex.yy.c: ../descr.l
	flex -o ../lex.yy.cpp ../descr.l
