#include <iostream>
#include <sqlite3.h>

void executeSQL(sqlite3 *db, const char *sql) {
    char *errorMessage = 0;
    int rc = sqlite3_exec(db, sql, 0, 0, &errorMessage);

    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << errorMessage << std::endl;
        sqlite3_free(errorMessage);
    } else {
        std::cout << "SQL executed successfully" << std::endl;
    }
}

sqlite3* open_database(const char* filename) {
  sqlite3 *db;
  int rc = sqlite3_open(filename, &db);

  if (rc) {
    std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
    sqlite3_close(db); // Close the database connection even on error
    return nullptr;
  } else {
    std::cout << "Opened database successfully" << std::endl;
  }

  return db;
}

int rc_to_database(const char* filename){
    sqlite3 *db;
    return sqlite3_open("archive.db", &db);
}
