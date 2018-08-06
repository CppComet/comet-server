/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015 Free Software Foundation, Inc.

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

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "3.0.4"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* Copy the first part of user declarations.  */
#line 1 "../src/CometQL.y" /* yacc.c:339  */

#include <stdio.h>
#include <string.h>
 
#include "CometQL.h" 
    

    extern "C"
    {
        int yyparse (void *qInfo);
        void yyerror(void *qInfo, const char *str)
        {
            QueryData *qData = static_cast<struct QueryData*>(qInfo);
            qData->hasError = 1;
            fprintf(stderr,"Error: %s\n",str);
        }
        
        int yylex(void*);  
        int yywrap()
        {
            return 1;
        } 
        
        void* yy_scan_string(char *);
        void  yy_delete_buffer(void* buffer);
    }


 
void* QLParsing(char* query, QueryData *qInfo)
{
    /**
     * http://stackoverflow.com/questions/780676/string-input-to-flex-lexer
     */ 
     
    qInfo->StartQury = query;
    
    // note yy_scan_buffer is is looking for a double null string
    void* buffer = yy_scan_string(query);
    yyparse((void*)qInfo);
    return buffer;
}

void QLdeleteBuffer(void* buffer, QueryData *qInfo)
{
    qInfo->clear();
    yy_delete_buffer(buffer);
}


/**
 * @param buffer строка
 * @param len длина токена
 * @return число содержащиеся в строке
 */
int strToInt(const char* buffer, int len)
{
    const char* p = buffer;
    int value = 0;
    int positiv = 1;
    
    if(*p == '-')
    {
        positiv = -1;
        p++;
        len--;
        
        if(len <= 0)
        {
            return positiv*value;
        }
    }

    while(*p == '0' )
    {
        p++;
        len--;
        
        if(len <= 0)
        {
            return positiv*value;
        }
    }

    while(1)
    {
        if(*p > '9' || *p < '0')
        {
            return positiv*value;
        }

        value = (value*10)+(*p - '0');
        p++;
        len--;

        if(len <= 0)
        {
            return positiv*value;
        }
    }
    return 0;
}


#define setQData QueryData *qData = static_cast<struct QueryData*>(qInfo)
     
#define setTokValue(qData, place, tok) place.tokStart = tok.text - qData->StartQury;\
                    place.tokLen = tok.len;\
                    place.quote = tok.quote;\
                    

#line 178 "../src/CometQL.cc" /* yacc.c:339  */

# ifndef YY_NULLPTR
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULLPTR nullptr
#  else
#   define YY_NULLPTR 0
#  endif
# endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* In a future release of Bison, this section will be replaced
   by #include "CometQL.hh".  */
#ifndef YY_YY_SRC_COMETQL_HH_INCLUDED
# define YY_YY_SRC_COMETQL_HH_INCLUDED
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
/* Tokens.  */
#define TOK_SET 258
#define TOK_SHOW 259
#define TOK_DATABASES 260
#define TOK_TABLES 261
#define TOK_COLUMNS 262
#define TOK_STATUS 263
#define TOK_PROCESSLIST 264
#define FLAG_GLOBAL 265
#define FLAG_SESSION 266
#define FLAG_FULL 267
#define FLAG_FILESYSTEM 268
#define FLAG_RAM 269
#define FLAG_AVG 270
#define FLAG_UPTIME 271
#define TOK_FROM 272
#define TOK_USE 273
#define VAL_INT 274
#define VAL_NAME 275
#define TOK_SELECT 276
#define TOK_LIMIT 277
#define VAL_SYSTEM_VARIBLE 278
#define TOK_INSERT 279
#define TOK_INTO 280
#define TOK_VALUES 281
#define VAL_QUOTED_STRING 282
#define BRACKETS_RIGHT 283
#define COMMA 284
#define BRACKETS_LEFT 285
#define EQUALLY 286
#define ASTERISK 287
#define TOK_LIKE 288
#define TOK_VARIABLES 289
#define TOK_TABLE_STATUS 290
#define TOK_DESCRIBE 291
#define TOK_WHERE 292
#define TOK_ASC 293
#define TOK_DESC 294
#define TOK_ORDER_BY 295
#define TOK_DELETE 296
#define TOK_IN 297
#define TOK_OR 298
#define TOK_AND 299
#define MORE 300
#define LESS 301
#define TOK_DATABASE 302

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{
#line 115 "../src/CometQL.y" /* yacc.c:355  */

   struct TokStruct {
      char* text;
      int len;
      char quote;
   } tokStruct;

#line 320 "../src/CometQL.cc" /* yacc.c:355  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void* qInfo);

#endif /* !YY_YY_SRC_COMETQL_HH_INCLUDED  */

/* Copy the second part of user declarations.  */

#line 337 "../src/CometQL.cc" /* yacc.c:358  */

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif

#ifndef YY_ATTRIBUTE
# if (defined __GNUC__                                               \
      && (2 < __GNUC__ || (__GNUC__ == 2 && 96 <= __GNUC_MINOR__)))  \
     || defined __SUNPRO_C && 0x5110 <= __SUNPRO_C
