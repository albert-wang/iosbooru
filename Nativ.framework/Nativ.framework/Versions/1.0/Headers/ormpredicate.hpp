/* Copyright (c) 2011 Ironclad Mobile, Inc. All rights reserved */

#include <vector>
#include <map>

#include <boost/type_traits.hpp>
#include "json/json.h"

#include "ormcore.hpp"
#include "ormjson.hpp"

#ifndef ICM_ORMPREDICATE_HPP
#define ICM_ORMPREDICATE_HPP

namespace ORawrM
{
	struct GUID;

	struct Ordering
	{
		Ordering();
		std::string column;

		//Default constructed ordering has descending = true.
		bool descending;
	};

	//Two major types of queries: GUID select and Predicate Select.
	struct ColumnPredicate
	{
		std::string value; 

		//If this field is set, then the value above is a column name.
		boost::optional<GUID> optionalGUID;
	};

	struct WhereClause
	{
		std::string column;
		std::string operation;
		std::string predicate;
	};

	//Used in the store to do subqueries.
	struct Subquery
	{
		Subquery();

		size_t limit;
		size_t offset;

		//The subgraph to use to feed the subquery.
		std::string subgraph;

		//The relation to travel on.
		std::string relation;

		//The name of the subquery. Given back to the callback.
		std::string name;
	};

	WhereClause parsePredicate(const std::string& pred);
	ColumnPredicate createPredicate(const WhereClause& clause);
	ColumnPredicate createPredicate(const std::string& pred);

	template<typename T, typename ColumnExists, typename Sanitizer> 
	bool sanitizePredicate(std::vector<ColumnPredicate>& preds, const char * tableSuffix, ILogger * log, Sanitizer sanitize)
	{
		std::vector<ColumnPredicate> result;
		result.reserve(preds.size());

		bool removedPredicates = false;
		for (size_t i = 0; i < preds.size(); ++i)
		{
			if (preds[i].optionalGUID)
			{
				std::string name = preds[i].value;
				
				ColumnExists exists(name.c_str());
				Access::serializeTemporary<T>(exists);
				if (!exists.result)
				{
					ORAWRM_LOG_ERROR(log, "Removed a predicate on: " + preds[i].value + ".");
					removedPredicates = true;
					continue;
				}

				WhereClause clause;
				clause.operation = "=";
				clause.column = name;
				clause.predicate = "'" + stringFromGUID(*preds[i].optionalGUID) + "'";
				result.push_back(createPredicate(clause));
			} else 
			{
				WhereClause clause = parsePredicate(preds[i].value);
				if (clause.operation == "HASREL" || clause.operation == "NOTREL") 
				{
					size_t relationshipID = T::getRelationID(clause.column);
					if (relationshipID == 0) 
					{
						ORAWRM_LOG_ERROR(log, "Removed a HASREL predicate: " + clause.column + " since it does not exist");
						removedPredicates = true;
						continue;
					}

					GetBridgeRelationship relation(relationshipID);
					Access::serializeTemporary<T>(relation);

					std::stringstream res;
					res << relation.firstPkeyName;
					if (clause.operation == "NOTREL")
					{
						res << " NOT";
					}

					res << " IN (SELECT " << relation.firstColName << " FROM " << relation.bridgeName << tableSuffix << ")";

					ColumnPredicate predicate;
					predicate.value = res.str();
					result.push_back(predicate);
				} 
				else 
				{
					ColumnExists exists(clause.column.c_str());
					Access::serializeTemporary<T>(exists);
					if (exists.result)
					{
						if (!clause.predicate.empty())
						{
							if ((clause.predicate[0] == '\'' && clause.predicate[clause.predicate.size() - 1] == '\'') || 
								(clause.predicate[0] == '\"' && clause.predicate[clause.predicate.size() - 1] == '\"'))
							{
								//Strip surrounding 's and "s if they exist.
								clause.predicate = clause.predicate.substr(1, clause.predicate.size() - 2);
							}

							clause.predicate = "'" + sanitize(clause.predicate.c_str()) + "'";
						}
						result.push_back(createPredicate(clause));
					} else 
					{
						ORAWRM_LOG_ERROR(log, "Removed a predicate: " + clause.column + ".");
						removedPredicates = true;
					}
				}
			}
		}
		preds = result;
		return !removedPredicates;
	}

