#include "ormcore.hpp"
#include "ormpredicate.hpp"

#ifndef ICM_ORM_STORE_VISITORS_HPP
#define ICM_ORM_STORE_VISITORS_HPP

namespace ORawrM
{
	//Cascaded delete
	template<typename DS>
	struct CascadeDelete : public ArchiveAdapter< CascadeDelete<DS> >
	{
		explicit CascadeDelete(DS * ds)
			:datastore(ds)
		{}

		//Forward optional.
		template<typename T>
		void operator()(const char * n, boost::optional<T>& v, const Constraints& c)
		{
			//If its null, cascading deletes wouldn't do anything anyway.
			if (v)
			{
				(*this)(n, *v, c);
			}
		}

		template<typename T>
		MODEL_ENABLE(T) operator()(const char * n, std::vector<T>&, const Constraints& c)
		{
			if (c.ownsChild)
			{
				//The things are stored on the other value.
				GetParentReferenceGUID get(c.relationID);
				Access::serializeTemporary<T>(get);

				datastore->template removeWhere<T>(get.name, primaryKey);
			}
		}

		template<typename T>
		MODEL_ENABLE(T) operator()(const char * n, T& v, const Constraints& c)
		{
			if (c.ownsChild)
			{
				datastore->remove(v);
			}
		}

		template<typename T>
		MODEL_DISABLE(T) operator()(const char * n, T&, const Constraints& c)
		{
			assert(!c.ownsChild && "You cannot own primitive types.");
		}

		//Remove all bridges that reference this type too
		template<typename U, typename BridgeType, typename First, typename Second>
		void registerBridgeRelationship(U& self, size_t relationID)
		{
			GetDeclarationReferencing selfName(Metadata<U>::name);
			Access::serializeTemporary<BridgeType>(selfName);

			typename DS::Transaction transact(*datastore);

			KeyPredicate predicate;
			predicate.whereGUID(selfName.result, self.getPrimaryKey());
			datastore->template removeWhere<BridgeType>(predicate);
		}

		template<typename U>
		void start(U& self)
		{
			primaryKey = self.getPrimaryKey();
		}

		GUID primaryKey;
		DS * datastore;
	};
}
#endif
