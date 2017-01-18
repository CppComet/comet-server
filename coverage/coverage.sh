#!/bin/bash
 

mkdir cov
cd cov

# Определение coverage
# find ./ -type f -iname '*.gcno' | grep "main" | sed -e 's/main.cpp/*/g'
files=`find ../ -type f -iname '*.gcno' | grep "main" | sed -e 's/main.cpp/*/g'`

print "nnn - $files"

lcov --directory .. --capture --output-file cov.tmp1
gcov -b $files



# Генерация отчёта
mkdir cov

nstart1=`cat cov.tmp1 | grep -n "\.cc" | grep -E -o "^[0-9]+"`
print "nnn - $nstart1"
cat cov.tmp1 | sed -e "$nstart1,/end_of_record/d" >  cov.tmp2

nstart2=`cat cov.tmp2 | grep -n "\.y" | grep -E -o "^[0-9]+"` 
print "nnn - $nstart2"
cat cov.tmp2 | sed -e "$nstart2,/end_of_record/d" >  cov.info

genhtml -o ./cov cov.info






