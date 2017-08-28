// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "MySqlProtocol.h"
 
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include <vector>
#include <map>

#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <cmath>


#include "TagLoger.h"
#include "connection.h"

int OK_Package(unsigned int PacketNomber, char* buff)
{
    memcpy(buff,OK_Package_Nomber2, sizeof(OK_Package_Nomber2));
    buff[3] = PacketNomber % 255;
    return sizeof(OK_Package_Nomber2);
}

int OK_Package(LengthEncodedInteger affectedRows, LengthEncodedInteger lastInsertId, unsigned int PacketNomber, char* buff)
{  
    return OK_Package(affectedRows, lastInsertId, 0, PacketNomber, buff);
}

int OK_Package(LengthEncodedInteger affectedRows, LengthEncodedInteger lastInsertId, const char* status,  unsigned int PacketNomber, char* buff)
{  
    // Отправляем пакет описания 1 колонки 
    char* answer = buff;

    // первые 3 байта — длина пакета
    answer+=3;

    // четвертый байт — номер пакета.
    *answer = PacketNomber % 255;
    answer+=1;
    
    *answer = 0x00;
    answer+=1;
    
    answer+= affectedRows.setToBuff(answer);
    answer+= lastInsertId.setToBuff(answer);
    
    *answer = 0x00; // Status Flags
    answer+=1;
    *answer = 0x02; // Status Flags
    answer+=1;

    *answer = 0x00; // number of warnings
    answer+=1;
    *answer = 0x00; // number of warnings
    answer+=1;
    
    if(status != 0)
    {
        int textLen = strlen(status);
        memcpy(answer, status, textLen); // human readable status information
        answer+=textLen;
    }
    else
    {
        *answer = 0x00;
        answer+=1;
    }
    
    int dataLen = answer - buff;
    int TmpDataLen = dataLen - 4;
    memcpy(buff,  &TmpDataLen, 3); //  Длина пакета
    
    //printHex(buff, dataLen);

    return dataLen; 
}

int EOF_Package(unsigned int PacketNomber, char* buff)
{
    memcpy(buff,EOF_Package_Nomber2, sizeof(EOF_Package_Nomber2));
    buff[3] = PacketNomber % 255;
    return sizeof(EOF_Package_Nomber2);
}
 
int Err_Package(short errorCode, const char* errorText, unsigned int PacketNomber, char* buff)
{
    // @todo Добавить возможность задать состояние https://dev.mysql.com/doc/refman/5.5/en/error-messages-server.html
    
    /**  
     * Error: 1021 SQLSTATE: HY000 (ER_DISK_FULL)
     * Error: 1024 SQLSTATE: HY000 (ER_ERROR_ON_READ)
     * Error: 1026 SQLSTATE: HY000 (ER_ERROR_ON_WRITE)
     */
    const unsigned char Err_Package[] = {
                        0x00, 0x00, 0x00, 0x02,
                        0xff,                   // [ff] header of the ERR packet
                        0x00, 0x01,             // error-code
                        0x23,                   //  marker of the SQL State
                        ' ', 'o', '_', 'O', ' ' // SQL State
    };

    memcpy(buff,Err_Package, sizeof(Err_Package));
    buff[3] = PacketNomber % 255;
    
    memcpy(buff + 5,&errorCode, 2); // error-code
    
    int textLen = strlen(errorText);
    memcpy(buff + sizeof(Err_Package), errorText, textLen); // human readable error message
     
    int TmpDataLen = sizeof(Err_Package) + textLen - 4;
    memcpy(buff,  &TmpDataLen, 3); //  Длина пакета
    
    
    memcpy(buff + 8,  "HY000", 5);
    /*int i=0;
    int count = 12;
    if( rand() % count == i++)         memcpy(buff + 8,  " *_* ", 5); //  Смаил прикола ради.    
    else if( rand() % count == i++)    memcpy(buff + 8,  " +_+ ", 5); //  Смаил прикола ради.  
    else if( rand() % count == i++)    memcpy(buff + 8,  " -_- ", 5); //  Смаил прикола ради.  
    else if( rand() % count == i++)    memcpy(buff + 8,  " @_@ ", 5); //  Смаил прикола ради.  
    else if( rand() % count == i++)    memcpy(buff + 8,  " T_T ", 5); //  Смаил прикола ради.  
    else if( rand() % count == i++)    memcpy(buff + 8,  " o_O ", 5); //  Смаил прикола ради.
    else if( rand() % count == i++)    memcpy(buff + 8,  " ^_^ ", 5); //  Смаил прикола ради.
    else if( rand() % count == i++)    memcpy(buff + 8,  " *-* ", 5); //  Смаил прикола ради.
    else if( rand() % count == i++)    memcpy(buff + 8,  " @-@ ", 5); //  Смаил прикола ради.
    else if( rand() % count == i++)    memcpy(buff + 8,  " ^_^ ", 5); //  Смаил прикола ради.    */
    return sizeof(Err_Package) + textLen;
}


