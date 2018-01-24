// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com


#ifndef COMETQL_H
#define	COMETQL_H

class QueryData;
class mysqlAnswer;

#include "TagLoger.h"
#include "MySqlProtocol.h"
#include <map>

#define SQL_ERR_UNDEFINED           1
#define SQL_ERR_NOT_IMPLEMENTED     2 // not yet implemented
#define SQL_ERR_SYNTAX_ERROR        3 // Syntax error

// @FixME добавить ссылки на документацию в сообщения об ошибках
#define SQL_ERR_READ_ONLY           10 // Table is read only
#define SQL_ERR_OVERFLOW            11 // Data is too long
#define SQL_ERR_INVALID_DATA        12
#define SQL_ERR_WHERE_EXPRESSIONS   13 // Selection without transferring the requested values of the primary key is not supported
#define SQL_ERR_COLUMNS_LIST        14 // Error in columns list
#define SQL_ERR_NOT_EXIST           15 // doesn't exist
#define SQL_ERR_AUTHENTICATION      16 // Authentication failure
#define SQL_ERR_EMPTY               17 // Data is too short
#define SQL_ERR_TOO_MANY_CONDITIONS 18 // too many conditions
#define SQL_ERR_TOO_MANY_VALUES     19 // too many values
#define SQL_ERR_TOO_MANY_COLUMNS    20 // too many columns
#define SQL_ERR_TOO_MANY_NAMES      21 // too many names
#define SQL_ERR_INTERNAL_SERVER     500 // internal server error
#define SQL_ERR_ACCESS_DENIED       403 // iACCESS DENIED

void* QLParsing(char* qury, QueryData* param);
void QLdeleteBuffer(void* buffer, QueryData *qInfo);

/**
 * @param buffer строка
 * @param len длина токена
 * @return число содержащиеся в строке
 */
int strToInt(const char* buffer, int len);

/**
 * Для хранения позиций начала и конце текстового токена
 */
class tokPlace
{
public:
    int tokStart = 0;
    int tokLen = 0;
    char quote = 0;

    int ToInt(QueryData &query);
    char* Start(QueryData &query);
    int Copy(QueryData &query, char* buff);

    /**
     * Сравнивает строку value с значением из токена
     * @param value строка
     * @param place токен
     * @return Вернёт true если строки совпадают
     */
    bool Compare(const char* value, QueryData &query);

    /**
     * @return true если токен не начто не указывает
     */
    bool isNull()
    {
        return tokStart == 0 && tokLen == 0;
    }

};

/**
 * Максимальная длина имени колонки
 */
#define MAX_COLUMN_NAME 250

/**
 * Максимальное количество колонок в таблице
 */
#define MAX_COLUMNS_COUNT 20

/**
 * Максимальный размер данных вмещаемых в одну ячейку таблицы
 */
#define MAX_COLUMN_SIZE 2000

/**
 * Максимальное количество строк в ответе
 * @todo Вынести констану в конфигурационный файл | а лучше и экономичнее выделять память по мере надобности
 */
//#define MAX_LINES_IN_ANSWER 2048

/**
 * Максимальное количество выражений после токена where
 */
#define MAX_EXPRESSIONS_COUNT 10

/**
 * Максимальное количество значений в одном выражении ( для токена in )
 */
#define MAX_EXPRESSIONS_VALUES 4000

class sqlExpression
{
public:
    
    // Таблица значений переменных key->value
    //std::map<std::string, MySqlResulValue>* vars;
    
    /**
     * Содержит количество выражений
     */
    int tokensCount = 0;
    
    tokPlace exprTokens[MAX_EXPRESSIONS_COUNT]; // Токены токенов по порядку в выражении
    int exprTokensTypes[MAX_EXPRESSIONS_COUNT]; // Типы токенов по порядку в выражении
    
    
    bool isMatch(){
        
    }
    
    MySqlResulValue getValue(){
        
    }
       
    sqlExpression operator = (sqlExpression exp){
        return exp;
    }
    
    sqlExpression* operator = (sqlExpression* exp){
        return exp;
    }
};

class sqlWhere
{
public:
    /**
     * Содержит количество выражений
     */
    int whereExprCount = 0;

    /**
     * временная переменная для хрананения количества значений в выражении
     */
    int whereExprValueCount = 0;

    /**
     * Содержит имена колонок для выражения
     */
    tokPlace whereExprColum[MAX_EXPRESSIONS_COUNT];

    /**
     * Содержит значения для выражения
     */
    tokPlace whereExprValue[MAX_EXPRESSIONS_COUNT][MAX_EXPRESSIONS_VALUES];