#  define YY_ATTRIBUTE(Spec) __attribute__(Spec)
# else
#  define YY_ATTRIBUTE(Spec) /* empty */
# endif
#endif

#ifndef YY_ATTRIBUTE_PURE
# define YY_ATTRIBUTE_PURE   YY_ATTRIBUTE ((__pure__))
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# define YY_ATTRIBUTE_UNUSED YY_ATTRIBUTE ((__unused__))
#endif

#if !defined _Noreturn \
     && (!defined __STDC_VERSION__ || __STDC_VERSION__ < 201112)
# if defined _MSC_VER && 1200 <= _MSC_VER
#  define _Noreturn __declspec (noreturn)
# else
#  define _Noreturn YY_ATTRIBUTE ((__noreturn__))
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

#if defined __GNUC__ && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN \
    _Pragma ("GCC diagnostic push") \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")\
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif


#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYSIZE_T yynewbytes;                                            \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / sizeof (*yyptr);                          \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, (Count) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYSIZE_T yyi;                         \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  2
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   99

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  48
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  28
/* YYNRULES -- Number of rules.  */
#define YYNRULES  72
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  123

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   302

#define YYTRANSLATE(YYX)                                                \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, without out-of-bounds checking.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   181,   181,   182,   186,   188,   190,   192,   194,   201,
     210,   218,   231,   239,   247,   255,   264,   272,   281,   290,
     302,   304,   307,   310,   316,   322,   328,   334,   340,   347,
     350,   361,   371,   379,   387,   390,   394,   406,   422,   441,
     447,   453,   459,   465,   473,   475,   479,   488,   505,   508,
     515,   522,   530,   533,   541,   552,   558,   562,   564,   568,
     589,   596,   605,   607,   611,   613,   617,   635,   653,   670,
     681,   683,   685
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "TOK_SET", "TOK_SHOW", "TOK_DATABASES",
  "TOK_TABLES", "TOK_COLUMNS", "TOK_STATUS", "TOK_PROCESSLIST",
  "FLAG_GLOBAL", "FLAG_SESSION", "FLAG_FULL", "FLAG_FILESYSTEM",
  "FLAG_RAM", "FLAG_AVG", "FLAG_UPTIME", "TOK_FROM", "TOK_USE", "VAL_INT",
  "VAL_NAME", "TOK_SELECT", "TOK_LIMIT", "VAL_SYSTEM_VARIBLE",
  "TOK_INSERT", "TOK_INTO", "TOK_VALUES", "VAL_QUOTED_STRING",
  "BRACKETS_RIGHT", "COMMA", "BRACKETS_LEFT", "EQUALLY", "ASTERISK",
  "TOK_LIKE", "TOK_VARIABLES", "TOK_TABLE_STATUS", "TOK_DESCRIBE",
  "TOK_WHERE", "TOK_ASC", "TOK_DESC", "TOK_ORDER_BY", "TOK_DELETE",
  "TOK_IN", "TOK_OR", "TOK_AND", "MORE", "LESS", "TOK_DATABASE", "$accept",
  "commands", "command", "set_query", "show_query", "tok_from_or_in",
  "opt_for_show_status", "opt_for_show_columns", "select_query",
  "opt_for_where", "opt_for_where_expression_list",
  "opt_for_where_expression", "opt_for_select_value_list",
  "opt_for_select_expression_name", "opt_for_select_value",
  "opt_for_order_by", "opt_for_limit", "opt_for_select_columns",
  "opt_for_select_columns_list", "select_quoted_name", "insert_query",
  "opt_for_insert_columns_list", "opt_for_insert_values_list",
  "insert_quoted_name", "insert_quoted_value", "delete_query",
  "table_name", "quoted_custom_value", YY_NULLPTR
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302
};
# endif

#define YYPACT_NINF -71

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-71)))

