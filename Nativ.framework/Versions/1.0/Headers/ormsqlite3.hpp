/* Copyright (c) 2011 Ironclad Mobile, Inc. All rights reserved */

#include "ormcore.hpp"
#include "ormstore.hpp"
#include "ormpredicate.hpp"

#include <sqlite3.h>
#include <map>

#ifndef ICM_ORMSQLITE3_HPP
#define ICM_ORMSQLITE3_HPP

namespace ORawrM
{
	class SQLiteAdapter;

	//These are unique per model. 
	struct ModelData
	{
		ModelData();
		~ModelData();

		const char * modelName;

		sqlite3_stmt * insertStatement;
		sqlite3_stmt * queryStatement;
		sqlite3_stmt * updateStatement;
		sqlite3_stmt * deleteStatement;

		std::map<std::string, sqlite3_stmt *> queryStatements;
	};

	//Table creation
	class ModelGenerator : public ArchiveAdapter<ModelGenerator>
	{
		struct CustomIndexEntry
		{
			const char * name;
			size_t order;
			bool guid;

			inline bool operator<(const CustomIndexEntry& other) const
			{
				return order < other.order;
			}
		};
	public:
		ModelGenerator(const char * name, std::stringstream * selfstream);

		const std::vector<std::string>& getModelString();

		void operator()(const char * name, GUID&, const Constraints& c);
		void operator()(const char * name, std::string&, const Constraints& c);
		void operator()(const char * name, boost::int32_t&, const Constraints& c);
		void operator()(const char * name, boost::int64_t&, const Constraints& c);
		void operator()(const char * name, double&, const Constraints& c);
		void operator()(const char * name, float&, const Constraints& c);

		template<typename T>
		void operator()(const char * name, boost::optional<T>&, const Constraints& c)
		{
			T t;
			Constraints nullable = Constraints::nullable();
			nullable.combine(c);
			(*this)(name, t, nullable);
		}

		template<typename T>
		void operator()(const char * name, T& out, const Constraints& c)
		{
			BOOST_STATIC_ASSERT(Metadata<T>::requiresLink);
			GUID temp;
			(*this)(name, temp, c);
		}

		template<typename T>
		void operator()(const char * name, std::vector<T>& models, const Constraints& c)
		{
			BOOST_STATIC_ASSERT(Metadata<T>::requiresLink);
		}
	private:
		void handleConstraints(const char * name, const Constraints& c, bool isGUID);

		std::vector<std::string> queries;

		//Map of customIndexName -> declaration Names
		std::map< std::string, std::vector<CustomIndexEntry> > customIndices;

		std::stringstream * target;
		const char * modelname;
		const char * keyName;
	};

	//This drops the indices created by the things above.
	class SQLiteIndexDropper : public ArchiveAdapter<SQLiteIndexDropper>
	{
	public:
		SQLiteIndexDropper(const char * name, SQLiteAdapter * adapter);

		template<typename T>
		void operator()(const char * name, T&, const Constraints& c)
		{
			std::stringstream query;

			if (c.isKey)
			{
				query.str("");

				query << "DROP INDEX " << modelname << "KeyIndex";
				drops.push_back(query.str());
			}

			if (c.referenceType == Constraints::REFERENCE_IS_PARENT_LINK)
			{
				query.str("");

				query << "DROP INDEX " << modelname << "ToParentKeyIndex";
				drops.push_back(query.str());
			}

			if (c.isIndex)
			{
				query.str("");

				query << "DROP INDEX " << modelname << name << "Index";
				drops.push_back(query.str());
			}

			if (c.customIndexName)
			{
				customInds.insert(c.customIndexName);
			}
		}

		template<typename U>
		void end(U& self)
		{
			typedef std::set<std::string>::iterator iterator;

			for (iterator it = customInds.begin(); it != customInds.end(); ++it)
			{
				std::stringstream query;
				query << "DROP INDEX CustomIndex" << *it << "Index";
				drops.push_back(query.str());
			}

			execute();
		}
	private:
		void execute();

		const char * modelname;
		SQLiteAdapter * adapter;
		std::vector<std::string> drops;
		std::set<std::string> customInds;
	};