int LengthEncodedInteger::getSize()
{
    if(abs(value) < 251) return 1;
    if(abs(value) < pow(2,16)) return 3;
    if(abs(value) < pow(2,24)) return 4;
    if(abs(value) < pow(2,64)) return 9;
     
    printf("error LengthEncodedInteger is %lld\n", value); 
    return 9;
}

int LengthEncodedInteger::setToBuff(char* buff)
{ 
    return setToBuff(value, buff); 
}

int LengthEncodedInteger::setToBuff(long long value, char* buff)
{ 
    memcpy(buff, &value, 1);
    if(abs(value) < 251)
    {
        return 1;
    }
    
    if(abs(value) < pow(2,16))
    {
        *buff = 0xfc;
        memcpy(buff+1, &value, 3);
        return 3;
    }
    
    if(abs(value) < pow(2,24))
    {
        *buff = 0xfd;
        memcpy(buff+1, &value, 4);
        return 5;
    }
    
    if(abs(value) < pow(2,64))
    {
        printf("warning LengthEncodedInteger is %lld (9)\n", value); 
        *buff = 0xfe;
        memcpy(buff+1, &value, 8); // @fixme уточнить можноли так чтоб 9 байт
        return 9;
    }
    
    printf("error LengthEncodedInteger is %lld (>9)\n", value); 
    return 1;
}

/**
 * Формирует в память *buff данные для отправки описания количества колонок в ответе MySqlResultset
 * (не требует что бы память в buff была очищена перед использованием.)
 * @param ColumNumber Количество колонок в MySqlResultset
 * @param PacketNomber номер пакета который должен будет указан в описании колонки.
 * @param buff
 * @return Длина пакета ( сдвиг от начала buff и до конца описания колонки )
 */
int CountColumPackage(unsigned char ColumNumber, unsigned int PacketNomber, char* buff)
{
    // Отправляем пакет с количеством колонок  
    unsigned char columCount[] = {   0x01, 0x00, 0x00// первые 3 байта — длина пакета
                                         , (const unsigned char)(PacketNomber % 255) // четвертый байт — номер пакета.
                                         , (const unsigned char)(ColumNumber) // Количество колонок
    };
    memcpy(buff, columCount, sizeof(columCount));
    return sizeof(columCount);
}

/**
 * Формирует в память *buff данные для отправки описания одной строки в ответе MySqlResultset
 * (не требует что бы память в buff была очищена перед использованием.)
 * @param ColumNumber Количество колонок в MySqlResultset
 * @param PacketNomber номер пакета который должен будет указан в описании колонки.
 * @param buff
 * @return Длина пакета ( сдвиг от начала buff и до конца описания колонки )
 */
int RowPackage(unsigned char ColumNumber, MySqlResulValue* values, unsigned int PacketNomber, char* buff)
{ 
    // Отправляем пакет строки

    // Отправляем пакет описания 1 колонки 
    char* answer = buff;

    // первые 3 байта — длина пакета
    answer+=3;

    // четвертый байт — номер пакета.
    *answer = PacketNomber % 255;
    answer+=1;

    for(int i =0; i< ColumNumber; i++)
    { 
        int delta = values[i].toPackage(answer);
        answer += delta;
    }

    int dataLen = answer - buff;
    int TmpDataLen = dataLen - 4;
    memcpy(buff,  &TmpDataLen, 3); //  Длина пакета

    return dataLen;
}


/**
 * Формирует в память *buff данные для отправки описания всех колонок и добавляет EOF_Package, после этого остаётся только отправить данные строк (функция RowPackage) и ещё один EOF_Package в конце строк.
 * (не требует что бы память в buff была очищена перед использованием.)
 * @param ColumNumber Количество колонок в MySqlResultset
 * @param PacketNomber номер пакета который должен будет указан в описании колонки.
 * @param buff
 * @return Длина пакета ( сдвиг от начала buff и до конца описания колонки )
 * 
 * @Note Увеличивает  PacketNomber по формуле PacketNomber = PacketNomber + 2 + ColumNumber
 */
