#include <iostream>
#include <sqlite3.h>
#include <sstream>
#include <iomanip>
#include <vector>
#include <string>
#include <locale>
#include <codecvt>
#include <regex>
#include "queries.cpp"
using namespace std;

/**********************************************************
*                Технология Программирования              *
***********************************************************
*Project type: Win64 Console Application                  *
*Project name: service.sln                                *
*File name: main.cpp                                      *
*Language:CPP, MSVS 2022                                  *
*Programmers: Глазов В.B., Абдурахманов М.Р. М3О-309Б-22  *
**********************************************************/


const char* pathDB = "internal/db/database.db";

// exitWithError prints an error with FAILURE code
void exitWithError(sqlite3* db, const string& message) {
    cerr << "error: " << message << ": " << sqlite3_errmsg(db) << endl;
    sqlite3_close(db);
    exit(EXIT_FAILURE);
}

bool isValidDate(const string& date) {
    // Regexp for "dd.mm.yyyy"
    regex dateRegex(R"(^(\d{2})\.(\d{2})\.(\d{4})$)");
    smatch match;
    // Check matching regexp
    if (!regex_match(date, match, dateRegex)) {
        return false;
    }
    // Get day, month, yearfrom string
    int day = stoi(match[1].str());
    int month = stoi(match[2].str());
    int year = stoi(match[3].str());
    // Check month
    if (month < 1 || month > 12) {
        return false;
    }
    // Number of days per month
    const int daysInMonth[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    // Check leap year
    bool isLeapYear = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
    // Check max days per months
    int maxDays = daysInMonth[month - 1];
    if (month == 2 && isLeapYear) {
        maxDays = 29; // February in leap year
    }
    // Check day
    return day >= 1 && day <= maxDays;
}

// Checks if string is non-negative integer number
bool isNonNegativeNumber(const string& str) {
    regex numberRegex(R"(^\d+(\.\d+)?$)");
    return regex_match(str, numberRegex);
}

// convertToSQLiteFormat converts date and time from "dd.mm.yyyy" to "yyyy-mm-dd"
string convertToSQLiteFormat(const string& date) {
    int day, month, year;
    sscanf(date.c_str(), "%d.%d.%d", &day, &month, &year);
    ostringstream oss;
    oss << year << "-" << setw(2) << setfill('0') << month << "-" << setw(2) << setfill('0') << day;
    return oss.str();
}

// readSelectRouteParameters prints a Menu and reads corresponding fields
void readSelectRouteParameters(string& source, string& destination, string& date, string& transportType, string& ticketPrice) {
    cout << "Enter source location: ";
    getline(cin, source);
    cout << "Enter destination location: ";
    getline(cin, destination);
    cout << "Enter preferred departure date (DD.MM.YYYY): ";
    getline(cin, date);
    if (!isValidDate(date)) {
        cerr << "invalid date format\n";
        exit(1);
    }
    cout << "Enter preferred transport type (or leave empty for any): ";
    getline(cin, transportType);
    cout << "Enter higher ticket price (or leave empty for any): ";
    getline(cin, ticketPrice);
    if (!isNonNegativeNumber(ticketPrice)) {
        cerr << "invalid price\n";
        exit(1);
    }
}

// readInsertRouteParameters prints a Menu and reads corresponding fields
void readInsertRouteParameters(
    string& flight, 
    string& source, 
    string& destination, 
    string& departureDate, 
    string& arrivalDate,
    string& transportType, 
    string& ticketPrice, 
    string& distance, 
    string& seatsAvaliable) {
    cout << "Enter flight number: ";
    getline(cin, flight);
    cout << "Enter source location: ";
    getline(cin, source);
    cout << "Enter destination location: ";
    getline(cin, destination);
    cout << "Enter departure date (DD.MM.YYYY): ";
    getline(cin, departureDate);
    if (!isValidDate(departureDate)) {
        cerr << "invalid date format\n";
        exit(1);
    }
    cout << "Enter arrival date (DD.MM.YYYY): ";
    getline(cin, arrivalDate);
    if (!isValidDate(arrivalDate)) {
        cerr << "invalid date format\n";
        exit(1);
    }
    cout << "Enter transport type: ";
    getline(cin, transportType);
    cout << "Enter ticket price: ";
    getline(cin, ticketPrice);
    if (!isNonNegativeNumber(ticketPrice)) {
        cerr << "invalid price\n";
        exit(1);
    }
    cout << "Enter distance: ";
    getline(cin, distance);
    if (!isNonNegativeNumber(distance)) {
        cerr << "invalid distance\n";
        exit(1);
    }
    cout << "Enter number of avaliable seats: ";
    getline(cin, seatsAvaliable);
    if (!isNonNegativeNumber(seatsAvaliable)) {
        cerr << "invalid number of seats\n";
        exit(1);
    }
}

// utf8StringLen counts number of symbols of UTF-8-formated string (not number of bytes)
int utf8StringLen(const string& str) {
    int count = 0;
    for (size_t i = 0; i < str.size(); ++i) {
        // Symbol starts from 0xxxxxxx or 11xxxxxx in UTF-8
        if ((str[i] & 0xC0) != 0x80) {
            ++count;
        }
    }
    return count;
}

// printRaw print table's data row with separators
void printRaw(vector<string> row, vector<size_t> widths){
    cout << "|";
        for (size_t i = 0; i < row.size(); ++i) {
            cout << " " << row[i];
            for (int j = 0; j < widths[i] - utf8StringLen(row[i])-1; j++){
                cout << " ";
            }
            cout << "|";
        }
        cout << "\n";
}

// printLine print tabel's separators
void printLine(vector<size_t> column_widths){
    cout << "+";
    for (size_t width : column_widths) {
        cout << string(width, '-') << "+";
    }
    cout << "\n";
}

// doAndPrintQueryResult do sql-request and prints it to stdout as result table
void doAndPrintQueryResult(sqlite3_stmt *stmt) {
    const int column_count = 10;
    vector<size_t> column_widths(column_count, 0);
    // Headers
    const vector<string> headers = {"ID", "Flight", "Source", "Destination", "Distance", "Departure Time", 
                                    "Arrival Time", "Price", "Seats", "Transport"};
    // Get max columns widths
    for (size_t i = 0; i < headers.size(); ++i) {
        column_widths[i] = utf8StringLen(headers[i]) + 2;
    }
    // Do sql request, write results into `rows`
    vector<vector<string>> rows;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        vector<string> row;
        for (int i = 0; i < column_count; ++i) {
            const char* text = reinterpret_cast<const char*>(sqlite3_column_text(stmt, i));
            string value = text ? text : "N/A"; // not assigned
            row.push_back(value);
            // Update max columns widths
            size_t display_width = utf8StringLen(value) + 2;
            if (display_width > column_widths[i]) {
                column_widths[i] = display_width;
            }
        }
        rows.push_back(row);
    }
    // Print separators with headers
    printLine(column_widths);
    printRaw(headers, column_widths);
    printLine(column_widths);
    // Print data
    for (const auto& row : rows) {
        printRaw(row, column_widths);
    }
    printLine(column_widths);
}

