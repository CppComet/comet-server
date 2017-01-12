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

#ifndef YY_YY_COMETQL_HH_INCLUDED
# define YY_YY_COMETQL_HH_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    TOK_SHOW = 258,
    TOK_DATABASES = 259,
    TOK_TABLES = 260,
    TOK_COLUMNS = 261,
    TOK_STATUS = 262,
    TOK_PROCESSLIST = 263,
    FLAG_GLOBAL = 264,
    FLAG_SESSION = 265,
    FLAG_FULL = 266,
    FLAG_FILESYSTEM = 267,
    FLAG_RAM = 268,
    FLAG_AVG = 269,
    FLAG_UPTIME = 270,
    TOK_FROM = 271,
    TOK_USE = 272,
    VAL_INT = 273,
    VAL_NAME = 274,
    TOK_SELECT = 275,
    TOK_LIMIT = 276,
    VAL_SYSTEM_VARIBLE = 277,
    TOK_INSERT = 278,
    TOK_INTO = 279,
    TOK_VALUES = 280,
    VAL_QUOTED_STRING = 281,
    BRACKETS_RIGHT = 282,
    COMMA = 283,
    BRACKETS_LEFT = 284,
    EQUALLY = 285,
    ASTERISK = 286,
    TOK_LIKE = 287,
    TOK_VARIABLES = 288,
    TOK_TABLE_STATUS = 289,
    TOK_DESCRIBE = 290,
    TOK_WHERE = 291,
    TOK_ASC = 292,
    TOK_DESC = 293,
    TOK_ORDER_BY = 294,
    TOK_DELETE = 295,
    TOK_IN = 296,
    TOK_OR = 297,
    TOK_AND = 298,
    MORE = 299,
    LESS = 300,
    TOK_DATABASE = 301
  };
#endif
/* Tokens.  */
#define TOK_SHOW 258
#define TOK_DATABASES 259
#define TOK_TABLES 260
#define TOK_COLUMNS 261
#define TOK_STATUS 262
#define TOK_PROCESSLIST 263
#define FLAG_GLOBAL 264
#define FLAG_SESSION 265
#define FLAG_FULL 266
#define FLAG_FILESYSTEM 267
#define FLAG_RAM 268
#define FLAG_AVG 269
#define FLAG_UPTIME 270
#define TOK_FROM 271
#define TOK_USE 272
#define VAL_INT 273
#define VAL_NAME 274
#define TOK_SELECT 275
#define TOK_LIMIT 276
#define VAL_SYSTEM_VARIBLE 277
#define TOK_INSERT 278
#define TOK_INTO 279
#define TOK_VALUES 280
#define VAL_QUOTED_STRING 281
#define BRACKETS_RIGHT 282
#define COMMA 283
#define BRACKETS_LEFT 284
#define EQUALLY 285
#define ASTERISK 286
#define TOK_LIKE 287
#define TOK_VARIABLES 288
#define TOK_TABLE_STATUS 289
#define TOK_DESCRIBE 290
#define TOK_WHERE 291
#define TOK_ASC 292
#define TOK_DESC 293
#define TOK_ORDER_BY 294
#define TOK_DELETE 295
#define TOK_IN 296
#define TOK_OR 297
#define TOK_AND 298
#define MORE 299
#define LESS 300
#define TOK_DATABASE 301

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE YYSTYPE;
union YYSTYPE
{
#line 115 "CometQL.y" /* yacc.c:1909  */

   struct TokStruct {
      char* text;
      int len;
      char quote;
   } tokStruct;

#line 154 "CometQL.hh" /* yacc.c:1909  */
};
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void* qInfo);

#endif /* !YY_YY_COMETQL_HH_INCLUDED  */