int HeadAnswer(unsigned char ColumNumber, const MySqlResultset_ColumDef* columns, unsigned int &PacketNomber, char* buff)
{
    char* p = buff;
    int delta = CountColumPackage(ColumNumber, ++PacketNomber, p);
    p+= delta;

    for(int i =0; i< ColumNumber; i++)
    {
        delta = columns[i].toPackage(++PacketNomber, p);
        p += delta;
    }

    delta = EOF_Package(++PacketNomber, p);
    p += delta;
    return p - buff;
} 


MySqlResultset_ColumDef::MySqlResultset_ColumDef()
{
    bzero(column_name, MAX_COLUMN_NAME_LENGTH);
}

MySqlResultset_ColumDef::MySqlResultset_ColumDef(const char* text)
{ 
    bzero(column_name, MAX_COLUMN_NAME_LENGTH);
    int data_len = strlen(text);
    if(data_len > 250)
    {
        TagLoger::error(Log_MySqlServer, 0, "MySqlResultset_ColumDef not completed before working with lines longer than 250 characters\n");
        data_len = 250;
    }
    memcpy(column_name, text, data_len); 
}

MySqlResultset_ColumDef::MySqlResultset_ColumDef(const char* text, int data_len)
{  
    setName(text, data_len);
}

void MySqlResultset_ColumDef::setName(const char* text, int data_len)
{
    bzero(column_name, MAX_COLUMN_NAME_LENGTH);
    if(data_len > 250)
    {
        TagLoger::error(Log_MySqlServer, 0, "MySqlResultset_ColumDef not completed before working with lines longer than 250 characters\n");
        data_len = 250;
    }
    memcpy(column_name, text, data_len); 
}
    
int MySqlResultset_ColumDef::toPackage(unsigned int PacketNomber, char* buff) const
{
    // Отправляем пакет описания 1 колонки 
    char* answer = buff;

    // первые 3 байта — длина пакета
    answer+=3;

    // четвертый байт — номер пакета.
    *answer = PacketNomber % 255;
    answer+=1;

    // lenenc_str catalog (always "def")
    *answer = 0x03;
    answer++;
    memcpy(answer,"def", 3);
    answer+=3;

    // lenenc_str schema-name
    *answer = 0x00;
    answer++;

    // lenenc_str virtual table-name
    *answer = 0x00;
    answer++;

    // lenenc_str  physical table-name
    *answer = 0x00;
    answer++;

    // lenenc_str virtual column name
    int name_len = strlen(column_name);
    *answer = name_len;
    answer++;
    memcpy(answer,column_name, name_len);
    answer+=name_len;

    // lenenc_str physical  column name
    *answer = 0x00;
    answer++;

    // next_length (lenenc_int) -- length of the following fields (always 0x0c)
    *answer = 0x0c;
    answer++;

    // character_set (2) -- is the column character set and is defined in Protocol::CharacterSet.
    *answer = 0x21; // utf8_general_ci
    answer+=1;
    *answer = 0x00;
    answer+=1;

    // column_length (4) -- maximum length of the field
    *answer = 0x4d;
    answer+=1;
    *answer = 0x00;
    answer+=1;
    *answer = 0x00;
    answer+=1;
    *answer = 0x00;
    answer+=1;

    // column_type (1) -- type of the column as defined in Column Type
    *answer = column_type;
    answer+=1;

    // flags (2) -- flags
    *answer = 0x01; // Не понятные флаги из примера
    answer+=2;

    // decimals (1) -- max shown decimal digits
    // 0x00 for integers and static strings
    // 0x1f for dynamic strings, double, float
    // 0x00 to 0x51 for decimals
    *answer = decimals;
    answer+=1;

    answer+=2;// Не понятные байты из примера

    int dataLen = answer - buff;
    int TmpDataLen = dataLen - 4;
    memcpy(buff,  &TmpDataLen, 3); //  Длина пакета

    return dataLen;
}

void MySqlResultset_ColumDef::clear()
{
    bzero(column_name, MAX_COLUMN_NAME_LENGTH);
}

char* MySqlResultset_ColumDef::operator = (const char* text)
{
    int data_len = strlen(text);
    if(data_len > 250)
    {
        TagLoger::error(Log_MySqlServer, 0, "MySqlResultset_ColumDef not completed before working with lines longer than 250 characters\n");
        data_len = 250;
    }
    clear();
    memcpy(column_name, text, data_len); 
    return column_name;
}

MySqlResulValue::MySqlResulValue()
{
    longData = NULL;
    bzero(data, 255);
}
    
