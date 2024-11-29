DROP TABLE IF EXISTS transport_types;

DROP TABLE IF EXISTS destinations;

DROP TABLE IF EXISTS routes;

CREATE TABLE transport_types (
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  name TEXT NOT NULL
);

CREATE TABLE destinations (
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  name TEXT NOT NULL
);

CREATE TABLE routes (
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  transport_type_id INTEGER NOT NULL,
  source_id INTEGER NOT NULL,
  destination_id INTEGER NOT NULL,
  distance INTEGER NOT NULL,
  departure_time DATETIME NOT NULL,
  arrival_time DATETIME NOT NULL,
  seats_available INTEGER NOT NULL,
  ticket_price REAL NOT NULL,
  FOREIGN KEY (transport_type_id) REFERENCES transport_types (id),
  FOREIGN KEY (source_id) REFERENCES destinations (id),
  FOREIGN KEY (destination_id) REFERENCES destinations (id)
);