	//Used to encapsulate all queries.
	//Note that relationships are only properly constructable through a call to relationKeys
	struct KeyPredicate
	{
		KeyPredicate();

		//The primary key to query from
		std::string name;

		//An array of things to order by.
		std::vector<Ordering> orderings;

		//Limit and offset. Zero values indicate no limit or offset.
		size_t resultLimit;
		size_t resultOffset;

		//These are highly dangerous values.
		
		//A custom source replaces the main table in the 'from' portion of a query. THIS MUST 	
		//HAVE A TABLE ALIASED TO 's', and that table MUST be the table that corresponds to the
		//type that the predicate is selecting for or have the EXACT SAME COLUMNS
		//
		//If any of these are used, it is up to the user to ensure that the query is well
		//formed, and returns the rows in expected order. 
		//
		//There must be no tables aliased to 'b', 'f' or 't'.
		std::string customSource;
		std::vector<std::string> customWheres;
		std::vector<std::string> customSourceWheres;

		//Only avaliable for external queries.
		boost::optional<std::string> externalBooleanSearch;


		//Array of column predicates.
		//Mutable, because sanitization might modify this list. 
		mutable std::vector<ColumnPredicate> predicates;
		bool isPredicateOnly;
		bool bridgeJoinIsLogicalOr;

		//Set this to true if the result set of a join will be large and requires an orderBy
		//This will surpress the filtering index in favor of using an ordering index.
		bool surpressFinalJoinFilteringIndex; 

		//If this is one-to-one, GUIDs are the primary keys of the child.
		//If this is one-to-many, GUIDs are the primary keys of the parent.
		std::vector<GUID> guids;

		//This is the primary keys of the parent no matter what the relation.
		std::vector<GUID> parentGUIDs;

		//Null if this isn't a relationship.
		boost::optional<bool> relationIsOneToOne;
		bool isBridgeRelationship; 

		//The name of the parent in a relationship. Null if its not a relationship.
		std::string parentModelName;
		std::string relationName;
		size_t relationID;

		KeyPredicate& orderBy(const std::string& field, bool descending = false);
		KeyPredicate& where(const std::string& pred);
		KeyPredicate& customWhere(const std::string& cw);
		KeyPredicate& whereGUID(const std::string& field, const GUID& g);
		KeyPredicate& limit(size_t limit);
		KeyPredicate& offset(size_t limit);
		KeyPredicate& bridgeJoinLogicalOr();
		KeyPredicate& bridgeJoinLogicalAnd();
		KeyPredicate& externalSearch(const std::string& data);
	};

	Json::Value serializePredicate(const KeyPredicate& pred);
	KeyPredicate deserializePredicate(const Json::Value& value);

	//primaryKeys should be templated on the resulting model type.
	template<typename T>
	KeyPredicate primaryKeys(const std::vector<GUID>& buffer)
	{
		BOOST_STATIC_ASSERT(Metadata<T>::requiresLink);
		KeyPredicate pred;
		
		GetPrimaryKeyName get;
		Access::serializeTemporary<T>(get);

		pred.isPredicateOnly = false;
		pred.name = get.name;
		pred.guids = buffer;
		return pred;
	}

	template<typename T>
	KeyPredicate primaryKeys(const GUID * buffer, size_t number)
	{
		KeyPredicate pred;
		
		GetPrimaryKeyName get;
		Access::serializeTemporary<T>(get);

		pred.name = get.name;
		pred.isPredicateOnly = false;
		
		pred.guids.resize(number);
		for (size_t i = 0; i < number; ++i)
		{
			pred.guids[i] = buffer[i];
		}

		return pred;
	}

