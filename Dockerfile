FROM alpine:latest AS build

RUN apk update \
	&& apk upgrade \
	&& apk add --no-cache alpine-sdk clang clang-dev libsodium-dev libsodium-static zeromq-dev libzmq-static 

WORKDIR /app

COPY head.c .

RUN clang -o head head.c -static -lzmq -lstdc++ -lsodium

FROM scratch

COPY --from=build /app/head /head

CMD ["/head"]