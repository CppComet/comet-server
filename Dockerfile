FROM debian:9
MAINTAINER Trapenok Victor <TrapenokVictor@yandex.ru>


RUN apt-get update && apt-get install --assume-yes cmake make cpp gcc libssl-dev g++ pkg-config libmariadbclient-dev-compat mysql-client flex mailutils uuid-dev git wget
RUN git clone https://github.com/CppComet/comet-server && cd comet-server && cmake . && make && make install && mkdir /var/log/cppcomet
RUN apt-get remove -y cmake make flex git cpp gcc g++  && apt-get clean

EXPOSE 8087
EXPOSE 3307
EXPOSE 3311

CMD ["/usr/bin/cppcomet"]

# You need put comet.ini file in to /etc/comet-server/comet.ini with configuration for coocomet
# Build docker with command
# docker build --tag cppcomet .
# Start docker with command
# docker run -v /var/log/cppcomet:/var/log/cppcomet -v /etc/comet-server/comet.ini:/etc/comet-server/comet.ini -p=8088:80 -p=3305:3306 -p=3316:3316 cppcomet:latest



# stop all containers:
# docker kill $(docker ps -q)

# remove all containers
# docker rm $(docker ps -a -q)

# remove all docker images
# docker rmi $(docker images -q)
