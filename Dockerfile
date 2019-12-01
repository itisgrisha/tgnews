FROM debian:10.1

RUN mkdir /tgnews && mkdir /tgnews/meta /tgnews/models
COPY build/tgnews /tgnews
COPY models /tgnews/models
COPY meta /tgnews/meta

#RUN apt-get update && apt-get install -y libboost-all-dev libprotobuf-dev
