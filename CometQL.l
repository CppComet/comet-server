%{
#include <stdio.h>
#include "y.tab.h"   
  
#define setTokData yylval.tokStruct.text = yytext; yylval.tokStruct.len = yyleng;
#define setTokDataNoQUOTE yylval.tokStruct.text = yytext+1; yylval.tokStruct.len = yyleng-2; yylval.tokStruct.quote = yytext[0];
    
    /**
     * @note В дальнейшем в sql запросы введу ключевые сллова sync и async для указания в каком режиме работать с кластером
     * Слово sync будет требоватся в тех случаях если надо получить в ответе точное количество затронутых строк
     * async режим по умолчанию в ответ возвращает нижнею границу количества затронутых строк. Так как на других серверах это значение ещё не расчитано из за асинхронной работы команд.
     */
%}

%%
">"                     {  setTokData return MORE;}
"<"                     {  setTokData return LESS;}
","                     {  setTokData return COMMA;}
"="                     {  setTokData return EQUALLY;}
"*"                     {  setTokData return ASTERISK;}
"("                     {  setTokData return BRACKETS_LEFT;}
")"                     {  setTokData return BRACKETS_RIGHT;}
IN                      {  setTokData return TOK_IN;}
OR                      {  setTokData return TOK_OR;}
AND                     {  setTokData return TOK_AND;}
USE                     {  setTokData return TOK_USE;}
ASC                     {  setTokData return TOK_ASC;} 
RAM                     {  setTokData return FLAG_RAM;}
AVG                     {  setTokData return FLAG_AVG;}
DESC                    {  setTokData return TOK_DESC;}
SHOW                    {  setTokData return TOK_SHOW;}
FROM                    {  setTokData return TOK_FROM;}
INTO                    {  setTokData return TOK_INTO;}
LIKE                    {  setTokData return TOK_LIKE;}
LIMIT                   {  setTokData return TOK_LIMIT;}
FULL                    {  setTokData return FLAG_FULL;} 
WHERE                   {  setTokData return TOK_WHERE;}
TABLES                  {  setTokData return TOK_TABLES;}
DELETE                  {  setTokData return TOK_DELETE;}
STATUS                  {  setTokData return TOK_STATUS;}
VALUES                  {  setTokData return TOK_VALUES;}
SELECT                  {  setTokData return TOK_SELECT;}
INSERT                  {  setTokData return TOK_INSERT;}
COLUMNS                 {  setTokData return TOK_COLUMNS;}
GLOBAL                  {  setTokData return FLAG_GLOBAL;}
UPTIME                  {  setTokData return FLAG_UPTIME;}
"ORDER BY"              {  setTokData return TOK_ORDER_BY;}
SESSION                 {  setTokData return FLAG_SESSION;}
DESCRIBE                {  setTokData return TOK_DESCRIBE;}
DATABASES               {  setTokData return TOK_DATABASES;}
"DATABASE"              {  setTokData return TOK_DATABASE;}
VARIABLES               {  setTokData return TOK_VARIABLES;}
FILESYSTEM              {  setTokData return FLAG_FILESYSTEM;}
PROCESSLIST             {  setTokData return TOK_PROCESSLIST;}
"TABLE STATUS"          {  setTokData return TOK_TABLE_STATUS;}
@@[a-z\_]+[a-z0-9\_]*   {  setTokData return VAL_SYSTEM_VARIBLE;}
[0-9]+                  {  setTokData return VAL_INT;}
\"[0-9]+\"              {  setTokDataNoQUOTE return VAL_INT;}
'[0-9]+'                {  setTokDataNoQUOTE return VAL_INT;}
`[0-9]+`                {  setTokDataNoQUOTE return VAL_INT;}
[a-z\_]+[a-z0-9\_]*     {  setTokData return VAL_NAME;}
\"[a-z\_]+[a-z0-9\_]*\" {  setTokDataNoQUOTE return VAL_NAME;}
'[a-z\_]+[a-z0-9\_]*'   {  setTokDataNoQUOTE return VAL_NAME;}
`[a-z\_]+[a-z0-9\_]*`   {  setTokDataNoQUOTE return VAL_NAME;}
'(\\'|[^'])+'|''        {  setTokDataNoQUOTE return VAL_QUOTED_STRING;} 
\"(\\\"|[^\"])+\"|\"\"  {  setTokDataNoQUOTE return VAL_QUOTED_STRING;}
%%