/* A Bison parser, made by GNU Bison 3.0.2.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2013 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

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

#ifndef YY_YY_COMETQL_TAB_H_INCLUDED
# define YY_YY_COMETQL_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    TOK_SET = 258,
    TOK_SHOW = 259,
    TOK_DATABASES = 260,
    TOK_TABLES = 261,
    TOK_COLUMNS = 262,
    TOK_STATUS = 263,
    TOK_PROCESSLIST = 264,
    FLAG_GLOBAL = 265,
    FLAG_SESSION = 266,
    FLAG_FULL = 267,
    FLAG_FILESYSTEM = 268,
    FLAG_RAM = 269,
    FLAG_AVG = 270,
    FLAG_UPTIME = 271,
    TOK_FROM = 272,
    TOK_USE = 273,
    VAL_INT = 274,
    VAL_NAME = 275,
    TOK_SELECT = 276,
    TOK_LIMIT = 277,
    VAL_SYSTEM_VARIBLE = 278,
    TOK_INSERT = 279,
    TOK_INTO = 280,
    TOK_VALUES = 281,
    VAL_QUOTED_STRING = 282,
    BRACKETS_RIGHT = 283,
    COMMA = 284,
    BRACKETS_LEFT = 285,
    EQUALLY = 286,
    ASTERISK = 287,
    TOK_LIKE = 288,
    TOK_VARIABLES = 289,
    TOK_TABLE_STATUS = 290,
    TOK_DESCRIBE = 291,
    TOK_WHERE = 292,
    TOK_ASC = 293,
    TOK_DESC = 294,
    TOK_ORDER_BY = 295,
    TOK_DELETE = 296,
    TOK_IN = 297,
    TOK_OR = 298,
    TOK_AND = 299,
    MORE = 300,
    LESS = 301,
    TOK_DATABASE = 302
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE YYSTYPE;
union YYSTYPE
{
#line 115 "src/CometQL.y" /* yacc.c:1909  */

   struct TokStruct {
      char* text;
      int len;
      char quote;
   } tokStruct;

#line 110 "CometQL.tab.h" /* yacc.c:1909  */
};
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void* qInfo);

#endif /* !YY_YY_COMETQL_TAB_H_INCLUDED  */
