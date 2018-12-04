//===----------------------------------------------------------------------===//
//
//                         DuckDB
//
// parser/expression.hpp
//
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include <memory>
#include <stack>
#include <vector>

#include "common/common.hpp"
#include "common/printable.hpp"
#include "common/types/statistics.hpp"

namespace duckdb {
class SQLNodeVisitor;
class AggregateExpression;

//!  Expression class is a base class that can represent any expression
//!  part of a SQL statement.
/*!
 The Expression class is a base class that can represent any expression
 part of a SQL statement. This is, for example, a column reference in a SELECT
 clause, but also operators, aggregates or filters.

 In the execution engine, an Expression always returns a single Vector
 of the type specified by return_type. It can take an arbitrary amount of
 Vectors as input (but in most cases the amount of input vectors is 0-2).
 */
class Expression : public Printable {
  public:
	//! Create an Expression
	Expression(ExpressionType type) : type(type), stats(*this) {
	}
	//! Create an Expression with zero, one or two children with the
	//! specified return type
	Expression(ExpressionType type, TypeId return_type,
	           std::unique_ptr<Expression> left = nullptr,
	           std::unique_ptr<Expression> right = nullptr)
	    : type(type), return_type(return_type), stats(*this) {
		if (left)
			AddChild(std::move(left));
		if (right)
			AddChild(std::move(right));
	}

	virtual std::unique_ptr<Expression> Accept(SQLNodeVisitor *) = 0;
	virtual void AcceptChildren(SQLNodeVisitor *v);

	//! Resolves the type for this expression based on its children
	virtual void ResolveType() {
		for (auto &child : children) {
			child->ResolveType();
		}
	}

	//! Add a child node to the Expression. Note that the order of
	//! adding children is important in most cases
	void AddChild(std::unique_ptr<Expression> child) {
		children.push_back(std::move(child));
	}

	//! Return a list of the deepest aggregates that are present in the
	//! Expression (if any).
	/*!
	 This function is used by the execution engine to figure out which
	 aggregates/groupings have to be computed.

	 Examples:

	 (1) SELECT SUM(a) + SUM(b) FROM table; (Two aggregates, SUM(a) and SUM(b))

	 (2) SELECT COUNT(SUM(a)) FROM table; (One aggregate, SUM(a))
	 */
	virtual void GetAggregates(std::vector<AggregateExpression *> &expressions);
	//! Returns true if this Expression is an aggregate or not.
	/*!
	 Examples:

	 (1) SUM(a) + 1 -- True

	 (2) a + 1 -- False
	 */
	virtual bool IsAggregate();

	//! Returns true if the query contains a subquery
	virtual bool HasSubquery();

	// returns true if expression does not contain a group ref or col ref or
	// parameter
	virtual bool IsScalar();

	//! Returns the type of the expression
	ExpressionType GetExpressionType() {
		return type;
	}

	virtual bool Equals(const Expression *other);

	bool operator==(const Expression &rhs) {
		return this->Equals(&rhs);
	}

	std::string ToString() const override;

	virtual std::string GetName() {
		return !alias.empty() ? alias : "Unknown";
	}
	virtual ExpressionClass GetExpressionClass() = 0;

	//! Create a copy of this expression
	virtual std::unique_ptr<Expression> Copy() = 0;

	//! Serializes an Expression to a stand-alone binary blob
	virtual void Serialize(Serializer &serializer);
	//! Deserializes a blob back into an Expression [CAN THROW:
	//! SerializationException]
	static std::unique_ptr<Expression> Deserialize(Deserializer &source);

	//! Clears the statistics of this expression and all child expressions
	void ClearStatistics() {
		stats.has_stats = false;
		for (auto &child : children) {
			child->ClearStatistics();
		}
	}

	//! Type of the expression
	ExpressionType type;
	//! Return type of the expression. This must be known in the execution
	//! engine
	TypeId return_type = TypeId::INVALID;

	//! The statistics of the current expression in the plan
	ExpressionStatistics stats;

	//! The alias of the expression, used in the SELECT clause (e.g. SELECT x +
	//! 1 AS f)
	std::string alias;

	//! A list of children of the expression
	std::vector<std::unique_ptr<Expression>> children;

  protected:
	//! Copy base Expression properties from another expression to this one,
	//! used in Copy method
	void CopyProperties(Expression &other) {
		type = other.type;
		return_type = other.return_type;
		stats = other.stats;
		alias = other.alias;
	}
	void CopyChildren(Expression &other) {
		for (auto &child : other.children) {
			assert(child);
			children.push_back(child->Copy());
		}
	}
};

//! Expression deserialize information
struct ExpressionDeserializeInformation {
	ExpressionType type;
	TypeId return_type;
	std::vector<std::unique_ptr<Expression>> children;
};

} // namespace duckdb
