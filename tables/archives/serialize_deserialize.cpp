// Pseudo-code for inserting a header into SQL
void serialize(const record& header) {
    std::string query = "INSERT INTO block_headers (milestone, parent_fk, version, timestamp, bits, nonce, merkle_root, context_flags, context_height, context_mtp) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";
    
    // Prepare SQL statement (depends on your SQL library, e.g., SQLite, MySQL)
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr);

    // Bind values to SQL statement
    sqlite3_bind_int(stmt, 1, header.milestone);
    sqlite3_bind_int64(stmt, 2, header.parent_fk);
    sqlite3_bind_int(stmt, 3, header.version);
    sqlite3_bind_int(stmt, 4, header.timestamp);
    sqlite3_bind_int(stmt, 5, header.bits);
    sqlite3_bind_int(stmt, 6, header.nonce);
    sqlite3_bind_blob(stmt, 7, header.merkle_root.data(), header.merkle_root.size(), SQLITE_STATIC);
    sqlite3_bind_int(stmt, 8, header.ctx.flags);
    sqlite3_bind_int(stmt, 9, header.ctx.height);
    sqlite3_bind_int(stmt, 10, header.ctx.mtp);

    // Execute the statement
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

// Pseudo-code for loading a header from SQL
record deserialize(int header_id) {
    std::string query = "SELECT milestone, parent_fk, version, timestamp, bits, nonce, merkle_root, context_flags, context_height, context_mtp FROM block_headers WHERE id = ?";
    
    // Prepare SQL statement
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, header_id);

    record header;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        header.milestone = sqlite3_column_int(stmt, 0);
        header.parent_fk = sqlite3_column_int64(stmt, 1);
        header.version = sqlite3_column_int(stmt, 2);
        header.timestamp = sqlite3_column_int(stmt, 3);
        header.bits = sqlite3_column_int(stmt, 4);
        header.nonce = sqlite3_column_int(stmt, 5);
        header.merkle_root = std::vector<uint8_t>((uint8_t*)sqlite3_column_blob(stmt, 6), (uint8_t*)sqlite3_column_blob(stmt, 6) + sqlite3_column_bytes(stmt, 6));
        header.ctx.flags = sqlite3_column_int(stmt, 7);
        header.ctx.height = sqlite3_column_int(stmt, 8);
        header.ctx.mtp = sqlite3_column_int(stmt, 9);
    }

    sqlite3_finalize(stmt);
    return header;
}