#define YYTABLE_NINF -30

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int8 yypact[] =
{
     -71,     1,   -71,    34,     3,     0,   -19,    -9,    18,   -71,
     -71,   -71,   -71,   -71,   -71,    29,    52,   -71,   -71,   -71,
     -71,   -71,   -71,   -71,   -71,   -71,   -71,   -71,    -6,    53,
     -71,    51,   -71,    44,    58,    47,   -71,    -9,   -71,   -71,
      -9,    57,   -71,   -71,    45,     4,    60,   -71,    54,    -9,
      61,   -23,    43,   -71,    56,   -71,   -71,    -9,    55,   -71,
      43,   -71,    59,    65,    66,    48,   -71,   -71,    68,    48,
      14,   -71,    15,   -71,   -71,   -14,   -71,    19,    70,    51,
     -71,    51,   -71,   -71,   -71,    30,   -71,   -71,    67,    65,
      66,    66,    14,    14,    62,    14,    14,    24,   -71,   -71,
     -71,    14,    64,   -71,   -71,   -71,   -71,   -71,   -71,    14,
     -71,   -71,   -71,   -71,   -71,    14,    38,   -71,    40,   -71,
      14,   -71,   -71
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       2,     0,     1,     0,    22,     0,     0,     0,     0,     3,
       8,     4,     5,     6,     7,     0,     0,    12,    14,    13,
      23,    24,    30,    25,    26,    27,    28,    15,     0,     0,
      59,    52,    55,     0,     0,    56,    57,     0,    69,    18,
       0,    10,     9,    16,     0,     0,     0,    31,     0,     0,
       0,     0,    34,    11,     0,    21,    20,     0,    53,    32,
      34,    58,     0,     0,     0,    48,    19,    17,     0,    48,
       0,    66,     0,    62,    46,    35,    36,     0,     0,    52,
      54,    52,    72,    71,    70,     0,    64,    67,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    49,    68,    33,
      60,     0,     0,    63,    37,    38,    42,    47,    39,     0,
      40,    41,    50,    51,    65,     0,     0,    44,     0,    43,
       0,    61,    45
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -71,   -71,   -71,   -71,   -71,   -71,   -71,   -71,   -71,    31,
     -71,   -20,   -71,   -71,   -69,    26,   -26,   -71,   -71,    46,
     -71,   -71,   -18,     9,    -2,   -71,    -1,   -70
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
      -1,     1,     9,    10,    11,    57,    28,    29,    12,    65,
      75,    76,   116,    77,   106,    79,    47,    34,    35,    36,
      13,    72,    85,    73,    86,    14,    39,   107
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int8 yytable[] =
{
      87,     2,    43,    62,     3,     4,    37,    63,    17,    18,
     -29,    38,    19,    20,    21,    22,    23,    24,    25,    26,
      30,    55,     5,    31,   108,     6,   110,   111,    44,    90,
      91,    87,    32,    82,    83,    40,    51,     7,    27,    52,
     117,    84,     8,    88,    89,    87,    56,    33,    60,    41,
      92,   122,    93,    98,    15,    99,    67,    16,   100,   101,
      45,    94,   112,   113,    95,    96,   119,   120,   121,   101,
     104,   105,    42,    46,    48,    49,    50,    53,    54,    58,
      64,    30,    59,    66,    68,    71,    74,    80,    78,    70,
      97,    69,   109,   102,   115,    81,    61,   118,   103,   114
};

static const yytype_uint8 yycheck[] =
{
      70,     0,     8,    26,     3,     4,    25,    30,     5,     6,
       7,    20,     9,    10,    11,    12,    13,    14,    15,    16,
      20,    17,    21,    23,    93,    24,    95,    96,    34,    43,
      44,   101,    32,    19,    20,    17,    37,    36,    35,    40,
     109,    27,    41,    28,    29,   115,    42,    47,    49,    20,
      31,   120,    33,    79,    20,    81,    57,    23,    28,    29,
       7,    42,    38,    39,    45,    46,    28,    29,    28,    29,
      90,    91,    20,    22,    30,    17,    29,    20,    33,    19,
      37,    20,    28,    27,    29,    20,    20,    19,    40,    30,
      20,    60,    30,    26,    30,    69,    50,   115,    89,   101
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    49,     0,     3,     4,    21,    24,    36,    41,    50,
      51,    52,    56,    68,    73,    20,    23,     5,     6,     9,
      10,    11,    12,    13,    14,    15,    16,    35,    54,    55,
      20,    23,    32,    47,    65,    66,    67,    25,    20,    74,
      17,    20,    20,     8,    34,     7,    22,    64,    30,    17,
      29,    74,    74,    20,    33,    17,    42,    53,    19,    28,
      74,    67,    26,    30,    37,    57,    27,    74,    29,    57,
      30,    20,    69,    71,    20,    58,    59,    61,    40,    63,
      19,    63,    19,    20,    27,    70,    72,    75,    28,    29,
      43,    44,    31,    33,    42,    45,    46,    20,    64,    64,
      28,    29,    26,    71,    59,    59,    62,    75,    62,    30,
      62,    62,    38,    39,    72,    30,    60,    62,    70,    28,
      29,    28,    62
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    48,    49,    49,    50,    50,    50,    50,    50,    51,
      51,    51,    52,    52,    52,    52,    52,    52,    52,    52,
      53,    53,    54,    54,    54,    54,    54,    54,    54,    55,
      55,    56,    56,    56,    57,    57,    58,    58,    58,    59,
      59,    59,    59,    59,    60,    60,    61,    62,    63,    63,
      63,    63,    64,    64,    64,    65,    65,    66,    66,    67,
      68,    68,    69,    69,    70,    70,    71,    72,    73,    74,
      75,    75,    75
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     0,     2,     1,     1,     1,     1,     1,     3,
       3,     4,     2,     2,     2,     2,     3,     5,     2,     5,
       1,     1,     0,     1,     1,     1,     1,     1,     1,     0,
       1,     3,     4,     7,     0,     2,     1,     3,     3,     3,
       3,     3,     3,     5,     1,     3,     1,     1,     0,     2,
       3,     3,     0,     2,     4,     1,     1,     1,     3,     1,
       7,    10,     1,     3,     1,     3,     1,     1,     6,     1,
       1,     1,     1
};


#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)
#define YYEMPTY         (-2)
#define YYEOF           0

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                  \
do                                                              \
  if (yychar == YYEMPTY)                                        \
    {                                                           \
      yychar = (Token);                                         \
      yylval = (Value);                                         \
      YYPOPSTACK (yylen);                                       \
      yystate = *yyssp;                                         \
      goto yybackup;                                            \
    }                                                           \
  else                                                          \
    {                                                           \
      yyerror (qInfo, YY_("syntax error: cannot back up")); \
      YYERROR;                                                  \
    }                                                           \
while (0)

/* Error token number */
#define YYTERROR        1
#define YYERRCODE       256



/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)

