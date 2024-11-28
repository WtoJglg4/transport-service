#include <iostream>
#include <sqlite3.h>
#include <sstream>
#include <iomanip>
#include "queries.cpp"
using namespace std;

const char* pathDB = "internal/db/database.db";

// Функция для вывода ошибки и завершения программы
void exitWithError(sqlite3* db, const string& message) {
    cerr << "error: " << message << ": " << sqlite3_errmsg(db) << endl;
    sqlite3_close(db);
    exit(EXIT_FAILURE);
}

// Функция для преобразования даты с формата "dd.mm.yyyy" в "yyyy-mm-dd"
string convertToSQLiteFormat(const string& date) {
    // date в формате "dd.mm.yyyy"
    int day, month, year;
    sscanf(date.c_str(), "%d.%d.%d", &day, &month, &year);

    // Преобразуем в формат "yyyy-mm-dd"
    ostringstream oss;
    oss << year << "-" << setw(2) << setfill('0') << month << "-" << setw(2) << setfill('0') << day;
    return oss.str();
}

void readRouteParameters(string& source, string& destination, string& date, string& transportType) {
    cout << "Enter source location: ";
    getline(cin, source);
    cout << "Enter destination location: ";
    getline(cin, destination);
    cout << "Enter preferred date (DD.MM.YYYY): ";
    getline(cin, date);
    cout << "Enter preferred transport type (or leave empty for any): ";
    getline(cin, transportType);
}

void doAndPrintQueryResult(sqlite3_stmt *stmt) {
    cout << "\nAvailable routes:\n";
    cout << "-----------------------------------------\n";
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        const char* src = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        const char* dest = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        const char* departure = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        const char* arrival = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        double price = sqlite3_column_double(stmt, 5);
        int seats = sqlite3_column_int(stmt, 6);
        const char* transport = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
        cout << "Route ID: " << id << "\n"
                  << "From: " << src << " To: " << dest << "\n"
                  << "Departure: " << departure << ", Arrival: " << arrival << "\n"
                  << "Price: " << price << ", Seats Available: " << seats << "\n"
                  << "Transport: " << transport << "\n";
        cout << "-----------------------------------------\n";
    }
}

// Функция для поиска всех маршрутов
void showAllRoutes(sqlite3* db, string queryTemplate) {
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, queryTemplate.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        exitWithError(db, "failed to prepare statement");
    }
    // Do query and print th results
    doAndPrintQueryResult(stmt);
    sqlite3_finalize(stmt);
}


// Функция для поиска маршрутов с фильтрацией по дате, источнику и типу транспорта
void findRoutes(
    sqlite3* db, 
    string queryTemplate, 
    const string& source, 
    const string& destination, 
    const string& date, 
    const string& transportType) {
    // Format date from 'dd.mm.yyyy' to 'yyyy-dd-mm'
    string formatted_date = convertToSQLiteFormat(date);
    sqlite3_stmt* stmt;
    // Inserting query parameters into query
    sqlite3_prepare_v2(db, queryTemplate.c_str(), -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, formatted_date.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, source.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, destination.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, transportType.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, transportType.c_str(), -1, SQLITE_STATIC);
    // Do query and print th results
    doAndPrintQueryResult(stmt);
    sqlite3_finalize(stmt);
}

void findRoutesByTransport(
    sqlite3* db, 
    string queryTemplate, 
    const string& transportType) {
    sqlite3_stmt* stmt;
    // Inserting query parameters into query
    sqlite3_prepare_v2(db, queryTemplate.c_str(), -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, transportType.c_str(), -1, SQLITE_STATIC);
    // Do query and print th results
    doAndPrintQueryResult(stmt);
    sqlite3_finalize(stmt);
}

int main() {
    sqlite3* db;
    if (sqlite3_open(pathDB, &db) != SQLITE_OK) {
        exitWithError(db, "cannot open database");
    }
    string source, destination, date, transportType;

    cout << "Welcome to the Route Finder!\n";

    string operatoinRaw;
    int operation;
    while (true){
        cout << "Choose the operation:\n";
        cout << "Show all routes - 1\n";
        cout << "Find route by parameters - 2\n";
        cout << "Find route by transport type - 3\n";
        cout << "Exit - 4\n";
        getline(cin, operatoinRaw);
        operation = atoi(operatoinRaw.c_str());
        switch (operation){
            case 1:
                showAllRoutes(db, SelectAllRoutesQuery);
                break;
            case 2:
                readRouteParameters(source, destination, date, transportType);
                findRoutes(db, SelectRoutesQuery, source, destination, date, transportType);
                break;
            case 3:
                cout << "Enter preferred transport type: ";
                getline(cin, transportType);
                findRoutesByTransport(db, SelectRoutesByTransportTypeQuery, transportType);
                break;
            case 4:
                return 0;
            default:
                exitWithError(db, "invalid operation");
                break;
        }
    }

    // Ввод данных от пользователя
    cout << "Enter source location: ";
    // getline(cin, source);

    cout << "Enter destination location: ";
    // getline(cin, destination);

    cout << "Enter preferred date (DD.MM.YYYY): ";
    // getline(cin, date);

    cout << "Enter preferred transport type (or leave empty for any): ";
    // getline(cin, transportType);

    source = "Москва";
    destination = "Санкт-Петербург";
    date = "20.11.2024";
    transportType = "Самолет";
    // transportType = "Поезд";

    // Поиск маршрутов
    findRoutes(db, SelectRoutesQuery, source, destination, date, transportType);

    sqlite3_close(db);
    return 0;
}
