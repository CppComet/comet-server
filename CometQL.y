%{
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
            fprintf(stderr,"ошибка: %s\n",str);
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
                    
%}
%lex-param	{ void* qInfo }
%parse-param	{ void* qInfo }

%union {
   struct TokStruct {
      char* text;
      int len;
      char quote;
   } tokStruct;
}

%token <tokStruct> TOK_SHOW
%token <tokStruct> TOK_DATABASES
%token <tokStruct> TOK_TABLES
%token <tokStruct> TOK_COLUMNS
%token <tokStruct> TOK_STATUS
%token <tokStruct> TOK_PROCESSLIST

%token <tokStruct> FLAG_GLOBAL
%token <tokStruct> FLAG_SESSION
%token <tokStruct> FLAG_FULL
%token <tokStruct> FLAG_FILESYSTEM
%token <tokStruct> FLAG_RAM
%token <tokStruct> FLAG_AVG
%token <tokStruct> FLAG_UPTIME

%token <tokStruct> TOK_FROM

%token <tokStruct> TOK_USE
%token <tokStruct> VAL_INT
%token <tokStruct> VAL_NAME 

%token <tokStruct> TOK_SELECT
%token <tokStruct> TOK_LIMIT
%token <tokStruct> VAL_SYSTEM_VARIBLE

%token <tokStruct> TOK_INSERT
%token <tokStruct> TOK_INTO
%token <tokStruct> TOK_VALUES
%token <tokStruct> VAL_QUOTED_STRING

%token <tokStruct> BRACKETS_RIGHT
%token <tokStruct> COMMA
%token <tokStruct> BRACKETS_LEFT
%token <tokStruct> EQUALLY
%token <tokStruct> ASTERISK

%token <tokStruct> TOK_LIKE
%token <tokStruct> TOK_VARIABLES
%token <tokStruct> TOK_TABLE_STATUS
%token <tokStruct> TOK_DESCRIBE

%token <tokStruct> TOK_WHERE
%token <tokStruct> TOK_ASC
%token <tokStruct> TOK_DESC
%token <tokStruct> TOK_ORDER_BY
%token <tokStruct> TOK_DELETE

%token <tokStruct> TOK_IN
%token <tokStruct> TOK_OR
%token <tokStruct> TOK_AND
%token <tokStruct> MORE
%token <tokStruct> LESS 

%token <tokStruct> TOK_DATABASE

%type <tokStruct> quoted_custom_value
%%
commands: /* empty */
        | commands command
        ;

command:
        show_query
        |
        select_query
        |
        insert_query
        |
        delete_query 
        ;

/**
 * SHOW
 */
show_query:
        TOK_SHOW TOK_DATABASES
        { 
                setQData;
                qData->command = TOK_SHOW;
                qData->arg_show.command = TOK_DATABASES; 
                //printf("\tСписок баз данных\n");
        }
        |
        TOK_SHOW TOK_PROCESSLIST
        {
                setQData;
                qData->command = TOK_SHOW;
                qData->arg_show.command = TOK_PROCESSLIST; 
                //printf("\tСписок потоков\n");
        }
        |
        TOK_SHOW TOK_TABLES
        {
                setQData;
                qData->command = TOK_SHOW;
                qData->arg_show.command = TOK_TABLES; 
                //printf("\tСписок таблиц\n");
        }
        |
        TOK_SHOW TOK_TABLE_STATUS
        {
                setQData;
                qData->command = TOK_SHOW;
                qData->arg_show.command = TOK_TABLE_STATUS; 
                
                //printf("\tСписок таблиц\n");
        }
        |
        TOK_SHOW opt_for_show_status TOK_STATUS
        {
                setQData;
                qData->command = TOK_SHOW;
                qData->arg_show.command = TOK_STATUS; 
                //printf("\tСтатус системы\n");
        }
        |
        TOK_SHOW opt_for_show_columns TOK_COLUMNS tok_from_or_in table_name 
        {
                // Тоже самое что и TOK_DESCRIBE VAL_NAME 
                setQData;
                qData->command = TOK_SHOW;
                qData->arg_show.command = TOK_COLUMNS;  
                //printf("\tСписок колонок\n");
        }
        |
        TOK_DESCRIBE table_name 
        {
                // Тоже самое что и TOK_SHOW opt_for_show_columns TOK_COLUMNS TOK_FROM VAL_NAME 
                setQData;
                qData->command = TOK_SHOW;
                qData->arg_show.command = TOK_COLUMNS;  
                //printf("\tСписок колонок\n");
        }
        |
        TOK_SHOW opt_for_show_status TOK_VARIABLES TOK_LIKE VAL_QUOTED_STRING
        {
                setQData;
                qData->command = TOK_SHOW;
                qData->arg_show.command = TOK_VARIABLES; 
                
                setTokValue(qData, qData->arg_show.like, $5); 
                //printf("\tПоказать переменные\n");
        }
        ;
        
tok_from_or_in:
        TOK_IN
        |
        TOK_FROM
        ;
        