	template<typename T>
	KeyPredicate primaryKeys(const GUID& g)
	{
		return primaryKeys<T>(&g, 1);
	}

	//U is the resulting type, T is inferred.
	template<typename U, typename T>
	KeyPredicate relationKeys(size_t relationID, T start, T end)
	{
		ORawrM::GetRelationByID relationship(relationID);
		typedef typename std::iterator_traits<T>::value_type parent_type;
		Access::serializeTemporary<parent_type>(relationship);
		
		if (relationship.isBridgeRelationship)
		{
			KeyPredicate result;
			result.relationID = relationID;
			result.isBridgeRelationship = true;
			result.isPredicateOnly = false;
			
			while (start != end)
			{
				result.guids.push_back(start->getPrimaryKey());
				start++;
			}

			result.parentModelName = Metadata<parent_type>::name;
			result.relationName = parent_type::getRelationName(relationID);
			return result;
		}

		//Keys are stored on the child value.
		if (!relationship.isOneToOne)
		{
			KeyPredicate result;

			if (!relationship.isBridgeBridgeRelationship)
			{
				ORawrM::GetParentReferenceGUID parent(relationID);
				Access::serializeTemporary<U>(parent);

				result.name = parent.name;
			} 
			else 
			{
				result.name = relationship.name;
			}

			result.relationIsOneToOne = false;
			result.isPredicateOnly = false;

			while (start != end)
			{
				result.guids.push_back(start->getPrimaryKey());
				result.parentGUIDs.push_back(start->getPrimaryKey());

				++start;
			}

			result.parentModelName = Metadata<parent_type>::name;
			result.relationName = parent_type::getRelationName(relationID);
			result.relationID = relationID;
			return result;
		} else 
		{
			KeyPredicate result;
			result.isPredicateOnly = false;
			result.relationIsOneToOne = true;

			GetPrimaryKeyName pkeyName;
			Access::serializeTemporary<U>(pkeyName);

			result.name = pkeyName.name;

			while (start != end)
			{
				GetRelationByID relation(relationID);
				start->serialize(relation);

				result.parentGUIDs.push_back(start->getPrimaryKey());
				result.guids.push_back(relation.result);
				++start;
			}

			result.parentModelName = Metadata<parent_type>::name;
			result.relationName = parent_type::getRelationName(relationID);
			result.relationID = relationID;
			return result;
		}
	}

	template<typename U, typename T>
	KeyPredicate relationKeys(size_t relation, T& element)
	{
		return relationKeys<U>(relation, &element, &element + 1);
	}

	//Same thing as relationKeys, except that it takes in an array of pointers to model.
	//Mostly used to avoid copies that might be expensive.
	//Mostly just used in node <_<
	template<typename U, typename T>
	KeyPredicate relationKeysThroughPointer(size_t relationID, T start, T end)
	{
		return relationKeys<U>(relationID, ORawrM::makeDerefIterator(start), ORawrM::makeDerefIterator(end));
	}

	//Used to resolve relationships in the opposite direction.
	//U is the result type, T is inferred.
	template<typename U, typename T>
	KeyPredicate inverseRelation(size_t relation, T start, T end)
	{
		typedef typename std::iterator_traits<T>::value_type child_type;
		typedef U parent_type;

		ORawrM::GetRelationByID relationship(relation);
		Access::serializeTemporary<parent_type>(relationship);

		if (!relationship.isOneToOne)
		{
			//If the relationship is one to many, then the child holds primary keys of the parents, so its pretty easy.
			//Flatten GUIDs
			std::vector<GUID> guids;
			while(start != end)
			{
				ORawrM::GetParentReferenceGUID parent(relation);
				start->serialize(parent);
				
				guids.push_back(*parent.result);

				++start;
			}

			std::sort(guids.begin(), guids.end());
			guids.erase(std::unique(guids.begin(), guids.end()), guids.end());
			
			return primaryKeys<U>(guids);
		} else 
		{
			//One to one relations are held on the parent relation as a GUID to the child.
			KeyPredicate result;
			result.name = relationship.name;
		   
			while(start != end)
			{
				GUID pid = start->getPrimaryKey();
				result.guids.push_back(pid);
				++start;
			}

			std::sort(result.guids.begin(), result.guids.end());
			result.guids.erase(std::unique(result.guids.begin(), result.guids.end()), result.guids.end());

			result.isPredicateOnly = false;
			return result;
		}
	}

