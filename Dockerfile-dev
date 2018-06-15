FROM debian:9
MAINTAINER Levhav@yandex.ru


RUN apt-get update && apt-get install --assume-yes htop nano mc cmake make cpp gcc libssl-dev g++ pkg-config libmariadbclient-dev-compat mysql-client flex mailutils uuid-dev git wget checkinstall
RUN apt-get install -y build-essential libssl-dev curl

RUN curl -sL https://deb.nodesource.com/setup_10.x | bash -
RUN apt-get install -y nodejs valgrind

RUN npm install websockets mysql
  
EXPOSE 8087
EXPOSE 3307 
EXPOSE 3311 

# RUN history -s "rm -rf /comet/ && cp -R /cppcomet/ /comet && cd /comet/ && rm -rf /comet/dependencies/jwt-cpp/CMakeCache.txt && rm -rf /comet/CMakeCache.txt && cmake . && make"

CMD ["/bin/bash"]

# This file used for building .deb package
# Build docker with command
# docker build --tag debian-dev -f ./Dockerfile-dev .


# Start docker with command
# docker run -v $(pwd):/cppcomet -p=8087:8087 -p=3305:3305 -p=3316:3316 -it debian-dev:latest bash



# stop all containers:
# docker kill $(docker ps -q)

# remove all containers
# docker rm $(docker ps -a -q)

# remove all docker images
# docker rmi $(docker images -q)