/* This macro is provided for backward compatibility. */
#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#endif


# define YY_SYMBOL_PRINT(Title, Type, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Type, Value, qInfo); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*----------------------------------------.
| Print this symbol's value on YYOUTPUT.  |
`----------------------------------------*/

static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, void* qInfo)
{
  FILE *yyo = yyoutput;
  YYUSE (yyo);
  YYUSE (qInfo);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
  YYUSE (yytype);
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, void* qInfo)
{
  YYFPRINTF (yyoutput, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep, qInfo);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yytype_int16 *yyssp, YYSTYPE *yyvsp, int yyrule, void* qInfo)
{
  unsigned long int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       yystos[yyssp[yyi + 1 - yynrhs]],
                       &(yyvsp[(yyi + 1) - (yynrhs)])
                                              , qInfo);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule, qInfo); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
yystrlen (const char *yystr)
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
yystpcpy (char *yydest, const char *yysrc)
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
        switch (*++yyp)
          {
          case '\'':
          case ',':
            goto do_not_strip_quotes;

          case '\\':
            if (*++yyp != '\\')
              goto do_not_strip_quotes;
            /* Fall through.  */
          default:
            if (yyres)
              yyres[yyn] = *yyp;
            yyn++;
            break;

          case '"':
            if (yyres)
              yyres[yyn] = '\0';
            return yyn;
          }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYSIZE_T *yymsg_alloc, char **yymsg,
                yytype_int16 *yyssp, int yytoken)
{
  YYSIZE_T yysize0 = yytnamerr (YY_NULLPTR, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[*yyssp];
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;

          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                {
                  YYSIZE_T yysize1 = yysize + yytnamerr (YY_NULLPTR, yytname[yyx]);
                  if (! (yysize <= yysize1
                         && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
                    return 2;
                  yysize = yysize1;
                }
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  {
    YYSIZE_T yysize1 = yysize + yystrlen (yyformat);
    if (! (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
      return 2;
    yysize = yysize1;
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          yyp++;
          yyformat++;
        }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep, void* qInfo)
{
  YYUSE (yyvaluep);
  YYUSE (qInfo);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yytype);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}




/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;


/*----------.
| yyparse.  |
`----------*/