opt_for_show_status:
        // nothing
        |
        FLAG_GLOBAL
        {
                setQData;
                qData->arg_show.flag = FLAG_GLOBAL;
        }
        |
        FLAG_SESSION
        {
                setQData;
                qData->arg_show.flag = FLAG_SESSION;
        }
        |
        FLAG_FILESYSTEM
        {
                setQData;
                qData->arg_show.flag = FLAG_FILESYSTEM;
        }
        |
        FLAG_RAM
        {
                setQData;
                qData->arg_show.flag = FLAG_RAM;
        }
        |
        FLAG_AVG
        {
                setQData;
                qData->arg_show.flag = FLAG_AVG;
        }
        |
        FLAG_UPTIME
        {
                setQData;
                qData->arg_show.flag = FLAG_UPTIME;
        }
        ;
        
opt_for_show_columns:
        // nothing
        |
        FLAG_FULL
        {
                setQData;
                qData->arg_show.flag = FLAG_FULL;
        }
        ;

/**
 * SELECT
 */
select_query:
        TOK_SELECT VAL_SYSTEM_VARIBLE opt_for_limit
        {
                setQData;
                qData->command = TOK_SELECT;
                qData->arg_select.command = VAL_SYSTEM_VARIBLE; 
                setTokValue(qData, qData->arg_select.systemVaribleName, $2);  
                
                //printf("\tВыборка %s\n", $2.text);
        }
        |
        TOK_SELECT TOK_DATABASE BRACKETS_LEFT BRACKETS_RIGHT
        {
                setQData;
                qData->command = TOK_SELECT;
                qData->arg_select.command = TOK_DATABASE;  
                //printf("\tВыборка DATABASE\n");
        }
        |
        TOK_SELECT opt_for_select_columns TOK_FROM table_name opt_for_where opt_for_order_by opt_for_limit
        {
                setQData; 
                qData->command = TOK_SELECT;
                qData->arg_select.command = TOK_FROM;  
        }
        ;
         
opt_for_where:
        // nothing
        |
        TOK_WHERE opt_for_where_expression_list
        ;

opt_for_where_expression_list:
        opt_for_where_expression 
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
        |
        opt_for_where_expression_list TOK_OR opt_for_where_expression 
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
        |
        opt_for_where_expression_list TOK_AND opt_for_where_expression 
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
        ;


opt_for_where_expression:
        opt_for_select_expression_name TOK_LIKE opt_for_select_value
        {
                setQData; 
                qData->where.whereExprCondition[qData->where.whereExprCount] = TOK_LIKE;
        }
        |
        opt_for_select_expression_name MORE opt_for_select_value
        {
                setQData; 
                qData->where.whereExprCondition[qData->where.whereExprCount] = MORE;
        }
        |
        opt_for_select_expression_name LESS opt_for_select_value
        {
                setQData; 
                qData->where.whereExprCondition[qData->where.whereExprCount] = LESS;
        }
        |
        opt_for_select_expression_name EQUALLY opt_for_select_value
        {
                setQData; 
                qData->where.whereExprCondition[qData->where.whereExprCount] = EQUALLY;
        }
        |
        opt_for_select_expression_name TOK_IN BRACKETS_LEFT opt_for_select_value_list BRACKETS_RIGHT
        {
                setQData; 
                qData->where.whereExprCondition[qData->where.whereExprCount] = TOK_IN;
        }
        ;

opt_for_select_value_list:
        opt_for_select_value 
        |
        opt_for_select_value_list COMMA opt_for_select_value 
        ;

opt_for_select_expression_name:
        VAL_NAME 
        { 
                setQData;  
                setTokValue(qData, qData->where.whereExprColum[qData->where.whereExprCount], $1);   
                //printf("\texpr_Name %d[%.3s...]\n", qData->where.whereExprCount, $1.text);
        }
        ;
        
opt_for_select_value:
        quoted_custom_value 
        {
                setQData; 
                if(qData->where.whereExprValueCount >= MAX_EXPRESSIONS_VALUES)
                { 
                    qData->setError("Error in expression list, too many values.", SQL_ERR_TOO_MANY_VALUES);  
                    qData->where.whereExprValueCount=0;
                }
                else
                {
                    setTokValue(qData, qData->where.whereExprValue[qData->where.whereExprCount][qData->where.whereExprValueCount], $1);   
                    qData->where.whereExprValueCount++;
                    //printf("\texpr_Value %d[%.3s...]\n", qData->where.whereExprValueCount, $1.text);
                }
        }
        ;

opt_for_order_by:
        // nothing
        |
        TOK_ORDER_BY VAL_NAME 
        { 
                setQData; 
                setTokValue(qData, qData->orderBy.name, $1);   
                qData->orderBy.type = TOK_ASC;
        }
        |
        TOK_ORDER_BY VAL_NAME TOK_ASC
        { 
                setQData; 
                setTokValue(qData, qData->orderBy.name, $2);  
                qData->orderBy.type = TOK_ASC;
        }
        |
        TOK_ORDER_BY VAL_NAME TOK_DESC
        { 
                setQData; 
                setTokValue(qData, qData->orderBy.name, $2);   
                qData->orderBy.type = TOK_DESC;
        }
        ;
        
