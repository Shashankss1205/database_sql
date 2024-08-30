#include <bitcoin/system.hpp>

void store_block_header_in_sql(const libbitcoin::system::chain::block& block) {
    // Extract validated block header data
    int version = block.header().version();
    libbitcoin::system::hash_digest previous_block_hash = block.header().previous_block_hash();
    libbitcoin::system::hash_digest merkle_root = block.header().merkle();
    uint32_t timestamp = block.header().timestamp();
    uint32_t bits = block.header().bits();
    uint32_t nonce = block.header().nonce();

    // Convert hashes to their string representation for storage in SQL
    std::string previous_block_hash_str = libbitcoin::system::encode_base16(previous_block_hash);
    std::string merkle_root_str = libbitcoin::system::encode_base16(merkle_root);

    // Example: Store these values in a SQL database
    std::string sql_query = "INSERT INTO headers (version, previous_block_hash, merkle_root, timestamp, bits, nonce) "
                            "VALUES (" + std::to_string(version) + ", '" + previous_block_hash_str + "', '"
                            + merkle_root_str + "', " + std::to_string(timestamp) + ", "
                            + std::to_string(bits) + ", " + std::to_string(nonce) + ");";
    
    // Execute the SQL query using your preferred database library
    execute_sql_query(sql_query);
}