	//The statement binder.
	class StatementBinder : public ArchiveAdapter<StatementBinder>
	{
	public:
		StatementBinder(sqlite3_stmt * statement, size_t index);

		void operator()(const char *, double& d, const Constraints&);
		void operator()(const char *, float& d, const Constraints&);
		void operator()(const char *, boost::int32_t&, const Constraints&);
		void operator()(const char *, boost::int64_t&, const Constraints&);
		void operator()(const char *, GUID&, const Constraints&);
		void operator()(const char *, std::string&, const Constraints&);

		template<typename T>
		MODEL_DISABLE(T) operator()(const char * name, boost::optional<T>& t, const Constraints& c) {
			if (t) {
				(*this)(name, *t, c);
			} 
			else {
				sqlite3_bind_null(statement, index);
				consumedCount = 1;
			}
		}

		template<typename T>
		MODEL_ENABLE(T) operator()(const char * n, T& value, const Constraints& c)
		{
			GUID id = value.getPrimaryKey();
			(*this)(n, id, c);
		}

		template<typename T>
		MODEL_ENABLE(T) operator()(const char *, std::vector<T>&, const Constraints&)
		{}

		size_t getConsumed() const;
	private:
		sqlite3_stmt * statement;
		size_t index;
		size_t consumedCount;
	};

		/*
		Takes a statement and an index, and
		from the values in the statement starting at the index, 
		deserializes an object.
	*/
	class QueryHelper : public ArchiveAdapter<QueryHelper>
	{
	public:
		QueryHelper(sqlite3_stmt * statement, size_t index);

		void operator()(const char *, double& d, const Constraints&);
		void operator()(const char *, float& f, const Constraints&);
		void operator()(const char *, boost::int32_t&, const Constraints&);
		void operator()(const char *, boost::int64_t&, const Constraints&);
		void operator()(const char *, GUID&, const Constraints&);
		void operator()(const char *, std::string&, const Constraints&);

		template<typename T>
		void operator()(const char * name, boost::optional<T>& t, const Constraints& c)
		{
			int type = sqlite3_column_type(statement, index);
			if (type == SQLITE_NULL)
			{
				t = boost::optional<T>();
				if (boost::is_same<T, GUID>::value)
				{
					consumed = 2;
				} else 
				{
					consumed = 1;
				}
			} else 
			{
				t = T();
				(*this)(name, *t, c);
			}
		}

		template<typename T>
		MODEL_ENABLE(T) operator()(const char * name, T& out, const Constraints& c)
		{
			GUID temp;
			(*this)(name, temp, c);
			out.setCleanPrimaryKey(temp);
		}

		size_t getConsumed() const;
	private:
		sqlite3_stmt * statement;
		size_t index;
		size_t consumed;
	};

	//Insert generation
	class InsertGenerator : public ArchiveAdapter<InsertGenerator>
	{
	public: 
		InsertGenerator(const char * name, std::stringstream * self, SQLiteAdapter& adapter, ModelData& cached);

		void operator()(const char * name, GUID&, const Constraints&);
		void operator()(const char * name, boost::optional<GUID>&, const Constraints&);

		template<typename T>
		MODEL_DISABLE(T) operator()(const char * name, T& value, const Constraints&)
		{
			*target << "," << name; 
			fieldCount++;
		}

		template<typename T>
		MODEL_ENABLE(T) operator()(const char * name, T& value, const Constraints& c)
		{
			//Single relations are modeled as GUIDs
			GUID temp;
			operator()(name, temp, c);
		}

		template<typename T>
		void operator()(const char * name, std::vector<T>& value, const Constraints&)
		{
			BOOST_STATIC_ASSERT(Metadata<T>::requiresLink);
		}

		void finalize();
		bool needsSerialize() const;
	private:
		std::stringstream * target;
		const char * tablename;
		ModelData& cache;
		SQLiteAdapter& adapter;
		size_t fieldCount;
	};

	class InsertQuery : public ArchiveAdapter<InsertQuery>
	{
	public:
		InsertQuery(SQLiteAdapter& adapter, ModelData& cache);

