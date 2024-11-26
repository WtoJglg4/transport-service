sqlite-build:
	docker build -f docker/Dockerfile -t "sqlite-service" .

sqlite-run: 
	# docker run -it sqlite-service
	docker run --rm -it -v "$(pwd):/app" sqlite-service ./app

sqlite-rm: 
	docker stop sqlite-service
	docker image rm sqlite-service

build:
	g++ -o ./bin/main ./cmd/main.cpp -lsqlite3


