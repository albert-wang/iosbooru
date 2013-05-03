/* Copyright (c) 2011 Ironclad Mobile, Inc. All rights reserved */

#include "ormcore.hpp"
#include "ormstore.hpp"
#include "ormpredicate.hpp"
#include "ormexception.hpp"

#ifdef USING_POSTGRES 
#	include <libpq-fe.h>
#endif

#include <map>

#ifndef ICM_ORMPOSTGRES_HPP
#define ICM_ORMPOSTGRES_HPP

#ifdef USING_POSTGRES
namespace ORawrM
{
	class PostgresAdapter;	 
}

//All the postgres specific stuff is in namespace PG.
namespace ORawrM { namespace PG
{
	struct Statement
	{
		Statement();

		std::string name;
		bool initalized;
		size_t parameters;
	};

	struct ModelData
	{
		ModelData();
		   
		const char * modelName;

		Statement insertStatement;
		Statement selectStatement;
		Statement updateStatement;
		Statement deleteStatement;

		std::map<std::string, Statement> keyQueries;
	};

	struct ModelGenerator : public ArchiveAdapter<ModelGenerator>
	{
		struct CustomIndexEntry
		{
			const char * name;
			size_t order;

			inline bool operator<(const CustomIndexEntry& other) const
			{
				return order < other.order;
			}
		};
	public:
		ModelGenerator(const char * name, std::stringstream * selfstream);

		const std::vector<std::string>& getModelString();

		void operator()(const char *, std::string&, const Constraints&);

		void operator()(const char *, boost::int32_t&, const Constraints&);
		void operator()(const char *, boost::int64_t&, const Constraints&);
		void operator()(const char *, float&, const Constraints&);
		void operator()(const char *, double&, const Constraints&);
		void operator()(const char *, GUID&, const Constraints&);

		template<typename T>
		void operator()(const char * name, boost::optional<T>& v, const Constraints& c)
		{
			Constraints cons = Constraints::nullable();
			cons.combine(c);
			
			T temp;
			(*this)(name, temp, cons);
		}

		template<typename T>
		MODEL_ENABLE(T) operator()(const char * name, T&, const Constraints& c)
		{
			GUID g;
			(*this)(name, g, c);
		}

		template<typename T>
		MODEL_ENABLE(T) operator()(const char *, std::vector<T>&, const Constraints&)
		{}

	private:
		void handleConstraints(const char * name, const Constraints& c);
		std::vector<std::string> queries;

		//Map of customIndexName -> declaration Names
		std::map< std::string, std::vector<CustomIndexEntry> > customIndices;

		std::stringstream * target;

		const char * modelname;
		const char * keyName;
	};

	struct IndexDropper : public ArchiveAdapter<IndexDropper>
	{
	public:
		IndexDropper(const char * name, PostgresAdapter * adapter);