		template<typename T>
		MODEL_DISABLE(T) operator()(const char * name, T& value, const Constraints& c)
		{
			StatementBinder helper(cache.insertStatement, index + 1);

			helper(name, value, c);

			index += helper.getConsumed();
		}


		template<typename T>
		MODEL_ENABLE(T) operator()(const char * name, T& value, const Constraints& c)
		{
			GUID invalid = value.getPrimaryKey();
			(*this)(name, invalid, c);
		}

		template<typename T>
		void operator()(const char * name, std::vector<T>& arr, const Constraints& c)
		{
			BOOST_STATIC_ASSERT(Metadata<T>::requiresLink);
		}

		void finalize();

		template<typename U>
		void start(U& t)
		{
			targetGUID = t.getPrimaryKey();
		}
	private:
		SQLiteAdapter& adapter;
		ModelData& cache;
		ModelData * others;
		size_t index;
		GUID targetGUID;
	};

	//Query by GUID generation.
	class SQLiteQueryGenerator : public ArchiveAdapter<SQLiteQueryGenerator>
	{
	public:
		SQLiteQueryGenerator(const char * name, std::stringstream * output, SQLiteAdapter& adapter, ModelData& cached);

		//Query only works on GUID keys.
		void operator()(const char * name, GUID& guid, const Constraints& c);

		template<typename T>
		void operator()(const char *, T&, const Constraints&)
		{}

		bool needsSerialize() const;
		void finalize();
	private:
		std::stringstream * target;

		const char * tblname;
		ModelData& cache;
		SQLiteAdapter& adapter;
	};

	class SQLiteQuery : public ArchiveAdapter<SQLiteQuery>
	{
	public:
		SQLiteQuery(SQLiteAdapter& adapter, ModelData& cached);
		~SQLiteQuery();

		template<typename T>
		void operator()(const char * n, T& out, const Constraints& c)
		{
			QueryHelper q(cached.queryStatement, field);

			q(n, out, c);

			field += q.getConsumed();
		}

		template<typename T>
		void operator()(const char * n, std::vector<T>&, const Constraints& c)
		{}

		void setTarget(const GUID& t);
		void finalize(); 
		bool exists();
	private:
		GUID target;
		SQLiteAdapter& adapter;
		ModelData& cached;
		size_t field;
	};

	//Update generation.
	//Update only updates non-primary-key values.
	class UpdateGenerator : public ArchiveAdapter<UpdateGenerator>
	{
	public:
		UpdateGenerator(const char * name, std::stringstream * self, SQLiteAdapter& adapter, ModelData& cache);

		void operator()(const char * name, GUID& g, const Constraints& c);
		void operator()(const char * name, boost::optional<GUID>& g, const Constraints& c);

		template<typename T>
		MODEL_DISABLE(T) operator()(const char * name, T& value, const Constraints&)
		{
			*target << ", " << name << " = " << "?" << fieldCount;
			fieldCount++;
		}

		template<typename T>
		MODEL_ENABLE(T) operator()(const char * name, T& value, const Constraints& c)
		{
			GUID temp;
			operator()(name, temp, c);
		}

		template<typename T>
		void operator()(const char * name, std::vector<T>& value, const Constraints&)
		{
			BOOST_STATIC_ASSERT(Metadata<T>::requiresLink);
		}

		bool needsSerialize() const;
		void finalize();
	private:
		std::stringstream * target;
		const char * tblname;
		const char * primaryColumn;
		ModelData& cache;
		SQLiteAdapter& adapter;
		size_t fieldCount;
	};

	class UpdateQuery : public ArchiveAdapter<UpdateQuery>
	{
	public:
		UpdateQuery(SQLiteAdapter& adapt, ModelData& cache);

		template<typename T>
		MODEL_DISABLE(T) operator()(const char * name, T& value, const Constraints& c)
		{
			StatementBinder helper(cache.updateStatement, index);
			helper(name, value, c);
			index += helper.getConsumed();
		}

		void operator()(const char * name, boost::optional<GUID>& value, const Constraints& c);
		void operator()(const char * name, GUID& value, const Constraints& c);

		//Other links.
		template<typename T>
		MODEL_ENABLE(T) operator()(const char * name, T& value, const Constraints& c)
		{
			StatementBinder helper(cache.updateStatement, index);
			helper(name, value, c);
			index += helper.getConsumed();
		}

