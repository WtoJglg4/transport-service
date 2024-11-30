#include <iostream>
using namespace std;

string SelectAllRoutesQuery = R"(
        SELECT r.id, r.flight, d1.name AS source, d2.name AS destination, r.distance, r.departure_time, 
               r.arrival_time, r.ticket_price, r.seats_available, t.name AS transport
        FROM routes r
        JOIN destinations d1 ON r.source_id = d1.id
        JOIN destinations d2 ON r.destination_id = d2.id
        JOIN transport_types t ON r.transport_type_id = t.id
        ORDER BY r.ticket_price ASC;
    )";

string SelectRoutesQuery = R"(
    SELECT r.id,  r.flight, d1.name AS source, d2.name AS destination, r.distance,
    strftime('%d.%m.%Y %H:%M:%S', r.departure_time) AS departure_time, 
        r.arrival_time, r.ticket_price, r.seats_available, t.name AS transport
    FROM routes r
    JOIN destinations d1 ON r.source_id = d1.id
    JOIN destinations d2 ON r.destination_id = d2.id
    JOIN transport_types t ON r.transport_type_id = t.id
    WHERE r.departure_time >= ?
    AND d1.name = ?
    AND d2.name = ?
    AND (t.name = ? OR ? = '')
    AND (r.ticket_price <= ? OR ? = '')
    ORDER BY r.departure_time ASC;
)";

string SelectRoutesByTransportTypeQuery = R"(
    SELECT r.id,  r.flight, d1.name AS source, d2.name AS destination, r.distance,
    strftime('%d.%m.%Y %H:%M:%S', r.departure_time) AS departure_time, 
        r.arrival_time, r.ticket_price, r.seats_available, t.name AS transport
    FROM routes r
    JOIN destinations d1 ON r.source_id = d1.id
    JOIN destinations d2 ON r.destination_id = d2.id
    JOIN transport_types t ON r.transport_type_id = t.id
    WHERE t.name = ?
    ORDER BY t.name ASC;
)";

string SelectRoutesByTicketPriceQuery = R"(
    SELECT r.id, r.flight, d1.name AS source, d2.name AS destination, r.distance,
    strftime('%d.%m.%Y %H:%M:%S', r.departure_time) AS departure_time, 
        r.arrival_time, r.ticket_price, r.seats_available, t.name AS transport
    FROM routes r
    JOIN destinations d1 ON r.source_id = d1.id
    JOIN destinations d2 ON r.destination_id = d2.id
    JOIN transport_types t ON r.transport_type_id = t.id
    WHERE r.ticket_price <= ?
    ORDER BY r.ticket_price ASC;
)";

string InsertRouteQuery = R"(
INSERT INTO routes (
    flight,
    transport_type_id, 
    source_id, 
    destination_id, 
    distance, 
    departure_time, 
    arrival_time, 
    seats_available, 
    ticket_price
)
SELECT
    ?,
    (SELECT id FROM transport_types WHERE name = ?),
    (SELECT id FROM destinations WHERE name = ?),
    (SELECT id FROM destinations WHERE name = ?),
    ?,
    ?,
    ?,
    ?,
    ?
;
)";

string InsertInTranportTypesIfNotExists = R"(
    INSERT INTO transport_types (name) SELECT ? WHERE NOT EXISTS (SELECT 1 FROM transport_types WHERE name =  ?);
)";

string InsertInDestinationsIfNotExists = R"(
    INSERT INTO destinations (name) SELECT ? WHERE NOT EXISTS (SELECT 1 FROM destinations WHERE name =  ?);
)";