	template<typename U, typename T>
	KeyPredicate inverseRelationThroughPointer(size_t relation, T start, T end)
	{
		return inverseRelation<U>(relation, makeDerefIterator(start), makeDerefIterator(end));
	}
	
	template<typename U, typename T>
	KeyPredicate inverseRelation(size_t relation, T& value)
	{
		return inverseRelation<U>(relation, &value, &value + 1);
	}
}

namespace ORawrM
{
	static const char * MAIN_TABLE_ALIAS = "s";
	static const char * BRIDGE_TABLE_ALIAS = "b"; 
	static const char * SECOND_TABLE_ALIAS = "f";
	//static const char * AUXILLARY_TABLE_ALIAS = "t";

	template<typename T>
	class SelectGenerator 
	{
	public:
		void column(std::ostream& out, const char * tblName, const char * column, bool distinct)
		{}
	
		void join(std::ostream& out, const GetBridgeRelationship * bridge, const char * bta, const char * mta, const char * sta)
		{}

		void from(std::ostream& out, const char * tableName, const char * alias)
		{}

		void startWhere(std::ostream& out)
		{}

		void guids(std::ostream& out, const char * tableName, const char * columnName, const std::vector<ORawrM::GUID>& guids, bool requiresAndAfter)
		{}

		void guid(std::ostream& out, const char * tblA, const char * colA, const char * tblB, const char * colB)
		{}

		void guid(std::ostream& out, const char * tblA, const char * colA, const ORawrM::GUID& guid)
		{}

		void where(std::ostream& out, const char * tableName, const std::vector<ColumnPredicate>& preds)
		{}

		void endWhere(std::ostream& out)
		{}

		void startOrder(std::ostream& out)
		{}

		void order(std::ostream& out, const char * tableName, const char * orderingColumn, bool isDescending, size_t index)
		{}

		void endOrder(std::ostream& out)
		{}

		void limit(std::ostream&, size_t l)
		{}

		void offset(std::ostream&, size_t o)
		{}

		void beginDeleteQuery(std::ostream&)
		{}

		void beginQuery(std::ostream&)
		{}

		void endQuery(std::ostream&)
		{}

		void count(std::ostream& out, const char * tableName, const char * optionalColumn, bool distinct)
		{}