		template<typename T>
		void operator()(const char * name, std::vector<T>& arr, const Constraints& c)
		{
			BOOST_STATIC_ASSERT(Metadata<T>::requiresLink);
		}

		bool finalize();

		template<typename U>
		void start(U& t)
		{
			targetGUID = t.getPrimaryKey();
		}
	private:
		ModelData& cache;
		SQLiteAdapter& adapter;

		size_t index;
		GUID targetGUID;
	};

	class DeleteGenerator : public ArchiveAdapter<DeleteGenerator>
	{
	public:
		DeleteGenerator(const char * name, std::stringstream * out, SQLiteAdapter& adapter, ModelData& cached);

		void operator()(const char * name, GUID& guid, const Constraints& cons);

		template<typename T>
		void operator()(const char * name, T&, const Constraints& c)
		{}

		void finalize();
		bool needsSerialize() const; 
	private:
		std::stringstream * target;
		const char * tablename;
		ModelData& cache;
		SQLiteAdapter& adapter;
	};

	class DeleteQuery : public ArchiveAdapter<DeleteQuery>
	{
	public:
		DeleteQuery(SQLiteAdapter& adapter, ModelData& cached);

		void setTarget(const GUID& g);

		void operator()(const char *, GUID& g, const Constraints& c);

		template<typename T>
		void operator()(const char *, T&, const Constraints& c)
		{}
		
		void finalize();
	private:
		SQLiteAdapter& adapter;
		ModelData& cache;
		GUID target;
	};

	class SQLiteMultipleQuery : public ArchiveAdapter<SQLiteMultipleQuery>
	{
	public:
		SQLiteMultipleQuery(sqlite3_stmt * steppedStatement);
		SQLiteMultipleQuery();

		template<typename T>
		void operator()(const char * n, T& out, const Constraints& c)
		{
			QueryHelper q(statement, field);
			q(n, out, c);
			field += q.getConsumed();
		}

		template<typename T>
		void operator()(const char * n, std::vector<T>&, const Constraints& c)
		{}
	private:
		sqlite3_stmt * statement;
		size_t field;
	};

	class SQLiteArbitraryQuery : public ArchiveAdapter<SQLiteArbitraryQuery>
	{
	public:
		SQLiteArbitraryQuery(const char * q, SQLiteAdapter& adapter);
		~SQLiteArbitraryQuery();

		bool step(SQLiteMultipleQuery& out);
	private:
		sqlite3_stmt * statement;
		SQLiteAdapter& adapter;
	};

	class SQLiteKeyQuery 
		: public ArchiveAdapter<SQLiteKeyQuery> 
		, public SelectGenerator<SQLiteKeyQuery>
	{
	public:
		SQLiteKeyQuery(const KeyPredicate& preds, const char * pkeyName, const char * tblname, SQLiteAdapter& adapter, ModelData& cache, bool generateCount = false, GetBridgeRelationship * bt = NULL, bool isDelete = false);
		~SQLiteKeyQuery();

		template<typename T>
		void operator()(const char *, T&, const Constraints&)
		{}

		bool step(SQLiteMultipleQuery& out);
		size_t count();

		//Generation stuff.
		void column(std::ostream& out, const char * tblName, const char * column, bool dist);
		void join(std::ostream& out, const GetBridgeRelationship * bridge, const char * bta, const char * mta, const char * sta);
		void from(std::ostream& out, const char * tableName, const char * alias);
		void startWhere(std::ostream& out);
		void guids(std::ostream& out, const char * tableName, const char * columnName, const std::vector<ORawrM::GUID>& guids, bool requiresAndAfter);
		void guid(std::ostream& out, const char * A, const char * colA, const char * B, const char * colB);
		void guid(std::ostream& out, const char * A, const char * colA, const GUID& guid);
		void where(std::ostream& out, const char * tableName, const std::vector<ColumnPredicate>& preds);
		void endWhere(std::ostream& out);
		void startOrder(std::ostream& out);
		void order(std::ostream& out, const char * tableName, const char * orderingColumn, bool isDescending, size_t index);
		void endOrder(std::ostream& out);
		void limit(std::ostream&, size_t l);
		void offset(std::ostream&, size_t o);
		void beginDeleteQuery(std::ostream&);
		void beginQuery(std::ostream&);
		void endQuery(std::ostream&);
		void count(std::ostream& out, const char * tableName, const char * optionalColumn, bool distinct);

		sqlite3_stmt * statement;
		sqlite3_stmt * countStmt;
		SQLiteAdapter& adapter;
	};