opt_for_limit:
        // nothing
        |
        TOK_LIMIT VAL_INT
        {
                setQData;
                qData->limit.rows = strToInt($2.text, $2.len);
                
                //printf("\tВыборка limit %d\n", qData->limit.rows);
        }
        |
        TOK_LIMIT VAL_INT COMMA VAL_INT
        {
                setQData;
                qData->limit.setStart(strToInt($4.text, $4.len));
                qData->limit.setRows(strToInt($2.text, $2.len));
                
                //printf("\tВыборка limit_start %d, limit %d\n", qData->limit.start, qData->limit.rows);
        }
        ;
  
opt_for_select_columns:  
        ASTERISK
        { 
                setQData; 
                qData->arg_select.selectedColumnsCount=0;
        }
        |
        opt_for_select_columns_list
        ;

opt_for_select_columns_list:  
        select_quoted_name 
        |
        opt_for_select_columns_list COMMA select_quoted_name 
        ;

select_quoted_name:
        VAL_NAME
        {
                setQData; 
                if(qData->arg_select.selectedColumnsCount >= MAX_COLUMNS_COUNT)
                { 
                    qData->setError("Error in columns list, too many columns.", SQL_ERR_TOO_MANY_COLUMNS);  
                    qData->arg_select.selectedColumnsCount=0;
                }
                else
                {
                    setTokValue(qData, qData->arg_select.selectedColumns[qData->arg_select.selectedColumnsCount], $1);    
                    qData->arg_select.selectedColumnsCount++;
                    //printf("\tQUOTED_Name %d[%.3s...]\n", qData->arg_select.selectedColumnsCount, $1.text);
                }
        }
        ;
        
/**
 * INSERT
 */
insert_query:
        TOK_INSERT TOK_INTO table_name TOK_VALUES BRACKETS_LEFT opt_for_insert_values_list BRACKETS_RIGHT 
        {
                setQData;
                qData->command = TOK_INSERT;  
                //printf("\tВставка\n");
        }
        |
        TOK_INSERT TOK_INTO table_name BRACKETS_LEFT opt_for_insert_columns_list BRACKETS_RIGHT TOK_VALUES BRACKETS_LEFT opt_for_insert_values_list BRACKETS_RIGHT 
        {
                setQData;
                qData->command = TOK_INSERT;         
                //printf("\tВставка\n"); 
        }
        ;
        
opt_for_insert_columns_list:  
        insert_quoted_name 
        |
        opt_for_insert_columns_list COMMA insert_quoted_name 
        ;

opt_for_insert_values_list:
        insert_quoted_value 
        |
        opt_for_insert_values_list COMMA insert_quoted_value
        ;
        
insert_quoted_name:
        VAL_NAME
        {
                setQData; 
                if(qData->arg_insert.namesCount >= MAX_COLUMNS_COUNT)
                { 
                    qData->setError("Error in names list, too many names.", SQL_ERR_TOO_MANY_NAMES);  
                    qData->arg_insert.namesCount=0;
                }
                else
                {
                    setTokValue(qData, qData->arg_insert.names[qData->arg_insert.namesCount], $1);    
                    qData->arg_insert.namesCount++;
                    //printf("\tQUOTED_Name %d[%.3s...]\n", qData->arg_insert.namesCount, $1.text);
                }
        } 
        ;

insert_quoted_value:
        quoted_custom_value
        {
                setQData;
                if(qData->arg_insert.valuesCount >= MAX_EXPRESSIONS_VALUES)
                { 
                    qData->setError("Error in names list, too many values.", SQL_ERR_TOO_MANY_VALUES);  
                    qData->arg_insert.valuesCount=0;
                }
                else
                {
                    setTokValue(qData, qData->arg_insert.values[qData->arg_insert.valuesCount], $1);     
                    qData->arg_insert.valuesCount++;
                    //printf("\tQUOTED_Value %d[%.3s...%d]\n", qData->arg_insert.valuesCount, $1.text, $1.len);
                }
        }
        ;
        
delete_query:
        TOK_DELETE TOK_FROM table_name opt_for_where opt_for_order_by opt_for_limit
        {
                setQData;
                qData->command = TOK_DELETE;  
                //printf("\tУдаление\n");
        }
        ;
        
        
        
        
        
        
/**
 * Имя таблицы над которой производится операция
 */
table_name:
        VAL_NAME
        { 
                setQData;
                setTokValue(qData, qData->tableName, $1);   
                //printf("\tTable_Name [%.3s...%d]\n", $1.text, $1.len);
        }
        ;
        
        
        
quoted_custom_value:
        VAL_QUOTED_STRING
        |
        VAL_NAME
        |
        VAL_INT
        ;
        
%%
        
        