    /**
     * Содержит 0 или TOK_OR или TOK_AND
     */
    int whereExprOperator[MAX_EXPRESSIONS_COUNT];

    /**
     * Содержит TOK_LIKE или MORE или LESS или EQUALLY или TOK_IN
     */
    int whereExprCondition[MAX_EXPRESSIONS_COUNT];
};

class sqlLimit
{
public:
    unsigned int start = 0; // limit limit_start [, limit]
    unsigned int rows = 0;

    void setStart(int Start)
    {
        if(Start > 0)
        {
            start = Start;
        }
    }

    void setRows(int Rows)
    {
        if(Rows > 0)
        {
            rows = Rows;
        }
    }

    bool isAllowed(int row)
    {
        return (start == rows == 0) || (row > start && row < start + rows);
    }
};

class sqlOrderBy
{
public:

    /**
     * Токен по содержащий имя поля для сортировки
     */
    tokPlace name;

    /**
     * Токен содержащий тип сортировки TOK_ASC или TOK_DESC или 0 если сортировка не задана.
     */
    int type;
};

class Query_show
{
public:
    int command = 0;
    int flag;
    tokPlace like;
};

class Query_set
{
public:
    int command = 0;
    tokPlace section;
    tokPlace varible;
    tokPlace value;
};

class Query_select
{
public:
    int command = 0;

    /**
     * Для выборки системых переменных
     */
    tokPlace systemVaribleName;

    /**
     * Содержит количество выбираемых колонок
     */
    int selectedColumnsCount = 0;
    /**
     * Содержит список выбираемых колонок
     */
    tokPlace selectedColumns[MAX_COLUMNS_COUNT];
    
    /**
     * Содержит количество выбираемых колонок выражений
     */
    int selectedExpressionsCount = 0;
    sqlExpression selectedExpressions[MAX_COLUMNS_COUNT];
};

class Query_insert
{
public:
    int command = 0;

    /**
     * Количество колонок перечисленых при вставке
     */
    int namesCount = 0;
    /**
     * Количество значений перечисленых при вставке
     * @note значение valuesCount должно совпадать с namesCount если namesCount != 0
     */
    int valuesCount = 0;

    tokPlace names[MAX_COLUMNS_COUNT];
    tokPlace values[MAX_COLUMNS_COUNT];
};

/**
 * Класс для хранания распарсеной информации о запросе
 */
class QueryData
{
public:
    char* StartQury = NULL;
    
    sqlExpression* currentExpression;

    /**
     * Для сохранения текста ошибки
     */
    char errorText[250];

    /**
     * Для сохранения кода ошибки
     */
    int errorCode = 0;

    int command = 0;
    Query_show arg_show;
    Query_select arg_select;
    Query_insert arg_insert;
    Query_set arg_set;

    tokPlace tableName;
    sqlWhere where;
    sqlLimit limit;
    sqlOrderBy orderBy;

    int hasError = 0;

    QueryData():arg_show(),arg_select(), arg_insert(), tableName(), where(), limit(), orderBy()
    {
        bzero(errorText, 250);
    }

    /**
     * Очищает объект приводя его в начальное состояние
     */
    void clear()
    {
        bzero(this, sizeof(QueryData));
    }

    const char* getQuery() const
    {
        return (const char*)StartQury;
    }

    /**
     *
     * @param place
     * @return указатель на начало токена в запросе
     */
    char* tokStart(tokPlace place)
    {
        return (char*)StartQury + place.tokStart;
    }

    /**
     * Копирует токен в память buff
     * @param place
     * @param buff
     * @return Длина токена
     */
    int tokCopy(tokPlace place, char* buff)
    {
        memcpy(buff, tokStart(place), place.tokLen);
        return place.tokLen;
    }

    /**
     * Преобразование значения токена к целому числу
     * @param place
     * @return
     */
    int tokToInt(tokPlace place)
    {
        return strToInt(StartQury + place.tokStart, place.tokLen);
    }

    /**
     * Сравнивает строку value с значением из токена
     * @param value строка
     * @param place токен
     * @return Вернёт true если строки совпадают
     */
    bool tokCompare(const char* value, tokPlace place)
    {
        if(strlen(value) != place.tokLen)
        {
            return false;
        }
        return memcmp(value, StartQury + place.tokStart, place.tokLen) == 0;
    }

    void setError(const char* ErrorText, int ErrorCode)
    {
        int len = strlen(ErrorText);
        if(len > 250)
        {
            len = 250;
        }
        memcpy(errorText, ErrorText, len);
        errorCode = ErrorCode;
    }
};

/**
 * Класс для формирования заголовков описание таблицы и ответа в mysql
 */