int MySqlResulValue::toPackage(char* buff)
{ 
    if(type == MYSQL_PROTOCOL_TYPE_VAR_STRING)
    {
        if(longData != NULL)
        { 
            int delta = LengthEncodedInteger::setToBuff(longDataLen, buff); 
            buff += delta;
            memcpy(buff,longData, longDataLen); 
            return longDataLen+delta;
        }
        
        int data_len = strlen(data);
        *buff = data_len;
        buff++;
        memcpy(buff,data, data_len); 
        return data_len+1;
    }

    return -1;
}

char* MySqlResulValue::clear()
{
    if(longData != NULL)
    {
        delete[] longData;
        longData = NULL;
        longDataLen = 0;
    }

    bzero(data, 255);
    return data;
}

char* MySqlResulValue::operator = (const char* text)
{ 
    if(text == NULL)
    {
        return setValue(text, 0);
    }
    
    return setValue(text, strlen(text));
}

char* MySqlResulValue::setValue(const char* text, int data_len)
{  
    clear();
    if(text == NULL || data_len == 0)
    {
        return NULL;
    }
    
    if(data_len > 250)
    { 
        longDataLen = data_len;
        longData = new char[data_len+1];
        memcpy(longData, text, data_len); 
        return longData;
    }
    
    memcpy(data, text, data_len); 
    return data;
}

char* MySqlResulValue::operator = (const int value)
{
    clear();
    snprintf(data, 250, "%d", value);
    return data;
}


char* MySqlResulValue::operator = (const long value)
{
    clear();
    snprintf(data, 250, "%ld", value);
    return data;
}

char* MySqlResulValue::operator = (const float value)
{
    clear();
    snprintf(data, 250, "%f", value);
    return data;
}
 
char* MySqlResulValue::operator = (const char value)
{
    clear();
    data[0] = value;
    return data;
}

char* MySqlResulValue::operator = (const bool value)
{
    clear();
    if(value)
    {  
        data[0] = '1';
    }
    else
    {
        data[0] = '0';
    }
    return data;
}
 
MySqlResulValue::operator char* ()
{
    if(longData != NULL)
    {
        return longData;
    }
    return data;
}

MySqlResulValue::operator const unsigned char*()
{
    if(longData != NULL)
    {
        return (unsigned char*)longData;
    }
    return (unsigned char*)data;
}

char& MySqlResulValue::operator[] (int key)
{
    if(longData != NULL)
    {
        if(longDataLen <= key)
        { 
            TagLoger::error(Log_MySqlServer, 0, "MySqlResulValue::operator[%d] more them longDataLen=%d\n", key, longDataLen);
            return longData[longDataLen-1]; 
        }
        return longData[key];
    }
    
    return data[key];
}


int Send_OK_Package(unsigned int PacketNomber, thread_data* local_buf, connection* obj)
{
    local_buf->answer_buf.lock();
    int dataLen = OK_Package(PacketNomber,local_buf->answer_buf.getData());
    return obj->web_write(local_buf->answer_buf.getAndUnlock(), dataLen);
}

int Send_OK_Package(LengthEncodedInteger affectedRows, LengthEncodedInteger lastInsertId, unsigned int PacketNomber, thread_data* local_buf, connection* obj)
{
    return Send_OK_Package(affectedRows, lastInsertId, 0, PacketNomber, local_buf, obj);
}

int Send_OK_Package(LengthEncodedInteger affectedRows, LengthEncodedInteger lastInsertId, const char* status,  unsigned int PacketNomber, thread_data* local_buf, connection* obj)
{
    local_buf->answer_buf.lock();
    int dataLen = OK_Package(affectedRows, lastInsertId, status, PacketNomber,local_buf->answer_buf.getData());
    return obj->web_write(local_buf->answer_buf.getAndUnlock(), dataLen);
}

int Send_EOF_Package(unsigned int PacketNomber, thread_data* local_buf, connection* obj)
{
    local_buf->answer_buf.lock();
    int dataLen = EOF_Package(PacketNomber,local_buf->answer_buf.getData());
    return obj->web_write(local_buf->answer_buf.getAndUnlock(), dataLen);
}

int Send_Err_Package(short errorCode, const char* errorText, unsigned int PacketNomber, thread_data* local_buf, connection* obj)
{
    local_buf->answer_buf.lock();
    int dataLen = Err_Package(errorCode, errorText, PacketNomber,local_buf->answer_buf.getData());
    return obj->web_write(local_buf->answer_buf.getAndUnlock(), dataLen);
}
