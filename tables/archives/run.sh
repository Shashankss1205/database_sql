#Before running this file write
# chmod +x ./run.sh

# Creating archieve tables
g++ archive_tables_create.cpp -lsqlite3 -o archive_tables_create
./archive_tables_create

# Populating archieve tables
g++ archive_db_populate.cpp -lsqlite3 -o archive_db_populate
./archive_db_populate

# Querying archieve tables
g++ archive_db_query.cpp -lsqlite3 -o archive_db_query
./archive_db_query

rm ./archive_tables_create ./archive_db_populate ./archive_db_query

