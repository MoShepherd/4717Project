FROM alpine:latest AS build

RUN apk update \
	&& apk upgrade \
	&& apk add --no-cache clang clang-dev alpine-sdk zeromq-dev libzmq-static 

WORKDIR /app

COPY head.c .

RUN clang -o head head.c -static -lzmq

FROM scratch

COPY --from=build /app/head /head

CMD ["/head"]