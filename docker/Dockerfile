FROM keinos/sqlite3:latest

WORKDIR /app

COPY ../internal/migrations/create.sql /app/create.sql
COPY ../internal/migrations/insert_data.sql /app/insert_data.sql

CMD sqlite3 /app/database.db < /app/create.sql && \
    sqlite3 /app/database.db < /app/insert_data.sql && \
    sqlite3 /app/database.db   
