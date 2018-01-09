// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com


#include <cstring>
#include <cstdio>
#include "CometQL.h"
#include "y.tab.h"


#include "TagLoger.h"
#include "connection.h"

int tokPlace::ToInt(QueryData &query){

        return strToInt(query.StartQury + tokStart, tokLen);
}

char* tokPlace::Start(QueryData &query)
{
    return query.StartQury + tokStart;
}

int tokPlace::Copy(QueryData &query, char* buff)
{
    memcpy(buff, Start(query), tokLen);
    return tokLen;
}

/**
 * Сравнивает строку value с значением из токена
 * @param value строка
 * @param place токен
 * @return Вернёт true если строки совпадают
 */
bool tokPlace::Compare(const char* value, QueryData &query)
{
    if(strlen(value) != tokLen)
    {
        return false;
    }
    return memcmp(value, query.StartQury + tokStart, tokLen) == 0;
}

/**
 * Заполняет массив columPositions в соответсвии с теми колонками которые есть в таблице, но так чтоб они в ответе шли в том же порядке в каком были запрошены а не в том каком объявлены в таблице
 * Так же проверяет то чтоб колонки были доступны
 * @param columDef Существующие колонки
 * @param qInfo Распарсеный запрос
 * @return
 * Если ошибка найдена то будет задан текст ошибки и её код в qInfo.setError
 */
bool mysqlAnswer::prepare_columns_for_select(const char** columDef, QueryData &qInfo)
{
    memset(columPositions, -1, MAX_COLUMNS_COUNT);
    if(qInfo.arg_select.selectedColumnsCount > 0)
    {
        for(int i=0; i<qInfo.arg_select.selectedColumnsCount; i++)
        {
            columns[i].setName(qInfo.arg_select.selectedColumns[i].Start(qInfo), qInfo.arg_select.selectedColumns[i].tokLen);
            bool isFound = false;
            for(int j=0; j < MAX_COLUMNS_COUNT; j++)
            {
                if(columDef[j] == NULL)
                {
                    break;
                }
                
                if(qInfo.tokCompare(columDef[j], qInfo.arg_select.selectedColumns[i]))
                {
                    if(columPositions[j] != -1)
                    {
                        qInfo.setError("Each column in the select used only once", SQL_ERR_WHERE_EXPRESSIONS);
                        return false;
                    }

                    columPositions[j] = i;
                    isFound = true;
                    break;
                }
            }
            if(!isFound)
            {
                qInfo.setError("Not a specific column", SQL_ERR_COLUMNS_LIST);
                return false;
            }
        }
    }
    else
    {
        for(int i=0; i < MAX_COLUMNS_COUNT; i++)
        {
            if(columDef[i] == NULL)
            {
                break;
            }

            columns[i] = columDef[i];
            columPositions[i] = i;
            qInfo.arg_select.selectedColumnsCount = i + 1;
        }
    }
    return prepare_where_expressions(columDef, qInfo);
}

/**
 * Проверяет валидность и выполнимость условий выборки
 * @param columDef
 * @param qInfo
 * @return 
 * Если ошибка найдена то будет задан текст ошибки и её код в qInfo.setError
 */
bool mysqlAnswer::prepare_where_expressions(const char** columDef, QueryData &qInfo)
{
    memset(expressionsPositions, -1, MAX_EXPRESSIONS_COUNT);
    /**
     * Проверка допустимости выражений.
     * Все первичные целочисленные ключи могут быть найдены только по точному совпадению EQUALLY или TOK_IN
     * Операция TOK_OR пока не реализована
     * Операция TOK_AND пока не реализована
     * Операция ORDER BY пока не реализована
     */
    if(qInfo.orderBy.type != 0)
    {
        qInfo.setError("Token ORDER BY is not yet implemented", SQL_ERR_NOT_IMPLEMENTED);
        return false;
    }

    for(int i=0; i<qInfo.where.whereExprCount; i++)
    {
        bool isFound = false;
        for(int j=0; j < MAX_COLUMNS_COUNT; j++)
        {
            if(columDef[j] == NULL)
            {
                break;
            }

            if(qInfo.tokCompare(columDef[j], qInfo.where.whereExprColum[i]))
            {
                if(expressionsPositions[j] != -1)
                {
                    qInfo.setError("Each column in the conditions used only once", SQL_ERR_WHERE_EXPRESSIONS);
                    return false;
                }

                expressionsPositions[j] = i;
                isFound = true;
                break;
            }
        }

        if(!isFound)
        {
            qInfo.setError("Not a specific column", SQL_ERR_COLUMNS_LIST);
            return false;
        }

        if(qInfo.where.whereExprOperator[i] == TOK_OR)
        {
            qInfo.setError("Operator OR is not yet implemented", SQL_ERR_NOT_IMPLEMENTED);
            return false;
        }
        //else if(qInfo.where.whereExprOperator[i] == TOK_AND)
        //{
        //    qInfo.setError("Operator AND is not yet implemented", SQL_ERR_NOT_IMPLEMENTED);
        //    return false;
        //}
    }

    return true;
}
 
