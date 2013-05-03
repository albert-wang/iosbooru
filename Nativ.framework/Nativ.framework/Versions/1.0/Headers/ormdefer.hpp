/* Copyright (c) 2011 Ironclad Mobile, Inc. All rights reserved */

#include "ormcore.hpp"
#include "ormpostgres.hpp"
#include "ormsqlite3.hpp"
#include "ormstore.hpp"

#include <boost/shared_ptr.hpp>

#ifndef ICM_ORM_DEFERR
#define ICM_ORM_DEFERR

namespace ORawrM
{
	struct DeferredDelete
	{
		std::string memberName;
		GUID guid;
	};

	template<typename DS>
	struct DeferredExecute : public ArchiveAdapter< DeferredExecute<DS> >
	{
	public: 
		DeferredExecute(DS * store, DeferredDelete * target)
			:store(store)
			,target(target)
		{}

		template<typename T>
		void operator()(const char *, T&, const Constraints&)
		{}

		template<typename T>
		void operator()(const char * n, std::vector<T>&, const Constraints& c)
		{
			if (n == target->memberName)
			{
				T val = store->template select<T>(target->guid);
				store->template remove(val);
			}
		}

		DS * store;
		DeferredDelete * target;
	};
}
#endif