int
yyparse (void* qInfo)
{
    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       'yyss': related to states.
       'yyvs': related to semantic values.

       Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */
  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        YYSTYPE *yyvs1 = yyvs;
        yytype_int16 *yyss1 = yyss;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * sizeof (*yyssp),
                    &yyvs1, yysize * sizeof (*yyvsp),
                    &yystacksize);

        yyss = yyss1;
        yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yytype_int16 *yyss1 = yyss;
        union yyalloc *yyptr =
          (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
        if (! yyptr)
          goto yyexhaustedlab;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
                  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = yylex (qInfo);
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 9:
#line 202 "../src/CometQL.y" /* yacc.c:1646  */
    {
                setQData;
                qData->command = TOK_SET;
                setTokValue(qData, qData->arg_set.varible, (yyvsp[-1].tokStruct));  
                setTokValue(qData, qData->arg_set.value, (yyvsp[0].tokStruct));  
                //printf("\tСписок баз данных\n");
        }
#line 1516 "../src/CometQL.cc" /* yacc.c:1646  */
    break;

  case 10:
#line 211 "../src/CometQL.y" /* yacc.c:1646  */
    {
                setQData;
                qData->command = TOK_SET; 
                setTokValue(qData, qData->arg_set.varible, (yyvsp[-1].tokStruct));  
                setTokValue(qData, qData->arg_set.value, (yyvsp[0].tokStruct));   
        }
#line 1527 "../src/CometQL.cc" /* yacc.c:1646  */
    break;

  case 11:
#line 219 "../src/CometQL.y" /* yacc.c:1646  */
    {
                setQData;
                qData->command = TOK_SET;   
                setTokValue(qData, qData->arg_set.section, (yyvsp[-2].tokStruct)); 
                setTokValue(qData, qData->arg_set.varible, (yyvsp[-1].tokStruct));  
                setTokValue(qData, qData->arg_set.value, (yyvsp[0].tokStruct));  
        }
#line 1539 "../src/CometQL.cc" /* yacc.c:1646  */
    break;

  case 12:
#line 232 "../src/CometQL.y" /* yacc.c:1646  */
    { 
                setQData;
                qData->command = TOK_SHOW;
                qData->arg_show.command = TOK_DATABASES; 
                //printf("\tСписок баз данных\n");
        }
#line 1550 "../src/CometQL.cc" /* yacc.c:1646  */
    break;

  case 13:
#line 240 "../src/CometQL.y" /* yacc.c:1646  */
    {
                setQData;
                qData->command = TOK_SHOW;
                qData->arg_show.command = TOK_PROCESSLIST; 
                //printf("\tСписок потоков\n");
        }
#line 1561 "../src/CometQL.cc" /* yacc.c:1646  */
    break;

  case 14:
#line 248 "../src/CometQL.y" /* yacc.c:1646  */
    {
                setQData;
                qData->command = TOK_SHOW;
                qData->arg_show.command = TOK_TABLES; 
                //printf("\tСписок таблиц\n");
        }
#line 1572 "../src/CometQL.cc" /* yacc.c:1646  */
    break;

  case 15:
#line 256 "../src/CometQL.y" /* yacc.c:1646  */
    {
                setQData;
                qData->command = TOK_SHOW;
                qData->arg_show.command = TOK_TABLE_STATUS; 
                
                //printf("\tСписок таблиц\n");
        }
#line 1584 "../src/CometQL.cc" /* yacc.c:1646  */
    break;

  case 16:
#line 265 "../src/CometQL.y" /* yacc.c:1646  */
    {
                setQData;
                qData->command = TOK_SHOW;
                qData->arg_show.command = TOK_STATUS; 
                //printf("\tСтатус системы\n");
        }
#line 1595 "../src/CometQL.cc" /* yacc.c:1646  */
    break;

  case 17:
#line 273 "../src/CometQL.y" /* yacc.c:1646  */
    {
                // Тоже самое что и TOK_DESCRIBE VAL_NAME 
                setQData;
                qData->command = TOK_SHOW;
                qData->arg_show.command = TOK_COLUMNS;  
                //printf("\tСписок колонок\n");
        }
#line 1607 "../src/CometQL.cc" /* yacc.c:1646  */
    break;

  case 18:
#line 282 "../src/CometQL.y" /* yacc.c:1646  */
    {
                // Тоже самое что и TOK_SHOW opt_for_show_columns TOK_COLUMNS TOK_FROM VAL_NAME 
                setQData;
                qData->command = TOK_SHOW;
                qData->arg_show.command = TOK_COLUMNS;  
                //printf("\tСписок колонок\n");
        }
#line 1619 "../src/CometQL.cc" /* yacc.c:1646  */
    break;

  case 19:
#line 291 "../src/CometQL.y" /* yacc.c:1646  */
    {
                setQData;
                qData->command = TOK_SHOW;
                qData->arg_show.command = TOK_VARIABLES; 
                
                setTokValue(qData, qData->arg_show.like, (yyvsp[0].tokStruct)); 
                //printf("\tПоказать переменные\n");
        }
#line 1632 "../src/CometQL.cc" /* yacc.c:1646  */
    break;

  case 23:
#line 311 "../src/CometQL.y" /* yacc.c:1646  */
    {
                setQData;
                qData->arg_show.flag = FLAG_GLOBAL;
        }
#line 1641 "../src/CometQL.cc" /* yacc.c:1646  */
    break;

  case 24:
#line 317 "../src/CometQL.y" /* yacc.c:1646  */
    {
                setQData;
                qData->arg_show.flag = FLAG_SESSION;
        }
#line 1650 "../src/CometQL.cc" /* yacc.c:1646  */
    break;

  case 25:
#line 323 "../src/CometQL.y" /* yacc.c:1646  */
    {
                setQData;
                qData->arg_show.flag = FLAG_FILESYSTEM;
        }
#line 1659 "../src/CometQL.cc" /* yacc.c:1646  */
    break;

  case 26:
#line 329 "../src/CometQL.y" /* yacc.c:1646  */
    {
                setQData;
                qData->arg_show.flag = FLAG_RAM;
        }
#line 1668 "../src/CometQL.cc" /* yacc.c:1646  */
    break;

  case 27:
#line 335 "../src/CometQL.y" /* yacc.c:1646  */
    {
                setQData;
                qData->arg_show.flag = FLAG_AVG;
        }
#line 1677 "../src/CometQL.cc" /* yacc.c:1646  */
    break;

  case 28:
#line 341 "../src/CometQL.y" /* yacc.c:1646  */
    {
                setQData;
                qData->arg_show.flag = FLAG_UPTIME;
        }
#line 1686 "../src/CometQL.cc" /* yacc.c:1646  */
    break;

  case 30:
#line 351 "../src/CometQL.y" /* yacc.c:1646  */
    {
                setQData;
                qData->arg_show.flag = FLAG_FULL;
        }
#line 1695 "../src/CometQL.cc" /* yacc.c:1646  */
    break;

  case 31:
#line 362 "../src/CometQL.y" /* yacc.c:1646  */
    {
                setQData;
                qData->command = TOK_SELECT;
                qData->arg_select.command = VAL_SYSTEM_VARIBLE; 
                setTokValue(qData, qData->arg_select.systemVaribleName, (yyvsp[-1].tokStruct));  
                
                //printf("\tВыборка %s\n", $2.text);
        }
#line 1708 "../src/CometQL.cc" /* yacc.c:1646  */
    break;

  case 32:
#line 372 "../src/CometQL.y" /* yacc.c:1646  */
    {
                setQData;
                qData->command = TOK_SELECT;
                qData->arg_select.command = TOK_DATABASE;  
                //printf("\tВыборка DATABASE\n");
        }
#line 1719 "../src/CometQL.cc" /* yacc.c:1646  */
    break;

  case 33:
#line 380 "../src/CometQL.y" /* yacc.c:1646  */
    {
                setQData; 
                qData->command = TOK_SELECT;
                qData->arg_select.command = TOK_FROM;  
        }
#line 1729 "../src/CometQL.cc" /* yacc.c:1646  */
    break;

  case 36:
#line 395 "../src/CometQL.y" /* yacc.c:1646  */
    {
                setQData; 
                if(qData->where.whereExprCount >= MAX_EXPRESSIONS_COUNT)
                { 
                    qData->setError("Error in expression list, too many conditions.", SQL_ERR_TOO_MANY_CONDITIONS);  
                    qData->where.whereExprCount=0;
                }
                qData->where.whereExprValueCount = 0;
                qData->where.whereExprCount++;
        }
#line 1744 "../src/CometQL.cc" /* yacc.c:1646  */
    break;

  case 37:
#line 407 "../src/CometQL.y" /* yacc.c:1646  */
    {
            /** 
             * нет скобочек для порядка выполнения
             */
                setQData; 
                if(qData->where.whereExprCount >= MAX_EXPRESSIONS_COUNT)
                { 
                    qData->setError("Error in expression list, too many conditions.", SQL_ERR_TOO_MANY_CONDITIONS);  
                    qData->where.whereExprCount=0;
                }
                qData->where.whereExprOperator[qData->where.whereExprCount] = TOK_OR;
                qData->where.whereExprCount++;
                qData->where.whereExprValueCount = 0;
        }
#line 1763 "../src/CometQL.cc" /* yacc.c:1646  */
    break;

  case 38:
#line 423 "../src/CometQL.y" /* yacc.c:1646  */
    {
            /** 
             * нет скобочек для порядка выполнения
             */
                setQData; 
                if(qData->where.whereExprCount >= MAX_EXPRESSIONS_COUNT)
                { 
                    qData->setError("Error in expression list, too many conditions.", SQL_ERR_TOO_MANY_CONDITIONS);  
                    qData->where.whereExprCount=0;
                }
                qData->where.whereExprOperator[qData->where.whereExprCount] = TOK_AND;
                qData->where.whereExprCount++;
                qData->where.whereExprValueCount = 0;
        }
#line 1782 "../src/CometQL.cc" /* yacc.c:1646  */
    break;

  case 39:
#line 442 "../src/CometQL.y" /* yacc.c:1646  */
    {
                setQData; 
                qData->where.whereExprCondition[qData->where.whereExprCount] = TOK_LIKE;
        }
#line 1791 "../src/CometQL.cc" /* yacc.c:1646  */
    break;

  case 40:
#line 448 "../src/CometQL.y" /* yacc.c:1646  */
    {
                setQData; 
                qData->where.whereExprCondition[qData->where.whereExprCount] = MORE;
        }
#line 1800 "../src/CometQL.cc" /* yacc.c:1646  */
    break;

  case 41:
#line 454 "../src/CometQL.y" /* yacc.c:1646  */
    {
                setQData; 
                qData->where.whereExprCondition[qData->where.whereExprCount] = LESS;
        }
#line 1809 "../src/CometQL.cc" /* yacc.c:1646  */
    break;

  case 42:
#line 460 "../src/CometQL.y" /* yacc.c:1646  */
    {
                setQData; 
                qData->where.whereExprCondition[qData->where.whereExprCount] = EQUALLY;
        }
#line 1818 "../src/CometQL.cc" /* yacc.c:1646  */
    break;

  case 43:
#line 466 "../src/CometQL.y" /* yacc.c:1646  */
    {
                setQData; 
                qData->where.whereExprCondition[qData->where.whereExprCount] = TOK_IN;
        }
#line 1827 "../src/CometQL.cc" /* yacc.c:1646  */
    break;

  case 46:
#line 480 "../src/CometQL.y" /* yacc.c:1646  */
    { 
                setQData;  
                setTokValue(qData, qData->where.whereExprColum[qData->where.whereExprCount], (yyvsp[0].tokStruct));   
                //printf("\texpr_Name %d[%.3s...]\n", qData->where.whereExprCount, $1.text);
        }
#line 1837 "../src/CometQL.cc" /* yacc.c:1646  */
    break;

  case 47:
#line 489 "../src/CometQL.y" /* yacc.c:1646  */
    {
                setQData; 
                if(qData->where.whereExprValueCount >= MAX_EXPRESSIONS_VALUES)
                { 
                    qData->setError("Error in expression list, too many values.", SQL_ERR_TOO_MANY_VALUES);  
                    qData->where.whereExprValueCount=0;
                }
                else
                {
                    setTokValue(qData, qData->where.whereExprValue[qData->where.whereExprCount][qData->where.whereExprValueCount], (yyvsp[0].tokStruct));   
                    qData->where.whereExprValueCount++;
                    //printf("\texpr_Value %d[%.3s...]\n", qData->where.whereExprValueCount, $1.text);
                }
        }
#line 1856 "../src/CometQL.cc" /* yacc.c:1646  */
    break;

  case 49:
#line 509 "../src/CometQL.y" /* yacc.c:1646  */
    { 
                setQData; 
                setTokValue(qData, qData->orderBy.name, (yyvsp[-1].tokStruct));   
                qData->orderBy.type = TOK_ASC;
        }
#line 1866 "../src/CometQL.cc" /* yacc.c:1646  */
    break;

  case 50:
#line 516 "../src/CometQL.y" /* yacc.c:1646  */
    { 
                setQData; 
                setTokValue(qData, qData->orderBy.name, (yyvsp[-1].tokStruct));  
                qData->orderBy.type = TOK_ASC;
        }
#line 1876 "../src/CometQL.cc" /* yacc.c:1646  */
    break;

  case 51:
#line 523 "../src/CometQL.y" /* yacc.c:1646  */
    { 
                setQData; 
                setTokValue(qData, qData->orderBy.name, (yyvsp[-1].tokStruct));   
                qData->orderBy.type = TOK_DESC;
        }
#line 1886 "../src/CometQL.cc" /* yacc.c:1646  */
    break;

  case 53:
#line 534 "../src/CometQL.y" /* yacc.c:1646  */
    {
                setQData;
                qData->limit.rows = strToInt((yyvsp[0].tokStruct).text, (yyvsp[0].tokStruct).len);
                
                //printf("\tВыборка limit %d\n", qData->limit.rows);
        }
#line 1897 "../src/CometQL.cc" /* yacc.c:1646  */
    break;

  case 54:
#line 542 "../src/CometQL.y" /* yacc.c:1646  */
    {
                setQData;
                qData->limit.setStart(strToInt((yyvsp[0].tokStruct).text, (yyvsp[0].tokStruct).len));
                qData->limit.setRows(strToInt((yyvsp[-2].tokStruct).text, (yyvsp[-2].tokStruct).len));
                
                //printf("\tВыборка limit_start %d, limit %d\n", qData->limit.start, qData->limit.rows);
        }
#line 1909 "../src/CometQL.cc" /* yacc.c:1646  */
    break;

  case 55:
#line 553 "../src/CometQL.y" /* yacc.c:1646  */
    { 
                setQData; 
                qData->arg_select.selectedColumnsCount=0;
        }
#line 1918 "../src/CometQL.cc" /* yacc.c:1646  */
    break;

  case 59:
#line 569 "../src/CometQL.y" /* yacc.c:1646  */
    {
                setQData; 
                if(qData->arg_select.selectedColumnsCount >= MAX_COLUMNS_COUNT)
                { 
                    qData->setError("Error in columns list, too many columns.", SQL_ERR_TOO_MANY_COLUMNS);  
                    qData->arg_select.selectedColumnsCount=0;
                }
                else
                {
                    setTokValue(qData, qData->arg_select.selectedColumns[qData->arg_select.selectedColumnsCount], (yyvsp[0].tokStruct));    
                    qData->arg_select.selectedColumnsCount++;
                    //printf("\tQUOTED_Name %d[%.3s...]\n", qData->arg_select.selectedColumnsCount, $1.text);
                }
        }
#line 1937 "../src/CometQL.cc" /* yacc.c:1646  */
    break;

  case 60:
#line 590 "../src/CometQL.y" /* yacc.c:1646  */
    {
                setQData;
                qData->command = TOK_INSERT;  
                //printf("\tВставка\n");
        }
#line 1947 "../src/CometQL.cc" /* yacc.c:1646  */
    break;

  case 61:
#line 597 "../src/CometQL.y" /* yacc.c:1646  */
    {
                setQData;
                qData->command = TOK_INSERT;         
                //printf("\tВставка\n"); 
        }
#line 1957 "../src/CometQL.cc" /* yacc.c:1646  */
    break;

  case 66:
#line 618 "../src/CometQL.y" /* yacc.c:1646  */
    {
                setQData; 
                if(qData->arg_insert.namesCount >= MAX_COLUMNS_COUNT)
                { 
                    qData->setError("Error in names list, too many names.", SQL_ERR_TOO_MANY_NAMES);  
                    qData->arg_insert.namesCount=0;
                }
                else
                {
                    setTokValue(qData, qData->arg_insert.names[qData->arg_insert.namesCount], (yyvsp[0].tokStruct));    
                    qData->arg_insert.namesCount++;
                    //printf("\tQUOTED_Name %d[%.3s...]\n", qData->arg_insert.namesCount, $1.text);
                }
        }
#line 1976 "../src/CometQL.cc" /* yacc.c:1646  */
    break;

  case 67:
#line 636 "../src/CometQL.y" /* yacc.c:1646  */
    {
                setQData;
                if(qData->arg_insert.valuesCount >= MAX_EXPRESSIONS_VALUES)
                { 
                    qData->setError("Error in names list, too many values.", SQL_ERR_TOO_MANY_VALUES);  
                    qData->arg_insert.valuesCount=0;
                }
                else
                {
                    setTokValue(qData, qData->arg_insert.values[qData->arg_insert.valuesCount], (yyvsp[0].tokStruct));     
                    qData->arg_insert.valuesCount++;
                    //printf("\tQUOTED_Value %d[%.3s...%d]\n", qData->arg_insert.valuesCount, $1.text, $1.len);
                }
        }
#line 1995 "../src/CometQL.cc" /* yacc.c:1646  */
    break;

  case 68:
#line 654 "../src/CometQL.y" /* yacc.c:1646  */
    {
                setQData;
                qData->command = TOK_DELETE;  
                //printf("\tУдаление\n");
        }
#line 2005 "../src/CometQL.cc" /* yacc.c:1646  */
    break;

  case 69:
#line 671 "../src/CometQL.y" /* yacc.c:1646  */
    { 
                setQData;
                setTokValue(qData, qData->tableName, (yyvsp[0].tokStruct));   
                //printf("\tTable_Name [%.3s...%d]\n", $1.text, $1.len);
        }
#line 2015 "../src/CometQL.cc" /* yacc.c:1646  */
    break;


#line 2019 "../src/CometQL.cc" /* yacc.c:1646  */
      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (qInfo, YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = (char *) YYSTACK_ALLOC (yymsg_alloc);
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (qInfo, yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval, qInfo);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYTERROR;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;


      yydestruct ("Error: popping",
                  yystos[yystate], yyvsp, qInfo);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#if !defined yyoverflow || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (qInfo, YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval, qInfo);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  yystos[*yyssp], yyvsp, qInfo);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  return yyresult;
}
#line 688 "../src/CometQL.y" /* yacc.c:1906  */

        
        
