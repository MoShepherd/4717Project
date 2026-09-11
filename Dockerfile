FROM alpine:latest AS build

RUN apk update \
	&& apk upgrade \
	&& apk add --no-cache clang clang-dev alpine-sdk

WORKDIR /app

COPY head.cpp .

RUN clang++ -o head head.cpp -static

FROM scratch

COPY --from=build /app/head /head

CMD ["/head"]