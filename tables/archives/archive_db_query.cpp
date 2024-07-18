#include <iostream>
#include <sqlite3.h>
#include <vector>
#include <string>
#include "archive_db_query.hpp"

int main() {
    libbitcoin::database::Database db("archive.db");

    // Test for is_tx method
    std::string tx_hash = "txn1_hash";
    if (db.is_tx(tx_hash)) {
        std::cout << "Transaction exists in the database." << std::endl;
    } else {
        std::cout << "Transaction does not exist in the database." << std::endl;
    }

    // Test for is_header method
    std::string header_hash = "header1_hash";
    if (db.is_header(header_hash)) {
        std::cout << "Header exists in the database." << std::endl;
    } else {
        std::cout << "Header does not exist in the database." << std::endl;
    }

    // Test for is_coinbase method
    std::string coinbase_link = "coinbase1";
    if (db.is_coinbase(coinbase_link)) {
        std::cout << "Coinbase exists in the database." << std::endl;
    } else {
        std::cout << "Coinbase does not exist in the database." << std::endl;
    }

    // Test for get_tx_keys method
    std::string header_link = "header1_link";
    std::vector<std::string> tx_keys = db.get_tx_keys(header_link);
    std::cout << "Transaction keys for header " << header_link << ":";
    for (const auto& key : tx_keys) {
        std::cout << " " << key;
    }
    std::cout << std::endl;

    // Test for get_header_key method
    std::string header_key = db.get_header_key(header_link);
    if (!header_key.empty()) {
        std::cout << "Header key for link " << header_link << ": " << header_key << std::endl;
    } else {
        std::cout << "No header key found for link " << header_link << std::endl;
    }

    // Test for get_height method
    size_t height;
    if (db.get_height(height, header_hash)) {
        std::cout << "Height of header " << header_hash << ": " << height << std::endl;
    } else {
        std::cout << "No height found for header " << header_hash << std::endl;
    }

    // Test for get_value method
    std::string output_link = "output1_link";
    uint64_t value;
    if (db.get_value(value, output_link)) {
        std::cout << "Value of output " << output_link << ": " << value << std::endl;
    } else {
        std::cout << "No value found for output " << output_link << std::endl;
    }

    return 0;
}