/**
 * Заполняет массив columPositions в соответсвии с теми колонками которые переданы в запросе вставки, но так чтоб они шли в том же порядке в каком были объявлены в таблице, а не в том каком переданы
 * Так же проверяет то чтоб колонки были доступны
 * @param columDef Существующие колонки
 * @param qInfo Распарсеный запрос
 * @return
 * Если ошибка найдена то будет задан текст ошибки и её код в qInfo.setError
 */
bool mysqlAnswer::prepare_columns_for_insert(const char** columDef, QueryData &qInfo)
{
    memset(columPositions, -1, MAX_COLUMNS_COUNT);

    if(qInfo.arg_insert.namesCount != qInfo.arg_insert.valuesCount && qInfo.arg_insert.namesCount != 0 )
    {
        qInfo.setError("Error in columns list", SQL_ERR_COLUMNS_LIST);
        return false;
    }

    if(qInfo.arg_insert.namesCount > 0)
    {
        for(int i=0; i<qInfo.arg_insert.namesCount; i++)
        {
            columns[i].setName(qInfo.arg_insert.names[i].Start(qInfo), qInfo.arg_insert.names[i].tokLen);
            bool isFound = false;
            for(int j=0; j < MAX_COLUMNS_COUNT; j++)
            {
                if(columDef[j] == NULL)
                {
                    break;
                }

                if(qInfo.tokCompare(columDef[j], qInfo.arg_insert.names[i]))
                {
                    if(columPositions[j] != -1)
                    {
                        qInfo.setError("Each column in the insert used only once", SQL_ERR_WHERE_EXPRESSIONS);
                        return false;
                    }

                    columPositions[j] = i;
                    isFound = true;
                    break;
                }
            }
            if(!isFound)
            {
                qInfo.setError("Not a specific column", SQL_ERR_COLUMNS_LIST);
                return false;
            }
        }
    }
    else
    {
        for(int i=0; i < MAX_COLUMNS_COUNT; i++)
        {
            if(columDef[i] == NULL)
            {
                break;
            }

            columns[i] = columDef[i];
            columPositions[i] = i;
            qInfo.arg_insert.namesCount = i + 1;
        }


        if(qInfo.arg_insert.namesCount != qInfo.arg_insert.valuesCount)
        {
            qInfo.setError("Error in columns list", SQL_ERR_COLUMNS_LIST);
            return false;
        }
    }

    return true;
}
 
/**
 * Отправляет ответ для sql select операции.
 * Отправляет пакеты с описанием колонок, пакеты со строками. И пакет EOF
 * @param local_buf
 * @param PacketNomber 
 * @param countRows количество строк которое надо отправить 
 * @return 
 */
int mysqlAnswer::sendAllRowsAndHeaders(thread_data* local_buf, unsigned int PacketNomber, int countRows, connection* obj)
{
    local_buf->answer_buf.lock();
    char* answer = local_buf->answer_buf.getData();
    answer += HeadAnswer(local_buf->qInfo.arg_select.selectedColumnsCount, local_buf->sql.columns, PacketNomber, answer);

    int realSendPackage = 0;
    int maxSize = local_buf->answer_buf.getSize() - (answer - local_buf->answer_buf.getData()); // Объём свободной памяти в буфере.
    answer += local_buf->sql.rowsToBuff(PacketNomber, answer, countRows, local_buf->qInfo, maxSize, realSendPackage);
    PacketNomber += realSendPackage;

    int dataSize = answer - local_buf->answer_buf.getData();
    obj->web_write(local_buf->answer_buf.getData(), dataSize);

    //TagLoger::log(Log_MySqlServer, 0, "dataSize=%d, realSendPackage=%d, countRows=%d", dataSize, realSendPackage, countRows);
 
    local_buf->answer_buf.unlock();
    return Send_EOF_Package(++PacketNomber, local_buf, obj);
}