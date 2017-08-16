FROM ubuntu:latest
RUN apt-get update && apt-get -y install git cmake 
RUN apt-get install software-properties-common
RUN add-apt-repository -y ppa:jonathonf/gcc-7.1
RUN apt-get install -y libboost-all-dev zlib1g-dev
RUN apt-get install -y libgamin-dev
RUN apt-get update && apt-get -y install g++-7
RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-7 1
RUN update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-7 1

