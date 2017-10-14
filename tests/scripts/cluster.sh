#!/bin/bash

 # Запуск с coverage
cd ../../coverage
cmake .
make

rm -rf ./cpp_cometN1.log # удалить старый лог

# Запуск под valgrind
print "start N1"
valgrind --tool=memcheck --track-origins=yes --leak-check=yes ./cpp_comet_coverage --conf ../tests/scripts/cometN1.ini #> ./cpp_cometN1.log

# grep лога на предмет вывода ошибок от valgrind
echo `cat ./cpp_cometN1.log | grep -E "^==="`
  
# Определение coverage
sh ./coverage.sh