		void generateSelectStatement(const KeyPredicate& p, const char * primaryKeyName, const char * tableName, std::ostream& out, bool isCount, GetBridgeRelationship * bridge, bool isDelete)
		{
			T& self = *static_cast<T *>(this);

			//This generates three or four distinct types of queries, depending on what we're passed in.
			if (isDelete)
			{
				self.beginDeleteQuery(out);
				out << " FROM "; 
				self.from(out, tableName, "");

				if (!p.isPredicateOnly || !p.predicates.empty())
				{
					self.startWhere(out);
					if (!p.isPredicateOnly)
					{
						self.guids(out, NULL, p.name.c_str(), p.guids, !p.predicates.empty());
					}

					if (!p.predicates.empty())
					{
						self.where(out, NULL, p.predicates);
					}
					self.endWhere(out);
				}

				if (p.resultLimit)
				{
					self.limit(out, p.resultLimit);
				}
				self.endQuery(out);
				out << ";";
				return;
			}

			//Standard query stuff.
			if (!p.isBridgeRelationship)
			{
				std::vector<std::string> mergedWheres;
				std::copy(p.customWheres.begin(), p.customWheres.end(), std::back_inserter(mergedWheres));
				std::copy(p.customSourceWheres.begin(), p.customSourceWheres.end(), std::back_inserter(mergedWheres));

				self.beginQuery(out);
				//If the query contains a custom Source, we can no longer guarantee 
				//uniqueness, so count and column become distinct to protect 
				//against this condition.
				if (isCount)
				{
					self.count(out, MAIN_TABLE_ALIAS, NULL, !p.customSource.empty());
				} else 
				{
					self.column(out, MAIN_TABLE_ALIAS, "*", !p.customSource.empty());
				}

				out << " FROM ";

				if (p.customSource.empty())
				{
					self.from(out, tableName, MAIN_TABLE_ALIAS);
				} else 
				{
					out << p.customSource << " ";
				}

				if (!p.isPredicateOnly || !p.predicates.empty() || !mergedWheres.empty())
				{
					self.startWhere(out);
					if (!p.isPredicateOnly)
					{
						bool requiresAnd = !p.predicates.empty() || !mergedWheres.empty();
						self.guids(out, MAIN_TABLE_ALIAS, p.name.c_str(), p.guids, requiresAnd);
					}
					
					if (!mergedWheres.empty())
					{
						for (size_t i = 0; i < mergedWheres.size(); ++i)
						{
							if (i) { out << " AND "; }
							out << "(" << mergedWheres[i] << ")";
						}

						if (!p.predicates.empty())
						{
							out << " AND ";
						}
					}

					if (!p.predicates.empty())
					{
						self.where(out, MAIN_TABLE_ALIAS, p.predicates);
					}

					self.endWhere(out);
				}

				//We're done here if htis is a count query
				if (isCount)
				{
					self.endQuery(out);
					out << ";";
					return;
				}

				if (!p.orderings.empty())
				{
					self.startOrder(out);
					
					for (size_t i = 0; i < p.orderings.size(); ++i)
					{
						self.order(out, MAIN_TABLE_ALIAS, p.orderings[i].column.c_str(), p.orderings[i].descending, i);
					}

					self.endOrder(out);
				} else
				{
					self.startOrder(out);
					self.order(out, MAIN_TABLE_ALIAS, NULL, false, 0); 
					self.endOrder(out);
				}

				if (p.resultLimit)
				{
					self.limit(out, p.resultLimit);
				}

				if (p.resultOffset && !isDelete)
				{
					self.offset(out, p.resultOffset);
				}
				
				self.endQuery(out);
				out << ";";
				return;
			}

			if (p.isBridgeRelationship && p.bridgeJoinIsLogicalOr)
			{
				self.beginQuery(out);

				/*
					The below requires some explanation.

					Normally, the generator will generate queries of the form
						select * from (second join bridge on second.id = bridge.id join main join bridge on main.id = bridge.id)
					
					This works perfectly, but if all you are doing is a count for one entry, then there is 
					no need to actually join on the main table. Bridge entries are usually unique, but certain
					usage patterns (in particular, the server) are not able to maintain this invariant. Therefore, 
					a distinct count is still required, but generally no join is required, which saves a lot of time. This 
					operation is common enough to deserve a custom code path.
				*/
				bool isSingleCount = isCount && !p.isPredicateOnly && p.guids.size() == 1 && p.predicates.empty() && p.customSource.empty();

				if (isCount)
				{
					//Basically, if we are a single count, then distinctness means nothing, and 
					//there is no need. Otherwise, it might be required, so add it in.
					self.count(out, isSingleCount ? SECOND_TABLE_ALIAS : MAIN_TABLE_ALIAS, NULL, true);
				} else 
				{
					self.column(out, MAIN_TABLE_ALIAS, "*", true);
				}

				out << " FROM ";
				
				//If there is a custom source, proxy the bridge data and inject it ino it.
				// This is terrible.
				if (!p.customSource.empty())
				{
					std::stringstream subquery;
					subquery << "(";
					self.beginQuery(subquery);
					self.column(subquery, MAIN_TABLE_ALIAS, "*", true); 
					subquery << " FROM " << p.customSource; 

					if (!p.customSourceWheres.empty())
					{
						self.startWhere(subquery);
						for (size_t i = 0; i < p.customSourceWheres.size(); ++i)
						{
							if (i) { subquery << " AND "; }
							subquery << " " << p.customSourceWheres[i] << " ";
						}
						self.endWhere(subquery);
					}

					self.endQuery(subquery);
					subquery << ")";

					GetBridgeRelationship proxy = *bridge;
					proxy.firstName = subquery.str().c_str();
					proxy.firstNameIsCustom = true;
					self.join(out, &proxy, BRIDGE_TABLE_ALIAS, MAIN_TABLE_ALIAS, SECOND_TABLE_ALIAS);
				} else
				{
					self.join(out, bridge, BRIDGE_TABLE_ALIAS, isSingleCount ? NULL : MAIN_TABLE_ALIAS, SECOND_TABLE_ALIAS);
				}

				if (!p.isPredicateOnly || !p.predicates.empty())
				{
					self.startWhere(out);
					if (!p.isPredicateOnly)
					{
						bool requiresAnd = !p.predicates.empty() || !p.customWheres.empty();
						self.guids(out, SECOND_TABLE_ALIAS, bridge->secondPkeyName, p.guids, requiresAnd);
					}

					if (!p.customWheres.empty())
					{
						for (size_t i = 0; i < p.customWheres.size(); ++i)
						{
							if (i) { out << " AND "; }
							out << "(" <<  p.customWheres[i] << ")";
						}

						if (!p.predicates.empty())
						{
							out << " AND ";
						}
					}

					if (!p.predicates.empty())
					{
						self.where(out, MAIN_TABLE_ALIAS, p.predicates);
					}

					self.endWhere(out);
				}
			
				//We're done here if htis is a count query
				if (isCount)
				{
					self.endQuery(out);
					out << ";";
					return;
				}

				if (!p.orderings.empty())
				{
					self.startOrder(out);
					
					for (size_t i = 0; i < p.orderings.size(); ++i)
					{
						self.order(out, MAIN_TABLE_ALIAS, p.orderings[i].column.c_str(), p.orderings[i].descending, i);
					}

					self.endOrder(out);
				} else
				{
					self.startOrder(out);
					self.order(out, MAIN_TABLE_ALIAS, NULL, false, 0); 
					self.endOrder(out);
				}

				if (p.resultLimit)
				{
					self.limit(out, p.resultLimit);
				}

				if (p.resultOffset && !isDelete)
				{
					self.offset(out, p.resultOffset);
				}

				self.endQuery(out);
				out << ";";
				return;
			}

			if (p.isBridgeRelationship && !p.bridgeJoinIsLogicalOr)
			{
				self.beginQuery(out);

				if (p.guids.empty())
				{
					//This makes no sense.
					return;
				}

				//This type of query does not require a unique
				if (isCount)
				{
					self.count(out, MAIN_TABLE_ALIAS, NULL, false);
				} else 
				{
					self.column(out, MAIN_TABLE_ALIAS, "*", false);
				}

				out << " FROM "; 

				/*
				This generates a query in the form
					select i.* from 
						Second s0 cross join Second s1 cross join Second s2 ....
					INNER JOIN Bridge b0
						b0.second_id = s0.id
					INNER JOIN Bridge b1
						b1.second_id = s1.id
						b1.first_id = b0.first_id
					....
					INNER JOIN First f
						bn.first_id = f.first_id         -- This index may be surpressed
				*/

				//Construct the cross joins.
				for (size_t i = 0; i < p.guids.size(); ++i)
				{
					std::stringstream tableAlias;
					tableAlias << SECOND_TABLE_ALIAS << i;
					if (i) 
					{
						out << " CROSS JOIN "; 
					}
					
					self.from(out, bridge->secondName, tableAlias.str().c_str()); 
				}

				//Now, conduct the bridge joins
				for (size_t i = 0; i < p.guids.size(); ++i)
				{
					std::stringstream bridgeAlias;
					bridgeAlias << BRIDGE_TABLE_ALIAS << i;

					std::stringstream tableAlias; 
					tableAlias << SECOND_TABLE_ALIAS << i;

					out << " INNER JOIN ";
					self.from(out, bridge->bridgeName, bridgeAlias.str().c_str());
					out << " ON "; 

					if (i)
					{
						std::stringstream previousBridgeAlias;
						previousBridgeAlias << BRIDGE_TABLE_ALIAS << i - 1;

						self.guid(out, bridgeAlias.str().c_str(), bridge->firstColName, previousBridgeAlias.str().c_str(), bridge->firstColName);
						out << " AND ";
					}

					self.guid(out, bridgeAlias.str().c_str(), bridge->secondColName, tableAlias.str().c_str(), bridge->secondPkeyName);
				}

				//The final join
				out << " INNER JOIN ";
				if (p.customSource.empty())
				{
					self.from(out, bridge->firstName, MAIN_TABLE_ALIAS);
				} else 
				{
					std::stringstream subquery;
					self.beginQuery(subquery);
					self.column(subquery, MAIN_TABLE_ALIAS, "*", true); 
					subquery << " FROM " << p.customSource; 

					if (!p.customSourceWheres.empty())
					{
						self.startWhere(subquery);
						for (size_t i = 0; i < p.customSourceWheres.size(); ++i)
						{
							if (i) { subquery << " AND "; }
							subquery << " " << p.customSourceWheres[i] << " ";
						}
						self.endWhere(subquery);
					}

					self.endQuery(subquery);
					out << "(" << subquery.str() << ") " << MAIN_TABLE_ALIAS;
				}
				out << " ON ";
				

				std::stringstream finalBridgeAlias;
				finalBridgeAlias << BRIDGE_TABLE_ALIAS << p.guids.size() - 1;

				std::stringstream auxillaryTableAlias;
				if (p.surpressFinalJoinFilteringIndex)
				{
					//XXX: this might be sqlite only
					auxillaryTableAlias << "+";
				}
				auxillaryTableAlias << MAIN_TABLE_ALIAS;
				
				self.guid(out, finalBridgeAlias.str().c_str(), bridge->firstColName, auxillaryTableAlias.str().c_str(), bridge->secondPkeyName);

				self.startWhere(out);
				for (size_t i = 0; i < p.customWheres.size(); ++i)
				{
					if (i) { out << " AND "; }
					out << "(" <<  p.customWheres[i] << ")";
				}

				if (!p.predicates.empty() || !p.guids.empty())
				{
					out << " AND ";
				}
				
				if (!p.guids.empty())
				{
					for (size_t i = 0; i < p.guids.size(); ++i)
					{
						std::stringstream tableAlias;
						tableAlias << SECOND_TABLE_ALIAS << i;

						if (i) { out << " AND "; }
						self.guid(out, tableAlias.str().c_str(), bridge->secondPkeyName, p.guids[i]);
					}

					//If an 'and' is required here...
					if (!p.predicates.empty())
					{
						out << " AND ";
					}
				}

				if (!p.predicates.empty())
				{
					self.where(out, MAIN_TABLE_ALIAS, p.predicates);
				}

				self.endWhere(out);

				//We're done here if this is a count query
				if (isCount)
				{
					self.endQuery(out);
					out << ";";
					return;
				}

				if (!p.orderings.empty())
				{
					self.startOrder(out);
					
					for (size_t i = 0; i < p.orderings.size(); ++i)
					{
						self.order(out, MAIN_TABLE_ALIAS, p.orderings[i].column.c_str(), p.orderings[i].descending, i);
					}

					self.endOrder(out);
				} else
				{
					self.startOrder(out);
					self.order(out, MAIN_TABLE_ALIAS, NULL, false, 0); 
					self.endOrder(out);
				}

				if (p.resultLimit)
				{
					self.limit(out, p.resultLimit);
				}

				if (p.resultOffset && !isDelete)
				{
					self.offset(out, p.resultOffset);
				}

				self.endQuery(out);
				out << ";";
				return;
			}
		}
	};
}
#endif
