#include <iostream>
#include <sqlite3.h>
#include <vector>
#include <string>
#include "archive_db_query.hpp"

int main() {
    libbitcoin::database::Database db("archive.db");

    // Sample usage
    std::string tx_hash = "txn1_hash";
    if (db.is_tx(tx_hash)) {
        std::cout << "Transaction exists in the database." << std::endl;
    } else{
        std::cout<<"Not exists." << std::endl;
    }

    return 0;
}
