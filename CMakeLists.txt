project(ccommet)
cmake_minimum_required(VERSION 2.6)

message(STATUS "System: " ${CMAKE_SYSTEM_NAME} " " ${CMAKE_SYSTEM_VERSION})
message(STATUS "Processor: " ${CMAKE_HOST_SYSTEM_PROCESSOR})

SET(CMAKE_CXX_FLAGS "-std=c++11 -g3")

#add_definitions(-DMY_PROJECT)
add_definitions(-DCMAKE_BUILD_TYPE=debug)

# Поиск библиотеки pthreads
find_package(Threads REQUIRED)
if(NOT Threads_FOUND)
    message(SEND_ERROR "Не найден Threads.")
    return()
else()
    include_directories(${Threads_INCLUDE_DIRS})
endif()
  
###########################################
#
# Необходимо уставновить пакет libmysqlclient-dev для работы с mysql
# sudo apt-get install libmysqlclient-dev
#
###########################################


###########################################
#
# - Find mysqlclient
# Find the native MySQL includes and library
#
#  MYSQL_INCLUDE_DIR - where to find mysql.h, etc.
#  MYSQL_LIBRARIES   - List of libraries when using MySQL.
#  MYSQL_FOUND       - True if MySQL found.

IF (MYSQL_INCLUDE_DIR)
  # Already in cache, be silent
  SET(MYSQL_FIND_QUIETLY TRUE)
ENDIF (MYSQL_INCLUDE_DIR)

FIND_PATH(MYSQL_INCLUDE_DIR mysql.h
  /usr/local/include/mysql
  /usr/include/mysql
)

SET(MYSQL_NAMES mysqlclient mysqlclient_r)
FIND_LIBRARY(MYSQL_LIBRARY
  NAMES ${MYSQL_NAMES}
  PATHS /usr/lib /usr/local/lib
  PATH_SUFFIXES mysql
)

IF (MYSQL_INCLUDE_DIR AND MYSQL_LIBRARY)
  SET(MYSQL_FOUND TRUE)
  SET( MYSQL_LIBRARIES ${MYSQL_LIBRARY} )
ELSE (MYSQL_INCLUDE_DIR AND MYSQL_LIBRARY)
  SET(MYSQL_FOUND FALSE)
  SET( MYSQL_LIBRARIES )
ENDIF (MYSQL_INCLUDE_DIR AND MYSQL_LIBRARY)

IF (MYSQL_FOUND)
  IF (NOT MYSQL_FIND_QUIETLY)
    MESSAGE(STATUS "Found MySQL: ${MYSQL_LIBRARY}")
  ENDIF (NOT MYSQL_FIND_QUIETLY)
ELSE (MYSQL_FOUND)
  IF (MYSQL_FIND_REQUIRED)
    MESSAGE(STATUS "Looked for MySQL libraries named ${MYSQL_NAMES}.")
    MESSAGE(FATAL_ERROR "Could NOT find MySQL library")
  ENDIF (MYSQL_FIND_REQUIRED)
ENDIF (MYSQL_FOUND)

MARK_AS_ADVANCED(
  MYSQL_LIBRARY
  MYSQL_INCLUDE_DIR
  )
###########################################
include_directories(${MYSQL_INCLUDE_DIR})


# Расположение заголовочных файлов
#include_directories(foo)			

execute_process( COMMAND flex -i src/CometQL.l)
execute_process( COMMAND bison -d src/CometQL.y)
execute_process( COMMAND yacc -d src/CometQL.y)
execute_process( COMMAND bison -d --verbose --debug src/CometQL.y -o src/CometQL.cc)
execute_process( COMMAND yacc -d --verbose --debug src/CometQL.y -o src/CometQL.cc)
execute_process( COMMAND cc -c src/lex.yy.c -o src/lex.yy.o)
 

# Установка переменной со списком исходников для исполняемого файла
set(SOURCE_EXE src/hashlib2plus/src/hl_md5.cpp src/hashlib2plus/src/hl_md5wrapper.cpp src/TagLoger.cpp src/TagTimer.cpp src/main.cpp src/appConf.cpp src/intervalLoop.cpp src/base64.cpp  src/sha1.cpp src/CometQL.cpp src/devManager.cpp src/internalApi.cpp src/Client_connection.cpp src/MySqlProtocol.cpp src/connection.cpp src/mystring.cpp src/MySql_connection.cpp src/tcpServer.cpp src/user_index.cpp src/backtrace.cpp src/thread_data.cpp src/thread_data.h src/lex.yy.o src/CometQL.cc) 

# Создает исполняемый файл с именем cpp_comet из исходников
add_executable(cpp_comet ${SOURCE_EXE} )

# Подключение библиотеки pthreads
target_link_libraries (cpp_comet ${CMAKE_THREAD_LIBS_INIT}  ${MYSQL_LIBRARIES} crypt uuid )


#cmake ./ && make


