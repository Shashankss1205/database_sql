#include <iostream>
#include <sqlite3.h>
#include <vector>
#include <string>
#include "utilities.hpp"

namespace libbitcoin {
namespace database {

class Database {
public:
    Database(const std::string& db_path) {
        db = open_database(db_path.c_str());
    }

    ~Database() {
        sqlite3_close(db);
    }
    
    // Archival (surrogate-keyed).

    bool is_header(const std::string& key) const {
        return exists("aHeader", "SK_hash", key);
    }

    bool is_block(const std::string& key) const {
        return exists("aBlock", "SK_header_FK", key);
    }

    bool is_tx(const std::string& key) const {
        return exists("aTransaction", "SK_hash", key);
    }

    bool is_coinbase(const std::string& link) const {
        return exists("aTransaction", "coinbase", link);
    }

    bool is_milestone(const std::string& link) const {
        std::string sql = "SELECT milestone FROM aHeader WHERE SK_hash = ?";
        sqlite3_stmt* stmt;
        bool result = false;

        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, link.c_str(), -1, SQLITE_STATIC);
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                result = sqlite3_column_int(stmt, 0) > 0;
            }
            sqlite3_finalize(stmt);
        }

        return result;
    }

    bool is_associated(const std::string& link) const {
        std::string sql = "SELECT associated FROM aHeader WHERE SK_hash = ?";
        sqlite3_stmt* stmt;
        bool result = false;

        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, link.c_str(), -1, SQLITE_STATIC);
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                result = sqlite3_column_int(stmt, 0) > 0;
            }
            sqlite3_finalize(stmt);
        }

        return result;
    }

    // get methods.
    std::vector<std::string> get_tx_keys(const std::string& header_link) const {
        std::vector<std::string> tx_keys;
        std::string sql = "SELECT transaction_FK FROM aTxs WHERE SK_header_FK = ?";
        sqlite3_stmt* stmt;

        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, header_link.c_str(), -1, SQLITE_STATIC);
            while (sqlite3_step(stmt) == SQLITE_ROW) {
                tx_keys.push_back(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));
            }
            sqlite3_finalize(stmt);
        }

        return tx_keys;
    }

    std::string get_header_key(const std::string& link) const {
        return get_column_value("aHeader", "SK_hash", "PK", link);
    }

    std::string get_point_key(const std::string& link) const {
        return get_column_value("aPoint", "SK_hash", "PK", link);
    }

    std::string get_tx_key(const std::string& link) const {
        return get_column_value("aTransaction", "SK_hash", "PK", link);
    }

    bool get_height(size_t& out, const std::string& key) const {
        return get_height_from_column(out, "aHeader", "height", "SK_hash", key);
    }

    bool get_tx_height(size_t& out, const std::string& link) const {
        std::string sql = "SELECT height FROM aBlock WHERE SK_header_FK = (SELECT SK_header_FK FROM aTransaction WHERE SK_hash = ?)";
        sqlite3_stmt* stmt;
        bool result = false;

        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, link.c_str(), -1, SQLITE_STATIC);
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                out = sqlite3_column_int(stmt, 0);
                result = true;
            }
            sqlite3_finalize(stmt);
        }

        return result;
    }
    bool get_tx_position(size_t& out, const std::string& link) const {
        std::string sql = "SELECT position FROM aTransaction WHERE SK_hash = ?";
        sqlite3_stmt* stmt;
        bool result = false;

        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, link.c_str(), -1, SQLITE_STATIC);
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                out = sqlite3_column_int(stmt, 0);
                result = true;
            }
            sqlite3_finalize(stmt);
        }

        return result;
    }

    bool get_value(uint64_t& out, const std::string& link) const {
        return get_value_from_column(out, "aOutput", "value", "PK", link);
    }

    // Populate with meta-data.

    bool Database::populate_with_metadata(const input& input, const tx_link& link) const NOEXCEPT
    {
        if (input.point().is_null()) {
            std::cerr << "Error: Input point is null." << std::endl;
            return false;
        }

        if (input.prevout) {
            return true;
        }

        input.prevout = get_output(input.point());
        if (is_null(input.prevout)) {
            return false;
        }

        // Query the database for transaction and block data
        const auto tx = to_tx(input.point().hash());
        if (tx.is_terminal()) {
            return false;
        }

        const auto block = to_block(tx);
        if (block.is_terminal()) {
            return false;
        }

        context ctx{};
        if (!get_context(ctx, block)) {
            return false;
        }

        input.metadata.coinbase = is_coinbase(tx);
        input.metadata.spent = is_spent_prevout(input.point(), link);
        input.metadata.median_time_past = ctx.mtp;
        input.metadata.height = ctx.height;
        return true;
    }

    bool Database::populate_with_metadata(const transaction& tx, const tx_link& link) const NOEXCEPT
    {
        if (tx.is_coinbase()) {
            return false;  // Coinbase transactions are handled separately
        }

        bool result = true;
        const auto& ins = *tx.inputs_ptr();

        for (const auto& in : ins) {
            result &= populate_with_metadata(*in, link);
        }

        return result;
    }

    bool Database::populate_with_metadata(const transaction& tx) const NOEXCEPT
    {
        if (!tx.is_coinbase()) {
            std::cerr << "Error: Not a coinbase transaction." << std::endl;
            return false;
        }

        // A coinbase transaction is allowed only one input
        const auto& input = *tx.inputs_ptr()->front();

        const auto ec = unspent_duplicates(tx);
        if (ec == error::integrity) {
            return false;
        }

        input.metadata.coinbase = false;
        input.metadata.spent = (ec != error::unspent_coinbase_collision);
        input.metadata.median_time_past = max_uint32;
        input.metadata.height = zero;
        return true;
    }

    bool Database::populate_with_metadata(const block& block) const NOEXCEPT
    {
        const auto& txs = *block.transactions_ptr();
        if (txs.empty()) {
            return false;
        }

        bool result = true;
        const auto coinbase = populate_with_metadata(txs.front());
        for (auto it = std::next(txs.begin()); it != txs.end(); ++it) {
            const auto& tx = *it;
            const auto link = to_tx(tx.get_hash());
            result &= !link.is_terminal();

            const auto& ins = *tx->inputs_ptr();
            for (const auto& in : ins) {
                result &= populate_with_metadata(*in, link);
            }
        }

        return result;
    }

