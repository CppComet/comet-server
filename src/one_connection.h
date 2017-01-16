// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/* 
 * File:   one_connection.h
 * Author: victor
 *
 * Created on 2 Ноябрь 2012 г., 16:36
 */

#ifndef ONE_CONNECTION_H
#define	ONE_CONNECTION_H

template< class connectionType >
class one_connection
{
    public:
    int id;
    connectionType* obj;
    
    
    virtual ~one_connection()
    {
        close(id);
    }
};

#endif	/* ONE_CONNECTION_H */

