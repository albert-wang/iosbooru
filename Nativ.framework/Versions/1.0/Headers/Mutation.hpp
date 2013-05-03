/* Copyright (c) 2011 Ironclad Mobile, Inc. All rights reserved */

#include <string>
#include <boost/cstdint.hpp>
#include <boost/optional.hpp>
#include <boost/lexical_cast.hpp>
#include <algorithm>
#include "ormbacking.hpp"
#include "ormcore.hpp"

#ifndef ICM_MUTATION_HPP
#define ICM_MUTATION_HPP

//Starting Generated Model: Mutation
//Backing: ORawrM::NullBacking
class Mutation
{
	static const char * ORawrMGenerationKey;

	ORawrM::GUID pid;
	std::string tableName;
	std::string field;
	boost::optional<std::string> data;
	std::string operation;
	ORawrM::GUID target;
	ORawrM::NullBacking reserved_backing;
	friend class ORawrM::Access;
public:
	static const bool hasGeneratedKey = false;
	inline void regenerateKey() {}

	inline Mutation(){}
	enum 
	{
		RelationCount,
	};

	inline static std::string getRelationName() { return "<INVALID>"; }
	inline static size_t getRelationID(const std::string&) { return 0; }

	inline void deleted() { reserved_backing.deleted(*this); }
	inline ORawrM::NullBacking& getBacking() { return reserved_backing; }
	template<typename T> inline void setBackingStore(ORawrM::Datastore<T> * ds)
	{
		reserved_backing.setStore(*this, ds);
	}
	inline void setPid(const ORawrM::GUID& value)
	{
		reserved_backing.mutationReplace(*this, "pid", pid, value);
		pid = value;
	}

	inline void setCleanPid(const ORawrM::GUID& value)
	{
		pid = value;
	}

	inline const ORawrM::GUID& getPid() const
	{
		return pid;
	}

	inline void setTableName(const std::string& value)
	{
		reserved_backing.mutationReplace(*this, "tableName", tableName, value);
		tableName = value;
	}

	inline void setCleanTableName(const std::string& value)
	{
		tableName = value;
	}

	inline const std::string& getTableName() const
	{
		return tableName;
	}

	inline void setField(const std::string& value)
	{
		reserved_backing.mutationReplace(*this, "field", field, value);
		field = value;
	}

	inline void setCleanField(const std::string& value)
	{
		field = value;
	}

	inline const std::string& getField() const
	{
		return field;
	}

	inline void setData(const boost::optional<std::string>& value)
	{
		reserved_backing.mutationReplace(*this, "data", data, value);
		data = value;
	}

	inline void setCleanData(const boost::optional<std::string>& value)
	{
		data = value;
	}

	inline const boost::optional<std::string>& getData() const
	{
		return data;
	}

	inline void setOperation(const std::string& value)
	{
		reserved_backing.mutationReplace(*this, "operation", operation, value);
		operation = value;
	}

	inline void setCleanOperation(const std::string& value)
	{
		operation = value;
	}

	inline const std::string& getOperation() const
	{
		return operation;
	}

	inline void setTarget(const ORawrM::GUID& value)
	{
		reserved_backing.mutationReplace(*this, "target", target, value);
		target = value;
	}

	inline void setCleanTarget(const ORawrM::GUID& value)
	{
		target = value;
	}

	inline const ORawrM::GUID& getTarget() const
	{
		return target;
	}

	template<typename Archive>
	void serialize(Archive& arch)
	{
		arch.start(*this);

		ORawrM::Constraints constraint_for_id = ORawrM::Constraints::none();
		constraint_for_id.combine(ORawrM::Constraints::key());
		arch & ::ORawrM::makeEntry("pid", pid, constraint_for_id);

		ORawrM::Constraints noConstraint = ORawrM::Constraints::none();
		arch & ::ORawrM::makeEntry("tableName", tableName, noConstraint);
		arch & ::ORawrM::makeEntry("field", field, noConstraint);
		arch & ::ORawrM::makeEntry("data", data, noConstraint);
		arch & ::ORawrM::makeEntry("operation", operation, noConstraint);
		arch & ::ORawrM::makeEntry("target", target, noConstraint);

		arch.end(*this);
	}

	inline const ORawrM::GUID& getPrimaryKey() const { return pid; }

	inline void setPrimaryKey(const ORawrM::GUID& value) { setPid(value); }

	inline void setCleanPrimaryKey(const ORawrM::GUID& value) { setCleanPid(value); }
};
REGISTER_MODEL(Mutation);

#endif