// showAllRoutes makes sql query statement for selecting all available routes
void showAllRoutes(sqlite3* db, string queryTemplate) {
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, queryTemplate.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        exitWithError(db, "failed to prepare statement");
    }
    // Do query and print th results
    doAndPrintQueryResult(stmt);
    sqlite3_finalize(stmt);
}

// findRoutes makes sql query statement for selecting all available routes with specified fields
void findRoutes(
    sqlite3* db, 
    string queryTemplate, 
    const string& source, 
    const string& destination, 
    const string& date, 
    const string& transportType,
    const string& ticketPrice) {
    // Format date from 'dd.mm.yyyy' to 'yyyy-dd-mm'
    string formatted_date = convertToSQLiteFormat(date);
    sqlite3_stmt* stmt;
    // Inserting query parameters into query
    if (sqlite3_prepare_v2(db, queryTemplate.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        exitWithError(db, "failed to prepare query");
    }
    // Bind parameters
    if (sqlite3_bind_text(stmt, 1, formatted_date.c_str(), -1, SQLITE_STATIC) != SQLITE_OK ||
        sqlite3_bind_text(stmt, 2, source.c_str(), -1, SQLITE_STATIC) != SQLITE_OK ||
        sqlite3_bind_text(stmt, 3, destination.c_str(), -1, SQLITE_STATIC) != SQLITE_OK ||
        sqlite3_bind_text(stmt, 4, transportType.c_str(), -1, SQLITE_STATIC) != SQLITE_OK ||
        sqlite3_bind_text(stmt, 5, transportType.c_str(), -1, SQLITE_STATIC) != SQLITE_OK ||
        sqlite3_bind_text(stmt, 6, ticketPrice.c_str(), -1, SQLITE_STATIC) != SQLITE_OK ||
        sqlite3_bind_text(stmt, 7, ticketPrice.c_str(), -1, SQLITE_STATIC) != SQLITE_OK) {
        exitWithError(db, "failed to bind parameters");
    }
    // Do query and print th results
    doAndPrintQueryResult(stmt);
    sqlite3_finalize(stmt);
}

// findRoutesByTransport makes sql query statement for selecting 
// available routes with specified `transport_type`
void findRoutesByTransport(
    sqlite3* db, 
    string queryTemplate, 
    const string& transportType) {
    sqlite3_stmt* stmt;
    // Inserting query parameters into query
    if (sqlite3_prepare_v2(db, queryTemplate.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        exitWithError(db, "failed to prepare query");
    }
    // Bind parameters
    if (sqlite3_bind_text(stmt, 1, transportType.c_str(), -1, SQLITE_STATIC) != SQLITE_OK) {
        exitWithError(db, "failed to bind parameters");
    }
    // Do query and print th results
    doAndPrintQueryResult(stmt);
    sqlite3_finalize(stmt);
}

// findRoutesByPrice makes sql query statement for selecting 
// available routes with specified `ticket_price`
void findRoutesByPrice(
    sqlite3* db, 
    string queryTemplate, 
    const string& ticketPrice) {
    sqlite3_stmt* stmt;
    // Inserting query parameters into query
    if (sqlite3_prepare_v2(db, queryTemplate.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        exitWithError(db, "failed to prepare query");
    }
    // Bind parameters
    if (sqlite3_bind_text(stmt, 1, ticketPrice.c_str(), -1, SQLITE_STATIC) != SQLITE_OK) {
        exitWithError(db, "failed to bind parameters");
    }
    // Do query and print th results
    doAndPrintQueryResult(stmt);
    sqlite3_finalize(stmt);
}

// insertIfNotExists inserts value into table is value not exists
void insertIfNotExists(sqlite3* db, const string& queryTemplate, const string& field) {
    sqlite3_stmt* stmt = nullptr;
    // Preparing query
    if (sqlite3_prepare_v2(db, queryTemplate.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        exitWithError(db, "failed to prepare query");
    }
    // Bind parameters
    if (sqlite3_bind_text(stmt, 1, field.c_str(), -1, SQLITE_STATIC) != SQLITE_OK ||
        sqlite3_bind_text(stmt, 2, field.c_str(), -1, SQLITE_STATIC) != SQLITE_OK) {
        exitWithError(db, "failed to bind parameters");
    }
    // Do query
    int rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        exitWithError(db, "failed to execute statement");
    }
    // Free resources
    sqlite3_finalize(stmt);
}

// insertRoute inserts a new route by specified parameters
void insertRoute(
    sqlite3* db, 
    string queryTemplate, 
    string& flight,
    string& source, 
    string& destination, 
    string& departureDate, 
    string& arrivalDate,
    string& transportType, 
    string& ticketPrice, 
    string& distance, 
    string& seatsAvaliable) {
    // Format date from 'dd.mm.yyyy' to 'yyyy-dd-mm'
    string departureDateFormatted = convertToSQLiteFormat(departureDate);
    string arrivalDateFormatted = convertToSQLiteFormat(arrivalDate);
    // Insert specified transport_type, source and destination cities 
    // if they are not exists in corresponding tables
    insertIfNotExists(db, InsertInTranportTypesIfNotExists, transportType);
    insertIfNotExists(db, InsertInDestinationsIfNotExists, source);
    insertIfNotExists(db, InsertInDestinationsIfNotExists, destination);

    sqlite3_stmt* stmt;
    // Inserting query parameters into query
    if (sqlite3_prepare_v2(db, queryTemplate.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        exitWithError(db, "failed to prepare query");
    }
    // Bind parameters
    if (sqlite3_bind_text(stmt, 1, flight.c_str(), -1, SQLITE_STATIC)!= SQLITE_OK ||
        sqlite3_bind_text(stmt, 2, transportType.c_str(), -1, SQLITE_STATIC)!= SQLITE_OK ||
        sqlite3_bind_text(stmt, 3, source.c_str(), -1, SQLITE_STATIC)!= SQLITE_OK ||
        sqlite3_bind_text(stmt, 4, destination.c_str(), -1, SQLITE_STATIC)!= SQLITE_OK ||
        sqlite3_bind_text(stmt, 5, distance.c_str(), -1, SQLITE_STATIC)!= SQLITE_OK ||
        sqlite3_bind_text(stmt, 6, departureDateFormatted.c_str(), -1, SQLITE_STATIC)!= SQLITE_OK ||
        sqlite3_bind_text(stmt, 7, arrivalDateFormatted.c_str(), -1, SQLITE_STATIC)!= SQLITE_OK ||
        sqlite3_bind_text(stmt, 8, seatsAvaliable.c_str(), -1, SQLITE_STATIC)!= SQLITE_OK ||
        sqlite3_bind_text(stmt, 9, ticketPrice.c_str(), -1, SQLITE_STATIC) != SQLITE_OK) {
        exitWithError(db, "failed to bind parameters");
    }
    // Do query
    int rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        exitWithError(db, "failed to execute statement");
    }
    // Free resources
    sqlite3_finalize(stmt);
}

int main() {
    sqlite3* db;
    if (sqlite3_open(pathDB, &db) != SQLITE_OK) {
        exitWithError(db, "cannot open database");
    }
    // Define query parameters
    string source, flight, destination, departureDate, arrivalDate, 
    transportType, ticketPrice, distance, seatsAvaliable;

    cout << "Welcome to the Route Finder!\n";

    string operatoinRaw;
    int operation;
    while (true){
        cout << "Choose the operation:\n";
        cout << "Show all routes - 1\n";
        cout << "Find route by parameters - 2\n";
        cout << "Find route by transport type - 3\n";
        cout << "Find route by ticket price - 4\n";
        cout << "Insert a new route - 5\n";
        cout << "Exit - 6\n";
        getline(cin, operatoinRaw);
        operation = atoi(operatoinRaw.c_str());
        switch (operation){
            case 1:
                showAllRoutes(db, SelectAllRoutesQuery);
                break;
            case 2:
                readSelectRouteParameters(source, destination, departureDate, transportType, ticketPrice);
                findRoutes(db, SelectRoutesQuery, source, destination, departureDate, transportType, ticketPrice);
                break;
            case 3:
                cout << "Enter preferred transport type: ";
                getline(cin, transportType);
                findRoutesByTransport(db, SelectRoutesByTransportTypeQuery, transportType);
                break;
            case 4:
                cout << "Enter ticket price: ";
                getline(cin, ticketPrice);
                if (!isNonNegativeNumber(ticketPrice)) {
                    cerr << "invalid price\n";
                    exit(1);
                }
                findRoutesByPrice(db, SelectRoutesByTicketPriceQuery, ticketPrice);
                break;
            case 5:
                readInsertRouteParameters(
                    flight,
                    source, 
                    destination, 
                    departureDate,
                    arrivalDate, 
                    transportType, 
                    ticketPrice,
                    distance,
                    seatsAvaliable);
                insertRoute(
                    db, 
                    InsertRouteQuery,
                    flight,
                    source, 
                    destination, 
                    departureDate,
                    arrivalDate, 
                    transportType, 
                    ticketPrice,
                    distance,
                    seatsAvaliable);
                break;
            case 6:
                return 0;
            default:
                exitWithError(db, "invalid operation");
                break;
        }
    }
}
