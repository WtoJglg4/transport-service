sqlite-build:
	docker build -f docker/Dockerfile -t "sqlite-service" .

sqlite-run: sqlite-build
	docker run -d --rm -it --name sqlite-service -v "$(shell pwd):/app" sqlite-service

sqlite-exec:
	docker exec -it sqlite-service /bin/sh

sqlite-rm: 
	docker stop sqlite-service
	docker image rm sqlite-service

build:
	g++ -o ./bin/main ./cmd/main.cpp -lsqlite3 -std=c++11 -Iinternal/queries


run: build	
	./bin/main
