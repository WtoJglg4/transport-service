FROM keinos/sqlite3:latest

WORKDIR /app

CMD sqlite3 /app/internal/db/database.db < /app/internal/migrations/create.sql && \
    sqlite3 /app/internal/db/database.db < /app/internal/migrations/insert_data.sql && \
    sqlite3 /app/internal/db/database.db   