	class SQLiteAdapter;
	struct SQLiteSchemaMigration : public ArchiveAdapter<SQLiteSchemaMigration>
	{
		SQLiteSchemaMigration(SQLiteAdapter *);

		size_t getSchemaVersion(size_t current);
		void setSchemaVersion(size_t);

		void rename(const std::string& from, const std::string& to);
		void remove(const std::string& tbl);
		
		void startPopulate(const std::string& from, const std::string& to);
		
		template<typename F, typename T>
		void renameColumn(const std::string& fromdecl, const std::string& todecl)
		{
			columns.push_back(std::make_pair(fromdecl, todecl));
		}

		void endPopulate();
	
		std::string fromTable;
		std::string toTable;
		std::vector< std::pair<std::string, std::string> > columns;

		SQLiteAdapter * adapter;
	};

	struct SQLiteOpenParameters
	{
		explicit SQLiteOpenParameters(const std::string& databaseName);
		std::string database;
	};

	struct SQLiteColumnExists : public ArchiveAdapter<SQLiteColumnExists>
	{
		SQLiteColumnExists(const char * name);

		template<typename T>
		void operator()(const char * n, T&, const Constraints&)
		{
			if (strcmp(n, name) == 0)
			{
				result = true;
			}
		}

		void operator()(const char * n, GUID&, const Constraints&);

		const char * name;
		bool result;
	};

	//Adapter
	class SQLiteAdapter : public HasLogger
	{
	public:
		typedef ::ORawrM::SQLiteOpenParameters		OpenParameters;
		typedef ::ORawrM::ModelData					ModelCacheEntry;

		typedef ::ORawrM::ModelGenerator			ModelGenerator;
		
		typedef ::ORawrM::InsertGenerator			InsertGenerator;
		typedef ::ORawrM::InsertQuery				InsertQuery;
		
		typedef ::ORawrM::SQLiteQueryGenerator		QueryGenerator;
		typedef ::ORawrM::SQLiteQuery				Query;
		
		typedef ::ORawrM::UpdateGenerator			UpdateGenerator;
		typedef ::ORawrM::UpdateQuery				UpdateQuery;
		
		typedef ::ORawrM::DeleteGenerator			DeleteGenerator;
		typedef ::ORawrM::DeleteQuery				DeleteQuery;
		
		typedef ::ORawrM::SQLiteMultipleQuery		KeyQueryMultiple;
		typedef ::ORawrM::SQLiteArbitraryQuery		ArbitraryQuery;
		typedef ::ORawrM::SQLiteKeyQuery			KeyQuery;

		typedef ::ORawrM::SQLiteColumnExists		ColumnExists;

		typedef ::ORawrM::SQLiteSchemaMigration		SchemaMigration;
		typedef ::ORawrM::SQLiteIndexDropper		DropIndices;
		
		SQLiteAdapter();
		~SQLiteAdapter();

		void connect(const SQLiteOpenParameters& params);
		void associateCaches(ModelCacheEntry * base, size_t length);

		std::string execute(const char * name);

		void startTransaction();
		void commitTransaction();
		void rollbackTransaction();

		void afterRegistration();
		void updateSchema();

		sqlite3 * getDatabase();
		sqlite3_stmt * compile(const std::string& stmt);
		static std::string sanitize(const char * input);
		
		std::string formatMessage(size_t code);
		bool failed(size_t ec) const;

		//Returns true iff nothing was removed from the input.
		template<typename T>
		bool sanitizePredicates(std::vector<ColumnPredicate>& preds)
		{
			return ORawrM::sanitizePredicate<T, ColumnExists>(preds, "", getLogger(), sanitize);
		}
	private:
		sqlite3 * database;
		size_t transactionDepth;
	};
}
#endif