		template<typename T>
		void operator()(const char * name, T&, const Constraints& c)
		{
			std::stringstream index;

			if (c.isKey)
			{
				index << "DROP INDEX " << modelname << "KeyIndex";
				drops.push_back(index.str());
			}

			if (c.referenceType == Constraints::REFERENCE_IS_PARENT_LINK)
			{
				index.str("");

				index << "DROP INDEX " << modelname << "ToParentKeyIndex";
				drops.push_back(index.str());
			}

			if (c.isIndex && !c.isKey)
			{
				index.str("");

				index << "DROP INDEX " << modelname << name << "Index";
				drops.push_back(index.str());
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
		PostgresAdapter * adapter;
		std::vector<std::string> drops;
		std::set<std::string> customInds;
	};

	struct StatementBinder : public ArchiveAdapter<StatementBinder>
	{
	public:
		StatementBinder(const char ** outputData, int * outputLength, int * outputFormat, size_t index, char * buffer, size_t length);

		template<typename T>
		MODEL_DISABLE(T) operator()(const char * name, T& value, const Constraints& c)
		{
			size_t used = byteSwap(value, buffer + bytesUsed, bufferLength - bytesUsed);
			outputData[index] = (buffer + bytesUsed);
			bytesUsed += used;

			outputLength[index] = sizeof(value);
			outputFormat[index] = 1;
			consumed = 1;
		}

		template<typename T>
		void operator()(const char * name, boost::optional<T>& value, const Constraints& c)
		{
			if (value)
			{
				(*this)(name, *value, c);
			} else 
			{
				outputData[index] = NULL;
				outputLength[index] = 0;
				outputFormat[index] = 1;
				consumed = 1;
			}
		}

		void operator()(const char *, GUID& g, const Constraints&);
		void operator()(const char *, std::string&, const Constraints&);

		size_t getConsumed() const;
		size_t getBytesUsed() const;
	private:
		const char ** outputData;
		int * outputLength;
		int * outputFormat;

		size_t index;
		size_t consumed;

		char * buffer;
		size_t bytesUsed;
		size_t bufferLength;
	};

	struct QueryHelper : public ArchiveAdapter<QueryHelper>
	{
	public:
		QueryHelper(PGresult * result, size_t row, size_t column);

		template<typename T>
		MODEL_DISABLE(T) operator()(const char * name, T& out, const Constraints& c)
		{
			assert(PQfformat(result, column) == 1);

			char * data = PQgetvalue(result, row, column);
			size_t length = PQgetlength(result, row, column);
			assert(length >= sizeof(out));
			byteSwapOut(data, sizeof(out), out);

			consumed++;
		}

		template<typename T>
		MODEL_ENABLE(T) operator()(const char * name, T& out, const Constraints& c)
		{
			GUID id;
			(*this)(name, id, c);

			out.setPrimaryKey(id);
		}

		template<typename T>
		void operator()(const char * name, boost::optional<T>& out, const Constraints& c)
		{
			if (PQgetisnull(result, row, column))
			{
				out = boost::none;
				if (boost::is_same<T, GUID>::value) 
				{
					consumed = 2;
				} else 
				{
					consumed = 1;
				}
			} else 
			{
				out = T();
				(*this)(name, *out, c);
			}
		}

		void operator()(const char * name, GUID& out, const Constraints& c);
		void operator()(const char * name, std::string& out, const Constraints& c);
		
		size_t getConsumed() const;
	private:
		size_t row;
		size_t column;
		size_t consumed;
		PGresult * result;
	};

	struct InsertGenerator : public ArchiveAdapter<InsertGenerator>
	{
	public:
		InsertGenerator(const char * model, std::stringstream * self, PostgresAdapter& adapter, ModelData& cached);

		void operator()(const char * name, GUID&, const Constraints&);
		void operator()(const char * name, boost::optional<GUID>&, const Constraints&);

		template<typename T>
		MODEL_ENABLE(T) operator()(const char * name, std::vector<T>&, const Constraints&)
		{
			//These fields are not insertered.
		}

		template<typename T>
		MODEL_DISABLE(T) operator()(const char * name, T& value, const Constraints&)
		{
			*target << ", " << name;
			fieldCount++;
		}

		template<typename T>
		MODEL_ENABLE(T) operator()(const char * name, T& value, const Constraints& c)
		{
			//Single relations are modeled as GUIDs.
			GUID temp;
			(*this)(name, temp, c);
		}

		void finalize();
		bool needsSerialize() const;
	private:
		std::stringstream * target;
		const char * table;
		ModelData& cache;
		PostgresAdapter& adapter;
		size_t fieldCount;
	};

	struct InsertQuery : public ArchiveAdapter<InsertQuery>
	{
		static const size_t BUFFER_SIZE = 1024;
	public:
		InsertQuery(PostgresAdapter& adapter, ModelData& cache);
		~InsertQuery();

		template<typename T>
		MODEL_DISABLE(T) operator()(const char * name, T& value, const Constraints& c)
		{
			StatementBinder binder(data, length, format, index, swapbuffer + usedBuffer, BUFFER_SIZE - usedBuffer);
			binder.setLogger(log);
			binder(name, value, c);

			index += binder.getConsumed();
			usedBuffer += binder.getBytesUsed();
		}

		template<typename T>
		MODEL_ENABLE(T) operator()(const char * name, T& value, const Constraints& c)
		{
			GUID id = value.getPrimaryKey();

			(*this)(name, id, c);
		}

		template<typename T>
		MODEL_ENABLE(T) operator()(const char * name, std::vector<T>&, const Constraints&)
		{}
		
		void finalize();

	private:
		ModelData& cache;
		PostgresAdapter& adapter;

		const char ** data;
		int * length;
		int * format;
		size_t index;

		char swapbuffer[BUFFER_SIZE];
		size_t usedBuffer;
	};

	struct UpdateGenerator : public ArchiveAdapter<UpdateGenerator>
	{
	public:
		UpdateGenerator(const char * model, std::stringstream * self, PostgresAdapter& adapter, ModelData& cached);

		void operator()(const char * name, GUID&, const Constraints& c);
		void operator()(const char * name, boost::optional<GUID>&, const Constraints& c);

		template<typename T>
		MODEL_DISABLE(T) operator()(const char * name, T& value, const Constraints& c)
		{
			*target << ", " << name << " = " << "$" << fieldCount + 2;
			fieldCount++;
		}

		template<typename T>
		MODEL_ENABLE(T) operator()(const char * name, T& value, const Constraints& c)
		{
			GUID temp;
			(*this)(name, temp, c);
		}

		template<typename T>
		void operator()(const char * name, std::vector<T>& value, const Constraints&)
		{}

		void finalize();
		bool needsSerialize() const;
	private:
		std::stringstream * target;
		const char * table;
		const char * primary;
		ModelData& cache;
		PostgresAdapter& adapter;
		size_t fieldCount;
	};

	struct UpdateQuery : public ArchiveAdapter<UpdateQuery>
	{
		static const size_t BUFFER_SIZE = 1024;
	public:
		UpdateQuery(PostgresAdapter& adapter, ModelData& cache);
		~UpdateQuery();

		template<typename T>
		MODEL_DISABLE(T) operator()(const char * name, T& value, const Constraints& c)
		{
			StatementBinder binder(data, length, format, index + 1, swapbuffer + bytesUsed, BUFFER_SIZE - bytesUsed);
			binder.setLogger(log);

			binder(name, value, c);

			index += binder.getConsumed();
			bytesUsed += binder.getBytesUsed();
		}

		template<typename T>
		MODEL_ENABLE(T) operator()(const char * name, T& value, const Constraints& c)
		{
			GUID id = value.getPrimaryKey();
			(*this)(name, id, c);
		}

		template<typename T>
		MODEL_ENABLE(T) operator()(const char * name, std::vector<T>&, const Constraints&)
		{}

		bool finalize();

		template<typename U>
		void start(U& t)
		{
			GUID id = t.getPrimaryKey();
			StatementBinder binder(data, length, format, 0, swapbuffer + bytesUsed, BUFFER_SIZE - bytesUsed);
			binder.setLogger(log);

			binder(NULL, id, Constraints::none());
			bytesUsed += binder.getBytesUsed();
		}
	private:
		ModelData& cache;
		PostgresAdapter& adapter;

		const char ** data;
		int * length;
		int * format;
		size_t index;

		char swapbuffer[BUFFER_SIZE];
		size_t bytesUsed;
	};

	struct QueryGenerator : public ArchiveAdapter<QueryGenerator>
	{
	public:
		QueryGenerator(const char * name, std::stringstream * out, PostgresAdapter& adapter, ModelData& cache);

		void operator()(const char * name, GUID& guid, const Constraints& c);

		template<typename T>
		void operator()(const char * name, T&, const Constraints&)
		{}

		bool needsSerialize() const;
		void finalize();
	private:
		std::stringstream * target;
		const char * table;
		ModelData& cache;
		PostgresAdapter& adapter;
	};

	struct Query : public ArchiveAdapter<Query>
	{
		static const size_t BUFFER_SIZE = 1024;
	public:
		Query(PostgresAdapter& adapter, ModelData& cached);
		~Query();

		template<typename T>
		void operator()(const char * n, T& out, const Constraints& c)
		{
			QueryHelper q(result, 0, field);

			q(n, out, c);

			field += q.getConsumed();
		}

		template<typename T>
		void operator()(const char * name, std::vector<T>&, const Constraints& c)
		{}

		void setTarget(const GUID& t);
		void finalize();
		bool exists();
	private:
		ModelData& cache;
		PostgresAdapter& adapter;

		GUID target;
		size_t field;

		const char ** data;
		int * length;
		int * format;

		char swapbuffer[BUFFER_SIZE];
		size_t bytesUsed;

		PGresult * result;
	};

	struct DeleteGenerator : public ArchiveAdapter<DeleteGenerator>
	{
	public:
		DeleteGenerator(const char * name, std::stringstream * out, PostgresAdapter& adapter, ModelData& cache);

		void operator()(const char * name, GUID& guid, const Constraints& c);

		template<typename T>
		void operator()(const char * name, T&, const Constraints&)
		{}

		bool needsSerialize() const;
		void finalize();
	private:
		std::stringstream * target;
		const char * table;
		ModelData& cache;
		PostgresAdapter& adapter;
	};

	struct DeleteQuery : public ArchiveAdapter<DeleteQuery>
	{
		static const size_t BUFFER_SIZE = 1024;
	public:
		DeleteQuery(PostgresAdapter& adapter, ModelData& cached);
		~DeleteQuery();

		template<typename T>
		void operator()(const char *, T&, const Constraints&)
		{}

		void setTarget(const GUID& g);
		void finalize();
	private:
		ModelData& cache;
		PostgresAdapter& adapter;

		const char ** data;
		int * length;
		int * format;

		char swapbuffer[BUFFER_SIZE];
		size_t bytesUsed;
	};

	struct MultipleQuery : public ArchiveAdapter<MultipleQuery>
	{
	public:
		MultipleQuery();
		MultipleQuery(PGresult * result, size_t row);

		template<typename T>
		void operator()(const char * nm, T& out, const Constraints& c)
		{
			QueryHelper query(result, row, field);
			query(nm, out, c);

			field += query.getConsumed();
		}

		template<typename T>
		void operator()(const char * n, std::vector<T>&, const Constraints& c)
		{}
	private:
		PGresult * result;
		size_t row;
		size_t field;
	};

	struct ArbitraryQuery : public ArchiveAdapter<ArbitraryQuery>
	{
	public:
		ArbitraryQuery(const char * q, PostgresAdapter& adapter);
		~ArbitraryQuery();

		bool step(MultipleQuery& out);
	private:
		PGresult * result;
		PostgresAdapter& adapter;
		size_t row;
	};

	struct PostgresSchemaMigration : public ArchiveAdapter<PostgresSchemaMigration>
	{
		PostgresSchemaMigration(ORawrM::PostgresAdapter *);

		size_t getSchemaVersion(size_t current);
		void setSchemaVersion(size_t);

		void rename(const std::string& from, const std::string& to);
		void remove(const std::string& table);

		void startPopulate(const std::string& from, const std::string& to);
		void endPopulate();

		template<typename F, typename T>
		void renameColumn(const std::string& fd, const std::string& td)
		{
			columns.push_back(std::make_pair(fd, td));
		}

		std::string fromTable;
		std::string toTable;
		std::vector< std::pair<std::string, std::string> > columns;

		ORawrM::PostgresAdapter * adapter;
	};

	struct KeyQuery 
		: public ArchiveAdapter<KeyQuery>
		, public SelectGenerator<KeyQuery>
	{
	public:
		KeyQuery(const KeyPredicate& preds, const char * pkey, const char * table, PostgresAdapter& adapter, ModelData& data, bool generateCount = false, GetBridgeRelationship * bridge = NULL, bool isDelete = false);
		~KeyQuery();

		bool step(MultipleQuery& out);
		size_t count();

		void column(std::ostream& out, const char * tblName, const char * column, bool distinct);
		void join(std::ostream& out, const GetBridgeRelationship * bridge, const char * bta, const char * mta, const char * sta);
		void from(std::ostream& out, const char * tableName, const char * alias);
		void startWhere(std::ostream& out);
		void guids(std::ostream& out, const char * tableName, const char * columnName, const std::vector<ORawrM::GUID>& guids, bool requiresAndAfter);
		void guid(std::ostream& out, const char * A, const char * colA, const char * B, const char * colB);
		void guid(std::ostream& out, const char * A, const char * colA, const ORawrM::GUID& guid);
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
	private:
		const char * countedQueryAlias; 

		PostgresAdapter& adapter;
		PGresult * result;
		PGresult * countres;
		size_t row;
	};

	struct ColumnExists : public ArchiveAdapter<ColumnExists>
	{
		ColumnExists(const char * name);

		template<typename T>
		void operator()(const char * n, T&, const Constraints&)
		{
			if (strcmp(n, name) == 0)
			{
				result = true;
			}
		}

		const char * name;
		bool result;
	};

	struct OpenParameters
	{

		std::string server;
		std::string name;
		std::string pass;
	};

	struct Sanitizer
	{
		Sanitizer(PostgresAdapter * adapter);

		std::string operator()(const char *) const;

		PostgresAdapter * adapter;
	};
}}


namespace ORawrM 
{
	class PostgresAdapter : public HasLogger
	{
	public:
		typedef PG::OpenParameters	OpenParameters;
		typedef PG::ModelData		ModelCacheEntry;

