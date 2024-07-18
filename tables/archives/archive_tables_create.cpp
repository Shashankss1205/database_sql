#include <iostream>
#include <sqlite3.h>
#include "utilities.hpp"

int main() {
    sqlite3* db = open_database("archive.db");

    // SQL statements to create tables
    const char *sql_create_aHeader = R"(
    CREATE TABLE IF NOT EXISTS aHeader (
        PK INTEGER PRIMARY KEY,
        SK_hash CHAR(32), -- 32 bytes number
        flags INTEGER, -- 32 bit
        height INTEGER, -- PK
        mtp INTEGER, -- 32 bit
        header_FK INTEGER, -- PK
        version INTEGER, -- 32 bit
        time INTEGER, -- 32 bit
        bits INTEGER, -- 32 bit
        nonce INTEGER, -- 32 bit
        merkle_root INTEGER -- 32 bytes
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
        SK_hash CHAR(32), -- 32 bytes hash, indexing basis
        coinbase BOOLEAN, -- boolean
        size INTEGER, -- 32 bit int
        weight INTEGER, -- 32 bit int
        locktime INTEGER, -- 32 bit int
        version INTEGER, -- 32 bit int
        input_count INTEGER, -- 16 bits/32 bits
        inputs_FK INTEGER,
        output_count INTEGER, -- 16 bits/32 bits
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
        PK INTEGER PRIMARY KEY, -- 32 bit int
        SK_index INTEGER, -- spend table
        SK_point_FK INTEGER, 
        transaction_FK INTEGER, -- indexing
        indx SMALLINT, -- 16 bits
        sequence INTEGER, -- 32 bits
        length INTEGER, -- 32 bits / byte len of script
        script BLOB, -- 1mb
        count INTEGER, -- stack length 32 bits
        witness BLOB, -- blob => deserialize => stack
        FOREIGN KEY (SK_point_FK) REFERENCES aPoint(PK),
        FOREIGN KEY (transaction_FK) REFERENCES aTransaction(PK)
    );
    )";

    const char *sql_create_aPoint = R"(
    CREATE TABLE IF NOT EXISTS aPoint (
        PK INTEGER PRIMARY KEY,
        SK_hash CHAR(32) -- 32 bytes hash
    );
    )";

    const char *sql_create_aOutput = R"(
    CREATE TABLE IF NOT EXISTS aOutput (
        PK INTEGER PRIMARY KEY,
        transaction_FK INTEGER,
        indx INTEGER,
        value BIGINT, -- 64 bit int
        length INTEGER, -- 32 bits
        script BLOB, -- 1mb
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