#!/bin/bash

printf """\n \
setTimeout(function(){ \n \
""" > ./test-cov.js


printf """\n\n\n""" >> ./test-cov.js

# Эмуляция объекта window для запуска тестов js api в nodejs
cat ./window-object.js  >> ./test-cov.js
 

# Включение CometServerApi.js 
cat ../api/CometServerApi.js  >> ./test-cov.js

printf """\n\n\n""" >> ./test-cov.js

# Тело тестов
cat ./test/*.js  >> ./test-cov.js

# Завершение теста через 10 секунд
printf """\n \
setTimeout(function(){\n \
     console.log(\"[js-test] [js-test-done-ok] \x1b[1;32m all tests done in js test \x1b[0m\");\
    process.exit()\n \
}, 30000)\n  """ >> ./test-cov.js


# Начало всех тестов через 2 секунды после запуска комет сервера.
printf """\n }, 10000)""" >> ./test-cov.js

istanbul cover ./test-cov.js 


echo "See result in \"`pwd`/coverage/lcov-report/index.html\""