private:
    sqlite3* db;

    bool exists(const std::string& table, const std::string& column, const std::string& value) const {
        std::string sql = "SELECT COUNT(*) FROM " + table + " WHERE " + column + " = ?";
        sqlite3_stmt* stmt;
        bool result = false;

        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, value.c_str(), -1, SQLITE_STATIC);
            if (sqlite3_step(stmt) == SQLITE_ROW && sqlite3_column_int(stmt, 0) > 0) {
                result = true;
            }
            sqlite3_finalize(stmt);
        }

        return result;
    }

    std::string get_column_value(const std::string& table, const std::string& column, const std::string& key_column, const std::string& key_value) const {
        std::string sql = "SELECT " + column + " FROM " + table + " WHERE " + key_column + " = ?";
        sqlite3_stmt* stmt;
        std::string result;

        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, key_value.c_str(), -1, SQLITE_STATIC);
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                result = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            }
            sqlite3_finalize(stmt);
        }

        return result;
    }

    bool get_height_from_column(size_t& out, const std::string& table, const std::string& height_column, const std::string& key_column, const std::string& key_value) const {
        std::string sql = "SELECT " + height_column + " FROM " + table + " WHERE " + key_column + " = ?";
        sqlite3_stmt* stmt;
        bool result = false;

        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, key_value.c_str(), -1, SQLITE_STATIC);
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                out = sqlite3_column_int(stmt, 0);
                result = true;
            }
            sqlite3_finalize(stmt);
        }

        return result;
    }

    bool get_value_from_column(uint64_t& out, const std::string& table, const std::string& value_column, const std::string& key_column, const std::string& key_value) const {
        std::string sql = "SELECT " + value_column + " FROM " + table + " WHERE " + key_column + " = ?";
        sqlite3_stmt* stmt;
        bool result = false;

        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, key_value.c_str(), -1, SQLITE_STATIC);
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                out = sqlite3_column_int64(stmt, 0);
                result = true;
            }
            sqlite3_finalize(stmt);
        }

        return result;
    }
};

} // namespace database
} // namespace libbitcoin