class mysqlAnswer
{
    /**
     * Масив с структурой для значений ячеек для MySqL протокола
     */
    //MySqlResulValue values[MAX_LINES_IN_ANSWER][MAX_COLUMNS_COUNT];
    std::map<int, MySqlResulValue*> mapValues;

public:

    mysqlAnswer()
    {
        bzero(columPositions, MAX_COLUMNS_COUNT);
        bzero(expressionsPositions, MAX_EXPRESSIONS_COUNT);
    }

    /**
     * Содержит информацию о том в каком порядке идут колонке в ответе на запрос.
     * Номер элемента соответсвует номеру колонки при естественном следовании колонок
     * А значение элемента соответсвует номеру колонки при определении в запросе
     * Если значение -1 то колонка в ответе не отображается.
     */
    char columPositions[MAX_COLUMNS_COUNT];


    /**
     * Содержит информацию о том в каком порядке идут колонке в секции where
     * Номер элемента соответсвует номеру колонки при естественном следовании колонок в таблице
     * А значение элемента соответсвует номеру условия в массиве условий
     * Если значение -1 то колонка условиях не упоминается.
     * @Note если колонка в условии упоминается дважды то не хорошо. такое пока не поддерживается.
     */
    char expressionsPositions[MAX_EXPRESSIONS_COUNT];

    /**
     * Масив со структурой для колонок таблиц для MySqL протокола
     */
    MySqlResultset_ColumDef columns[MAX_COLUMNS_COUNT];

    /**
     * @param column Порядковый номер колонки в таблице (в соответсвии с определением в таблице)
     * @return Вернёт true если колонка используется в ответе.
     */
    inline bool useColumn(int column)
    {
        if(column >= MAX_COLUMNS_COUNT)
        {
            TagLoger::trace(Log_MySqlServer, 0, "\x1b[1;31museColumn column=%d more than MAX_COLUMNS_COUNT=%d\x1b[0m", column, MAX_COLUMNS_COUNT);
            return false;
        }

        return columPositions[column] != -1;
    }
 
    /** 
     * @param line Номер строки 
     * @return Вернёт ссылку на память для данных строки в таблице
     */
    MySqlResulValue* getMapRow(int line)
    { 
        auto it = mapValues.find(line);
        if( it != mapValues.end())
        {
            MySqlResulValue* row = it->second;
            return row;
        }

        MySqlResulValue* row = new MySqlResulValue[MAX_COLUMNS_COUNT];
        mapValues.insert(std::pair<int,MySqlResulValue*>(line,row));

        return row;
    }
    
    /**
     *
     * @param line Номер строки
     * @param column Номер колонки
     * @return Вернёт ссылку на память для данных ячейки
     */
    MySqlResulValue& getValue(int line, int column)
    {
        if(column >= MAX_COLUMNS_COUNT)
        {
            column = MAX_COLUMNS_COUNT - 1;
            TagLoger::trace(Log_MySqlServer, 0, "\x1b[1;31mNot enough memory for the field column=%d, MAX_COLUMNS_COUNT=%d\x1b[0m", column, MAX_COLUMNS_COUNT);
        }
 
        return getMapRow(line)[columPositions[column]];
    }

    /**
     *
     * @param line Номер строки
     * @param column Номер колонки
     * @return Вернёт ссылку на память для данных ячейки
     
    MySqlResulValue& getValue(int line, int column)
    {
        if(line >= MAX_LINES_IN_ANSWER)
        {
            line = MAX_LINES_IN_ANSWER - 1;
            TagLoger::trace(Log_MySqlServer, 0, "\x1b[1;31mNot enough memory for the line row=%d, MAX_LINES_IN_ANSWER=%d\x1b[0m", line, MAX_LINES_IN_ANSWER);
        }

        if(column >= MAX_COLUMNS_COUNT)
        {
            column = MAX_COLUMNS_COUNT - 1;
            TagLoger::trace(Log_MySqlServer, 0, "\x1b[1;31mNot enough memory for the field column=%d, MAX_COLUMNS_COUNT=%d\x1b[0m", column, MAX_COLUMNS_COUNT);
        }

        return values[line][columPositions[column]];
    }*/

    /**
     * Подсчитывает количество колонок в ответе на запрос.
     * @return
     */
    int countColumns()
    {
        int count = 0;
        for(int i=0; i<MAX_COLUMNS_COUNT; i++)
        {
            if(columPositions[i] >= 0)
            {
                count++;
            }
        }

        return count;
    }

