// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com


#ifndef MYSQL_PROTOCOL_H
#define	MYSQL_PROTOCOL_H

using namespace std;
   
#define MYSQL_PROTOCOL_COM_QUIT            0x01    // https://dev.mysql.com/doc/internals/en/com-quit.html
#define MYSQL_PROTOCOL_COM_INIT_DB         0x02    // https://dev.mysql.com/doc/internals/en/com-init-db.html
#define MYSQL_PROTOCOL_COM_QUERY           0x03    // https://dev.mysql.com/doc/internals/en/com-query.html
#define MYSQL_PROTOCOL_COM_FIELD_LIST      0x04    // https://dev.mysql.com/doc/internals/en/com-field-list.html
#define MYSQL_PROTOCOL_COM_CREATE_DB       0x05    // https://dev.mysql.com/doc/internals/en/com-create-db.html
#define MYSQL_PROTOCOL_COM_DROP_DB         0x06    // https://dev.mysql.com/doc/internals/en/com-drop-db.html
#define MYSQL_PROTOCOL_COM_PING            0x0e    // https://dev.mysql.com/doc/internals/en/com-ping.html
#define MYSQL_PROTOCOL_COM_DELAYED_INSERT  0x10    // https://dev.mysql.com/doc/internals/en/com-delayed-insert.html
 

// https://dev.mysql.com/doc/internals/en/com-query-response.html#column-definition
#define MYSQL_PROTOCOL_TYPE_DECIMAL      0x00
#define MYSQL_PROTOCOL_TYPE_TINY         0x01
#define MYSQL_PROTOCOL_TYPE_SHORT        0x02
#define MYSQL_PROTOCOL_TYPE_LONG         0x03
#define MYSQL_PROTOCOL_TYPE_FLOAT        0x04
#define MYSQL_PROTOCOL_TYPE_DOUBLE       0x05
#define MYSQL_PROTOCOL_TYPE_NULL         0x06
#define MYSQL_PROTOCOL_TYPE_TIMESTAMP    0x07
#define MYSQL_PROTOCOL_TYPE_VAR_STRING   0xfd
#define MYSQL_PROTOCOL_TYPE_STRING       0xfe 

  
#define MYSQL_CLIENT_LONG_PASSWORD                    0x00000001
#define MYSQL_CLIENT_FOUND_ROWS                       0x00000002
#define MYSQL_CLIENT_LONG_FLAG                        0x00000004
#define MYSQL_CLIENT_CONNECT_WITH_DB                  0x00000008
#define MYSQL_CLIENT_NO_SCHEMA                        0x00000010
#define MYSQL_CLIENT_COMPRESS                         0x00000020
#define MYSQL_CLIENT_ODBC                             0x00000040
#define MYSQL_CLIENT_LOCAL_FILES                      0x00000080
#define MYSQL_CLIENT_IGNORE_SPACE                     0x00000100
#define MYSQL_CLIENT_PROTOCOL_41                      0x00000200
#define MYSQL_CLIENT_INTERACTIVE                      0x00000400
#define MYSQL_CLIENT_SSL                              0x00000800
#define MYSQL_CLIENT_IGNORE_SIGPIPE                   0x00001000
#define MYSQL_CLIENT_TRANSACTIONS                     0x00002000
#define MYSQL_CLIENT_RESERVED                         0x00004000
#define MYSQL_CLIENT_SECURE_CONNECTION                0x00008000
#define MYSQL_CLIENT_MULTI_STATEMENTS                 0x00010000
#define MYSQL_CLIENT_MULTI_RESULTS                    0x00020000
#define MYSQL_CLIENT_PS_MULTI_RESULTS                 0x00040000
#define MYSQL_CLIENT_PLUGIN_AUTH                      0x00080000  

#define MYSQL_CLIENT_CONNECT_ATTRS                    0x00100000
#define MYSQL_CLIENT_PLUGIN_AUTH_LENENC_CLIENT_DATA   0x00200000
#define MYSQL_CLIENT_CAN_HANDLE_EXPIRED_PASSWORDS     0x00400000
#define MYSQL_CLIENT_SESSION_TRACK                    0x00800000
#define MYSQL_CLIENT_DEPRECATE_EOF                    0x01000000 


/** decimals (1) -- max shown decimal digits
 * 0x00 for integers and static strings
 * 0x1f for dynamic strings, double, float
 * 0x00 to 0x51 for decimals 
 */
#define MYSQL_decimals_static    0x00 
/** decimals (1) -- max shown decimal digits
 * 0x00 for integers and static strings
 * 0x1f for dynamic strings, double, float
 * 0x00 to 0x51 for decimals 
 */
#define MYSQL_decimals_dynamic   0x1f  

const unsigned char OK_Package_Nomber2[] =  {  0x07, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00};
const unsigned char EOF_Package_Nomber2[] = {  0x05, 0x00, 0x00, 0x02, 0xfe, 0x00, 0x00, 0x02, 0x00};



const unsigned char Initial_Handshake_Package[] = {   0x36, 0x00, 0x00, 0x00, 0x0a, 0x35, 0x2e, 0x35
                                                    , 0x2e, 0x32, 0x2d, 0x6d, 0x32, 0x00, 0x03, 0x00
                                                    , 0x00, 0x00, 0x27, 0x75, 0x3e, 0x6f, 0x38, 0x66
                                                    , 0x79, 0x4e, 0x00, 0xff, 0xf7, 0x21, 0x02, 0x00
                                                    , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
                                                    , 0x00, 0x00, 0x00, 0x00, 0x00, 0x57, 0x4d, 0x5d
                                                    , 0x6a, 0x7c, 0x53, 0x68, 0x32, 0x5c, 0x59, 0x2e
                                                    , 0x73, 0x00};