		typedef PG::ModelGenerator	ModelGenerator;

		typedef PG::InsertGenerator InsertGenerator;
		typedef PG::InsertQuery		InsertQuery;

		typedef PG::UpdateGenerator UpdateGenerator;
		typedef PG::UpdateQuery		UpdateQuery;

		typedef PG::QueryGenerator	QueryGenerator;
		typedef PG::Query			Query;

		typedef PG::DeleteGenerator DeleteGenerator;
		typedef PG::DeleteQuery		DeleteQuery;

		typedef PG::MultipleQuery	KeyQueryMultiple;
		typedef PG::ArbitraryQuery	ArbitraryQuery;
		typedef PG::KeyQuery		KeyQuery;

		typedef PG::ColumnExists	ColumnExists;

		typedef PG::PostgresSchemaMigration SchemaMigration;
		typedef PG::IndexDropper    DropIndices;

		static size_t OID_STRING;
		static size_t OID_INT32;
		static size_t OID_INT64;
		static size_t OID_FLOAT;
		static size_t OID_DOUBLE;
		static size_t OID_UUID;

		PostgresAdapter();
		~PostgresAdapter();
		 
		void connect(const OpenParameters& params);
		void associateCaches(ModelCacheEntry * base, size_t size);

		std::string execute(const char * data);
		std::string compile(const char * query);
		std::string sanitize(const char *);

		void startTransaction();
		void commitTransaction();
		void rollbackTransaction();

		//Schema operations.
		void afterRegistration();
		void updateSchema();

		PGconn * getConnection();

		template<typename T>
		bool sanitizePredicates(std::vector<ColumnPredicate>& preds)
		{
			PG::Sanitizer sanitizer(this);
			return ORawrM::sanitizePredicate<T, ColumnExists>(preds, "_table", getLogger(), sanitizer);
		}

		//Used to redirect notices from postgres into the logger interface
		static void postgresNoticeProcessor(void *, const char * msg);
	private:
		PGconn * connection;
		size_t compiledStatements;
		size_t transactionDepth;

		ModelCacheEntry * cache;
		size_t cacheSize;
	};

}

#endif
#endif

