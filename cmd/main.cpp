#include <iostream>
#include <sqlite3.h>
#include <string>

// Функция для вывода ошибки и завершения программы
void exitWithError(sqlite3* db, const std::string& message) {
    std::cerr << message << ": " << sqlite3_errmsg(db) << std::endl;
    sqlite3_close(db);
    exit(EXIT_FAILURE);
}

// Функция для поиска маршрутов
void findRoutes(sqlite3* db, const std::string& source, const std::string& destination, 
                const std::string& date, const std::string& transportType) {
    std::string query = R"(
        SELECT r.id, d1.name AS source, d2.name AS destination, r.departure_time, 
               r.arrival_time, r.ticket_price, r.seats_available, t.name AS transport
        FROM routes r
        JOIN destinations d1 ON r.source_id = d1.id
        JOIN destinations d2 ON r.destination_id = d2.id
        JOIN transport_types t ON r.transport_type_id = t.id
        WHERE d1.name = ? AND d2.name = ? AND t.name LIKE ? AND r.departure_time LIKE ?
        ORDER BY r.ticket_price ASC;
    )";

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        exitWithError(db, "Failed to prepare statement");
    }

    // Привязываем параметры
    sqlite3_bind_text(stmt, 1, source.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, destination.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, ("%" + transportType + "%").c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, (date + "%").c_str(), -1, SQLITE_STATIC);

    // Выполняем запрос и выводим результаты
    std::cout << "\nAvailable routes:\n";
    std::cout << "-----------------------------------------\n";
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        const char* src = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        const char* dest = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        const char* departure = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        const char* arrival = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        double price = sqlite3_column_double(stmt, 5);
        int seats = sqlite3_column_int(stmt, 6);
        const char* transport = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));

        std::cout << "Route ID: " << id << "\n"
                  << "From: " << src << " To: " << dest << "\n"
                  << "Departure: " << departure << ", Arrival: " << arrival << "\n"
                  << "Price: " << price << ", Seats Available: " << seats << "\n"
                  << "Transport: " << transport << "\n";
        std::cout << "-----------------------------------------\n";
    }

    sqlite3_finalize(stmt);
}

int main() {
    sqlite3* db;
    if (sqlite3_open("database.db", &db) != SQLITE_OK) {
        exitWithError(db, "Cannot open database");
    }

    std::cout << "Welcome to the Route Finder!\n";
    std::string source, destination, date, transportType;

    // Ввод данных от пользователя
    std::cout << "Enter source location: ";
    std::getline(std::cin, source);

    std::cout << "Enter destination location: ";
    std::getline(std::cin, destination);

    std::cout << "Enter preferred date (YYYY-MM-DD): ";
    std::getline(std::cin, date);

    std::cout << "Enter preferred transport type (or leave empty for any): ";
    std::getline(std::cin, transportType);

    // Поиск маршрутов
    findRoutes(db, source, destination, date, transportType);

    sqlite3_close(db);
    return 0;
}
