#include <iostream>
#include <sqlite3.h>
#include"utilities.hpp"

void insertTestData(sqlite3 *db) {
    // Insert test data into aHeader table
    executeSQL(db, "INSERT INTO aHeader (PK, SK_hash, flags, height, mtp, header_FK, version, time, bits, nonce, merkle_root) VALUES (1, 'header1_hash', 0, 100, 123456, NULL, 1, 1624809600, 12345, 6789, 'merkle_root1');");
    executeSQL(db, "INSERT INTO aHeader (PK, SK_hash, flags, height, mtp, header_FK, version, time, bits, nonce, merkle_root) VALUES (2, 'header2_hash', 0, 101, 123457, NULL, 1, 1624813200, 54321, 9876, 'merkle_root2');");

    // Insert test data into aTxs table
    executeSQL(db, "INSERT INTO aTxs (SK_header_FK, transaction_FK) VALUES (1, 1);");
    executeSQL(db, "INSERT INTO aTxs (SK_header_FK, transaction_FK) VALUES (2, 2);");

    // Insert test data into aTransaction table
    executeSQL(db, "INSERT INTO aTransaction (PK, SK_hash, coinbase, size, weight, locktime, version, input_count, inputs_FK, output_count, outputs_FK) VALUES (1, 'txn1_hash', 'coinbase1', 200, 300, 0, 1, 1, 1, 1, 1);");
    executeSQL(db, "INSERT INTO aTransaction (PK, SK_hash, coinbase, size, weight, locktime, version, input_count, inputs_FK, output_count, outputs_FK) VALUES (2, 'txn2_hash', 'coinbase2', 250, 350, 0, 1, 1, 2, 1, 2);");

    // Insert test data into aInput table
    executeSQL(db, "INSERT INTO aInput (PK, SK_index, SK_point_FK, transaction_FK, indx, sequence, length, script, count, witness) VALUES (1, 1, 1, 1, 0, 4294967295, 100, 'input_script1', 1, 'witness_data1');");
    executeSQL(db, "INSERT INTO aInput (PK, SK_index, SK_point_FK, transaction_FK, indx, sequence, length, script, count, witness) VALUES (2, 2, 2, 2, 0, 4294967294, 150, 'input_script2', 1, 'witness_data2');");

    // Insert test data into aPoint table
    executeSQL(db, "INSERT INTO aPoint (PK, SK_hash) VALUES (1, 'point1_hash');");
    executeSQL(db, "INSERT INTO aPoint (PK, SK_hash) VALUES (2, 'point2_hash');");

    // Insert test data into aOutput table
    executeSQL(db, "INSERT INTO aOutput (PK, transaction_FK, indx, value, length, script) VALUES (1, 1, 0, 50000, 200, 'output_script1');");
    executeSQL(db, "INSERT INTO aOutput (PK, transaction_FK, indx, value, length, script) VALUES (2, 2, 0, 75000, 250, 'output_script2');");
}

int main(){
    int rc = rc_to_database("archive.db");
    sqlite3* db = open_database("archive.db");

    // Insert test data into tables
    insertTestData(db);
}