    /**
     * @param PacketNomber номер пакета
     * @param buff буфер в который складываются значения
     * @param count количество строк которые надо вывести
     * @param qInfo
     * @param buffSize Объём буфера.
     * @param realSendPackage количество реально добавленых в буфер mysql пакетов
     * @return количество занятых байт в буфере
     *
     * @note В ответе не будет более чем MAX_LINES_IN_ANSWER строк и сумарный их вес не будет более чем buffSize
     * @note Если в параметр count передать больше строк чем было запронено в qInfo то оставшиеся будут содержать непонятно что а не нули.
    
    int rowsToBuff(unsigned int PacketNomber, char* buff, int count, QueryData &qInfo, int buffSize, int &realSendPackage)
    {
        int numColumns = countColumns();
        char* answer = buff;

        realSendPackage = 0;
        for(int i=qInfo.limit.start; (i < MAX_LINES_IN_ANSWER && i < count && (i< qInfo.limit.rows || qInfo.limit.rows == 0 )); i++)
        {
            if(numColumns * MAX_COLUMN_SIZE >=  buffSize - (answer - buff))
            {
                // Не хватило памяти в буфере вернём то что поместилось.
                printf("There was not enough memory in the buffer to return what was fit. row=%d, buffSize=%d, free=%ld\n", i, buffSize, buffSize - (answer - buff));
                return answer - buff;
            }

            answer += RowPackage(numColumns, values[i], ++PacketNomber, answer);
            realSendPackage++;
        }

        return answer - buff;
    } */

    /**
     * @param PacketNomber номер пакета
     * @param buff буфер в который складываются значения
     * @param count количество строк которые надо вывести
     * @param qInfo
     * @param buffSize Объём буфера.
     * @param realSendPackage количество реально добавленых в буфер mysql пакетов
     * @return количество занятых байт в буфере
     *
     * @note В ответе не будет более чем MAX_LINES_IN_ANSWER строк и сумарный их вес не будет более чем buffSize
     * @note Если в параметр count передать больше строк чем было запронено в qInfo то оставшиеся будут содержать непонятно что а не нули.
     */
    int rowsToBuff(unsigned int PacketNomber, char* buff, int count, QueryData &qInfo, int buffSize, int &realSendPackage)
    {
        int numColumns = countColumns();
        char* answer = buff;

        realSendPackage = 0;
        for(int i=qInfo.limit.start; (i < count && (i< qInfo.limit.rows || qInfo.limit.rows == 0 )); i++)
        {
            if(numColumns * MAX_COLUMN_SIZE >=  buffSize - (answer - buff))
            {
                // Не хватило памяти в буфере вернём то что поместилось.
                printf("There was not enough memory in the buffer to return what was fit. row=%d, buffSize=%d, free=%ld\n", i, buffSize, buffSize - (answer - buff));
                return answer - buff;
            }

            answer += RowPackage(numColumns, getMapRow(i), ++PacketNomber, answer);
            realSendPackage++;
        }

        return answer - buff;
    }
    /**
     * Отправляет ответ для sql select операции.
     * Отправляет пакеты с описанием колонок, пакеты со строками. И пакет EOF
     * @param local_buf
     * @param PacketNomber
     * @param countRows количество строк которое надо отправить
     * @return
     * @note Если в параметр countRows передать больше строк чем было запронено в  local_buf->sql то оставшиеся будут содержать непонятно что а не нули.
     */
    int sendAllRowsAndHeaders(thread_data* local_buf, unsigned int PacketNomber, int countRows, connection* obj);

    /**
     * Заполняет массив columPositions в соответсвии с теми колонками которые есть в таблице, но так чтоб они в ответе шли в том же порядке в каком были запрошены а не в том каком объявлены в таблице
     * Так же проверяет то чтоб колонки были доступны
     * @param columDef Существующие колонки
     * @param qInfo Распарсеный запрос
     * @return
     * Если ошибка найдена то будет задан текст ошибки и её код в qInfo.setError
     */
    bool prepare_columns_for_select(const char** columDef, QueryData &qInfo);

    /**
     * Проверяет валидность и выполнимость условий выборки
     * @param columDef
     * @param qInfo
     * @return
     * Если ошибка найдена то будет задан текст ошибки и её код в qInfo.setError
     */
    bool prepare_columns_for_insert(const char** columDef, QueryData &qInfo);

    /**
     * Заполняет массив columPositions в соответсвии с теми колонками которые переданы в запросе вставки, но так чтоб они шли в том же порядке в каком были объявлены в таблице, а не в том каком переданы
     * Так же проверяет то чтоб колонки были доступны
     * @param columDef Существующие колонки
     * @param qInfo Распарсеный запрос
     * @return
     * Если ошибка найдена то будет задан текст ошибки и её код в qInfo.setError
     */
    bool prepare_where_expressions(const char** columDef, QueryData &qInfo);
};

#endif	/* COMETQL_H */