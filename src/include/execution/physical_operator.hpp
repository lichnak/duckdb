
#pragma once

#include <vector>

#include "catalog/catalog.hpp"

#include "common/internal_types.hpp"
#include "common/printable.hpp"

#include "common/types/data_chunk.hpp"

#include "parser/expression/abstract_expression.hpp"
#include "parser/statement/select_statement.hpp"

namespace duckdb {
class PhysicalOperator;

class PhysicalOperatorState {
  public:
	PhysicalOperatorState(PhysicalOperator *child);
	virtual ~PhysicalOperatorState() {}

	bool finished;
	DataChunk child_chunk;
	std::unique_ptr<PhysicalOperatorState> child_state;
};

class PhysicalOperator : public Printable {
  public:
	PhysicalOperator(PhysicalOperatorType type) : type(type) {}

	PhysicalOperatorType GetOperatorType() { return type; }

	virtual std::string ToString() const override;

	virtual void InitializeChunk(DataChunk &chunk) = 0;
	virtual void GetChunk(DataChunk &chunk, PhysicalOperatorState *state) = 0;

	virtual std::unique_ptr<PhysicalOperatorState> GetOperatorState() = 0;

	PhysicalOperatorType type;
	std::vector<std::unique_ptr<PhysicalOperator>> children;
};
} // namespace duckdb