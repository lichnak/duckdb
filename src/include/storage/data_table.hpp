
#pragma once

#include <vector>

#include "common/types/data_chunk.hpp"

#include "storage/data_column.hpp"

namespace duckdb {
class StorageManager;

class DataTable {
  public:
	DataTable(StorageManager &storage, TableCatalogEntry &table)
	    : storage(storage), size(0), table(table) {}

	void AddColumn(ColumnCatalogEntry &column);

	void AddData(DataChunk &chunk);

	size_t size;
	StorageManager &storage;
	std::vector<std::unique_ptr<DataColumn>> columns;
	TableCatalogEntry &table;
};
} // namespace duckdb