class LengthEncodedInteger
{
public:
    long long value = 0;
    
    LengthEncodedInteger(){ }    
    LengthEncodedInteger(long long val):value(val){ }
    
    /**
     * @return количество байт занимаемое значением
     */
    int getSize();
    
    /**
     * Установит в buff значение типа LengthEncodedInteger
     * @param buff
     * @return количество байт занимаемое значением
     */
    int setToBuff(char* buff);
    static int setToBuff(long long value, char* buff);
};


int OK_Package(unsigned int PacketNomber, char* buff);
int OK_Package(LengthEncodedInteger affectedRows, LengthEncodedInteger lastInsertId, unsigned int PacketNomber, char* buff);

/**
 * @param affectedRows affected rows
 * @param lastInsertId last insert-id
 * @param status human readable status information
 * @param PacketNomber
 * @param buff
 * @return 
 */
int OK_Package(LengthEncodedInteger affectedRows, LengthEncodedInteger lastInsertId, const char* status,  unsigned int PacketNomber, char* buff);

int EOF_Package(unsigned int PacketNomber, char* buff);

/**
 * @todo Добавить возможность задать состояние https://dev.mysql.com/doc/refman/5.5/en/error-messages-server.html
 * @param errorCode
 * @param errorText
 * @param PacketNomber
 * @param buff
 * @return 
 */
int Err_Package(short errorCode, const char* errorText, unsigned int PacketNomber, char* buff);
 
#define MAX_COLUMN_NAME_LENGTH 255
class MySqlResultset_ColumDef
{ 
public:
    char column_name[MAX_COLUMN_NAME_LENGTH];
    char column_type = MYSQL_PROTOCOL_TYPE_VAR_STRING;
    char decimals = MYSQL_decimals_dynamic;
    
    MySqlResultset_ColumDef();
    MySqlResultset_ColumDef(const char* text);
    MySqlResultset_ColumDef(const char* text, int data_len);
     
    void setName(const char* text, int data_len);
     
    /**
     * Формирует в память *buff данные для отправки описания одной колонки в ответе MySqlResultset
     * (не требует что бы память в buff была очищена перед использованием.)
     * @param PacketNomber номер пакета который должен будет указан в описании колонки.
     * @param buff
     * @return Длина пакета ( сдвиг от начала buff и до конца описания колонки )
     */
    int toPackage(unsigned int PacketNomber, char* buff) const;
    
    void clear(); 
    
    operator char* ()
    {
        return column_name;
    }
    
    operator const unsigned char*()
    {
        return (unsigned char*)column_name;
    }
    
    char& operator[] (int key)
    {
        return column_name[key];
    }
    
    char* operator = (const char* text);
};

class MySqlResulValue
{     
    char data[255]; 
    
    /**
     * Для хранения строк длинее 250 символов 
     */
    char* longData = 0;
    int longDataLen = 0;
public:
    
    MySqlResulValue();
    
    unsigned char type = MYSQL_PROTOCOL_TYPE_VAR_STRING;
     
    /**
     * @FixMe Работает только на строках короче 255 символов!
     * @param buff
     * @return 
     */
    int toPackage(char* buff); 
    char* clear(); 
    
    operator char* ();    
    operator const unsigned char*();    
    char& operator[] (int key);
    
    char* operator = (const char* text);
    char* operator = (const int value);
    char* operator = (const long value);
    char* operator = (const float value);
    char* operator = (const char text);
    char* operator = (const bool value);
    
    char* setValue(const char* text, int data_len);
};
   
/**
 * Формирует в память *buff данные для отправки описания количества колонок в ответе MySqlResultset
 * (не требует что бы память в buff была очищена перед использованием.)
 * @param ColumNumber Количество колонок в MySqlResultset
 * @param PacketNomber номер пакета который должен будет указан в описании колонки.
 * @param buff
 * @return Длина пакета ( сдвиг от начала buff и до конца описания колонки )
 */
int CountColumPackage(unsigned char ColumNumber, unsigned int PacketNomber, char* buff);

/**
 * Формирует в память *buff данные для отправки описания одной строки в ответе MySqlResultset
 * (не требует что бы память в buff была очищена перед использованием.)
 * @param ColumNumber Количество колонок в MySqlResultset
 * @param PacketNomber номер пакета который должен будет указан в описании колонки.
 * @param buff
 * @return Длина пакета ( сдвиг от начала buff и до конца описания колонки )
 */
int RowPackage(unsigned char ColumNumber, MySqlResulValue* values, unsigned int PacketNomber, char* buff);


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
int HeadAnswer(unsigned char ColumNumber, const MySqlResultset_ColumDef* columns, unsigned int &PacketNomber, char* buff);

class connection;
class thread_data;

int Send_OK_Package(unsigned int PacketNomber, thread_data* local_buf, connection* obj);
int Send_OK_Package(LengthEncodedInteger affectedRows, LengthEncodedInteger lastInsertId, unsigned int PacketNomber, thread_data* local_buf, connection* obj);
int Send_OK_Package(LengthEncodedInteger affectedRows, LengthEncodedInteger lastInsertId, const char* status,  unsigned int PacketNomber, thread_data* local_buf, connection* obj);
int Send_EOF_Package(unsigned int PacketNomber, thread_data* local_buf, connection* obj);
int Send_Err_Package(short errorCode, const char* errorText, unsigned int PacketNomber, thread_data* local_buf, connection* obj);

#endif	/* MYSQL_PROTOCOL_H */