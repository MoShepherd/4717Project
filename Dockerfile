FROM alpine:latest AS build

RUN apk update \
	&& apk upgrade \
	&& apk add --no-cache alpine-sdk clang clang-dev libsodium-dev libsodium-static zeromq-dev libzmq-static 

WORKDIR /app

COPY head.c .
COPY worker.c .
COPY common.h .

RUN clang -o head head.c -static -lzmq -lstdc++ -lsodium -std=c23
RUN clang -o worker worker.c -static -lzmq -lstdc++ -lsodium -std=c23

FROM scratch AS head

COPY --from=build /app/head /head

EXPOSE 4770
EXPOSE 4771

CMD ["/head"]

FROM scratch AS worker

COPY --from=build /app/worker /worker

CMD ["/worker"]