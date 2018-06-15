
## Calculate coverage

Scripts for calculate coverage in tests

```./fulltests.sh```  - для запуска полного цикла тестов

```./jstestrun.sh``` - для запуска js тестов но без запуска инстансов cppcomet (Удобно при написании естов)

```./run.sh``` - Собирает и запускает CppComet с флагами для подсчёта покрытия тестами.

```./coverage.sh```  - генерация html отчёта о покрытии тестами C++ кода

```apt-get install lcov``` установить lcov

## Docker

Use file Dockerfile-dev for work enviroment

For build use command
```docker build --tag debian-dev -f ./Dockerfile-dev .```


For run use command
cd .. && docker run -v $(pwd):/cppcomet -p=8087:8087 -p=3305:3305 -p=3316:3316 -it debian-dev:latest bash