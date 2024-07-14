#include <iostream>
#include <sqlite3.h>
#include"utilities.hpp"

int main() {
    sqlite3* db = open_database("archive.db");

    // SQL statements to create tables
    const char *sql_create_aHeader = R"(
    CREATE TABLE IF NOT EXISTS aHeader (
        PK INTEGER PRIMARY KEY,
        SK_hash TEXT,
        flags INTEGER,
        height INTEGER,
        mtp INTEGER,
        header_FK INTEGER,
        version INTEGER,
        time INTEGER,
        bits INTEGER,
        nonce INTEGER,
        merkle_root TEXT
    );
    )";

    const char *sql_create_aTxs = R"(
    CREATE TABLE IF NOT EXISTS aTxs (
        SK_header_FK INTEGER,
        transaction_FK INTEGER,
        FOREIGN KEY (SK_header_FK) REFERENCES aHeader(PK),
        FOREIGN KEY (transaction_FK) REFERENCES aTransaction(PK)
    );
    )";

    const char *sql_create_aTransaction = R"(
    CREATE TABLE IF NOT EXISTS aTransaction (
        PK INTEGER PRIMARY KEY,
        SK_hash TEXT,
        coinbase TEXT,
        size INTEGER,
        weight INTEGER,
        locktime INTEGER,
        version INTEGER,
        input_count INTEGER,
        inputs_FK INTEGER,
        output_count INTEGER,
        outputs_FK INTEGER,
        FOREIGN KEY (inputs_FK) REFERENCES aInput(PK),
        FOREIGN KEY (outputs_FK) REFERENCES aOutput(PK)
    );
    )";

    const char *sql_create_aPuts = R"(
    CREATE TABLE IF NOT EXISTS aPuts (
        PK INTEGER PRIMARY KEY,
        put_FK INTEGER,
        FOREIGN KEY (put_FK) REFERENCES aTransaction(PK)
    );
    )";

    const char *sql_create_aInput = R"(
    CREATE TABLE IF NOT EXISTS aInput (
        PK INTEGER PRIMARY KEY,
        SK_index INTEGER,
        SK_point_FK INTEGER,
        transaction_FK INTEGER,
        indx INTEGER,
        sequence INTEGER,
        length INTEGER,
        script TEXT,
        count INTEGER,
        witness TEXT,
        FOREIGN KEY (SK_point_FK) REFERENCES aPoint(PK),
        FOREIGN KEY (transaction_FK) REFERENCES aTransaction(PK)
    );
    )";

    const char *sql_create_aPoint = R"(
    CREATE TABLE IF NOT EXISTS aPoint (
        PK INTEGER PRIMARY KEY,
        SK_hash TEXT
    );
    )";

    const char *sql_create_aOutput = R"(
    CREATE TABLE IF NOT EXISTS aOutput (
        PK INTEGER PRIMARY KEY,
        transaction_FK INTEGER,
        indx INTEGER,
        value INTEGER,
        length INTEGER,
        script TEXT,
        FOREIGN KEY (transaction_FK) REFERENCES aTransaction(PK)
    );
    )";

    // Execute SQL statements to create tables
    executeSQL(db, sql_create_aHeader);
    executeSQL(db, sql_create_aTxs);
    executeSQL(db, sql_create_aTransaction);
    executeSQL(db, sql_create_aPuts);
    executeSQL(db, sql_create_aInput);
    executeSQL(db, sql_create_aPoint);
    executeSQL(db, sql_create_aOutput);
    
    // aHeader JOIN aTxs JOIN aTransaction JOIN aInput JOIN aPoint JOIN aOutput
    const char *sql_join_query = R"(
    SELECT *
    FROM aHeader AS h
    JOIN aTxs AS t ON h.PK = t.SK_header_FK
    JOIN aTransaction AS txn ON t.transaction_FK = txn.PK
    JOIN aInput AS inp ON txn.inputs_FK = inp.PK
    JOIN aPoint AS pt ON inp.SK_point_FK = pt.PK
    JOIN aOutput AS out ON txn.outputs_FK = out.PK;
    )";

    // Execute the JOIN query
    executeSQL(db, sql_join_query);

    // Close the database connection
    sqlite3_close(db);

    return 0;
}