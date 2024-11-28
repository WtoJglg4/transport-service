#include <iostream>
using namespace std;

string SelectAllRoutesQuery = R"(
        SELECT r.id, d1.name AS source, d2.name AS destination, r.departure_time, 
               r.arrival_time, r.ticket_price, r.seats_available, t.name AS transport
        FROM routes r
        JOIN destinations d1 ON r.source_id = d1.id
        JOIN destinations d2 ON r.destination_id = d2.id
        JOIN transport_types t ON r.transport_type_id = t.id
        ORDER BY r.ticket_price ASC;
    )";

string SelectRoutesQuery = R"(
    SELECT r.id, d1.name AS source, d2.name AS destination, 
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
    ORDER BY r.departure_time ASC;
)";

string SelectRoutesByTransportTypeQuery = R"(
    SELECT r.id, d1.name AS source, d2.name AS destination, 
    strftime('%d.%m.%Y %H:%M:%S', r.departure_time) AS departure_time, 
        r.arrival_time, r.ticket_price, r.seats_available, t.name AS transport
    FROM routes r
    JOIN destinations d1 ON r.source_id = d1.id
    JOIN destinations d2 ON r.destination_id = d2.id
    JOIN transport_types t ON r.transport_type_id = t.id
    WHERE t.name = ?
    ORDER BY t.name ASC;
)";