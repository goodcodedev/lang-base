/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton interface for Bison's Yacc-like parsers in C

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     IDENTIFIER = 258,
     TOKEN = 259,
     ENUM = 260,
     AST = 261,
     LIST = 262,
     LEFT_BRACE = 263,
     RIGHT_BRACE = 264,
     LEFT_PAREN = 265,
     RIGHT_PAREN = 266,
     LBRACKET = 267,
     RBRACKET = 268,
     COMMA = 269,
     COLON = 270,
     START = 271,
     STRING = 272,
     TOKEN_STRING = 273,
     TOKEN_INT = 274,
     TOKEN_FLOAT = 275
   };
#endif
/* Tokens.  */
#define IDENTIFIER 258
#define TOKEN 259
#define ENUM 260
#define AST 261
#define LIST 262
#define LEFT_BRACE 263
#define RIGHT_BRACE 264
#define LEFT_PAREN 265
#define RIGHT_PAREN 266
#define LBRACKET 267
#define RBRACKET 268
#define COMMA 269
#define COLON 270
#define START 271
#define STRING 272
#define TOKEN_STRING 273
#define TOKEN_INT 274
#define TOKEN_FLOAT 275




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 27 "../code/LangBase/descr.y"
{
	int ival;
	double fval;
	char *sval;
	void *ast;
	void *vector;
	int enm;
}
/* Line 1529 of yacc.c.  */
#line 98 "../code/LangBase/gen/descr.tab.h"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;

