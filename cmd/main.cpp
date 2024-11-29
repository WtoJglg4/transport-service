#include <iostream>
#include <sqlite3.h>
#include <sstream>
#include <iomanip>
#include <vector>
#include <string>
#include <locale>
#include <codecvt>
#include "queries.cpp"
using namespace std;

const char* pathDB = "internal/db/database.db";

// exitWithError prints an error with FAILURE code
void exitWithError(sqlite3* db, const string& message) {
    cerr << "error: " << message << ": " << sqlite3_errmsg(db) << endl;
    sqlite3_close(db);
    exit(EXIT_FAILURE);
}

// convertToSQLiteFormat converts date and time from "dd.mm.yyyy" to "yyyy-mm-dd"
string convertToSQLiteFormat(const string& date) {
    int day, month, year;
    sscanf(date.c_str(), "%d.%d.%d", &day, &month, &year);
    ostringstream oss;
    oss << year << "-" << setw(2) << setfill('0') << month << "-" << setw(2) << setfill('0') << day;
    return oss.str();
}

// readRouteParameters prints a Menu and reads corresponding fields
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
    const int column_count = 8;
    vector<size_t> column_widths(column_count, 0);
    // Headers
    const vector<string> headers = {"ID", "Source", "Destination", "Departure Time", 
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

// findRoutesByTransport makes sql query statement for selecting 
// available routes with corresponding `transport_type`
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
    setlocale(LC_ALL, "");
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
}
