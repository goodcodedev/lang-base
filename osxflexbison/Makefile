all: descr.tab.c lex.yy.c
	#g++ -c descr.tab.cpp descr.tab.h lex.yy.cc
	#ar rcs descr.lib descr.tab.o lex.yy.o

descr.tab.c:
	bison -o ../code/LangBase/gen/descr.tab.cpp --defines=../code/LangBase/gen/descr.tab.h ../code/LangBase/descr.y

lex.yy.c: 
	flex -o ../code/LangBase/gen/lex.yy.cpp ../code/LangBase/descr.l
