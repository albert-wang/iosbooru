/* Copyright (c) 2011 Ironclad Mobile, Inc. All rights reserved */

#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <cassert>
#include <ctime>
#include <set>
#include <map>
#include <iterator>
#include <locale>

//Some simple boost stuff.
#include <boost/preprocessor.hpp>
#include <boost/cstdint.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/static_assert.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/optional.hpp>
#include <boost/type_traits.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/mpl/contains.hpp>
#include <boost/range.hpp>
#include <boost/variant.hpp>

#include "ormlogger.hpp"
#include "md5.h"

#ifndef ICM_ORMCORE_HPP
#define ICM_ORMCORE_HPP

#define MODEL_ENABLE(T) \
	typename boost::enable_if_c<Metadata<T>::requiresLink>::type

#define MODEL_DISABLE(T) \
	typename boost::disable_if_c<Metadata<T>::requiresLink>::type

/*
	Supported ORM types.
		boost::int32_t and boost::int64_t	=> INTEGER
		GUID								=> INTEGER, INTEGER
		double, float						=> REAL
		string								=> STRING

		boost::optional<Above>				=> NULLABLE T
		std::vector<Above>					=> A bridge table containing GUID parent, Above value
	  

	Given a mapped type T, the following values are allowed:
		T									=> GUID reference
		boost::otional<T>					=> NULLABLE GUID reference
		std::vector<T>						=> Bridge table containing GUID parent, GUID next

	Constraints that modify such a relationship:
		childLink on std::vector<T>			=> Bridge table is ommited, the child /must/ have a GUID reference to the parent
		parentLink on T						=> Required in a value referenced through a childLink
*/
//Core
namespace ORawrM
{
	//Some constraints on the type
	struct Constraints
	{
		enum ReferenceType
		{
			REFERENCE_NONE = 0,
			REFERENCE_REQUIRES_CHILD_LINK,
			REFERENCE_IS_PARENT_LINK,
		};

		static Constraints requiresChild(const char * target);
		static Constraints isParentReference(size_t ref);
		static Constraints references(const char * name);
		static Constraints key();
		static Constraints nullable();
		static Constraints none();
		static Constraints relation(size_t id);
		static Constraints ownsChildren();
		static Constraints serverOnly();
		static Constraints index();
		static Constraints createsUploadRequest();
		static Constraints keyGenerationInput();
		static Constraints customIndex(const char * name, size_t order);

		const char * reference;
		bool null;
		bool isKey;
		bool ownsChild;
		bool isServerOnly;
		bool isIndex;
		bool uploadRequest;
		bool isKeyGenerationInput;

		size_t relationID;
		size_t parentRelationID;
		ReferenceType referenceType;

		const char * customIndexName;
		size_t customIndexOrder;

		void combine(const Constraints& c);
	private:
		Constraints();
	};

	//Model Constraints. Generally a server thing
	struct ModelConstraint
	{};

	struct GUID
	{
		GUID();
		GUID(boost::uint64_t high, boost::uint64_t low);
	
		bool operator==(const GUID& other) const;
		bool operator!=(const GUID& other) const;
		bool operator<(const GUID& other) const;

		boost::uint64_t high;
		boost::uint64_t low;

		std::string toString() const;
	};
	std::ostream& operator<<(std::ostream&, const GUID& g);
	static const GUID INVALID_GUID = GUID();

	template<typename T>
	struct Entry
	{
		Entry(const char * name, T& value, const Constraints& c) 
			:name(name)
			,value(value)
			,constraints(c)
		{}

		const char * name;
		T& value;
		mutable Constraints constraints;
	
		//This value is ONLY valid if T is std::vector< T >, with T being a base reflectable type.
		size_t index;
	};

	template<typename T>
	Entry<T> makeEntry(const char * name, T& value, const Constraints& c)
	{
		return Entry<T>(name, value, c);
	}

	template<typename T>
	struct Metadata
	{
		enum
		{
			INVALID_MODEL_ID = ~0u
		};

		//The name of the model. Used as the table name in the sql backend.
		static const char * name; 

		//Returns true if a relation that points to this must be a GUID link. 
		//Things like strings, ints, doubles don't need this, and should be false. 
		static const bool requiresLink = false;

		//A number representing the model. May not be consistent across compiles.
		static const size_t modelID = INVALID_MODEL_ID;

		//Forbidden status
		static const bool isForbidden = false;
	};

	template<typename F, typename S>
	struct BridgeModelType
	{};

	class HasLogger
	{
	public:
		void setLogger(ILogger * log);
		ILogger * getLogger();
	protected:
		ILogger * log;
	};
	
	template<typename T>
	class ArchiveAdapter : public HasLogger
	{
	public:
		template<typename U>
		void operator&(const Entry<U>& entry)
		{
			(*static_cast<T *>(this))(entry.name, entry.value, entry.constraints);
		}

		template<typename U, typename BridgeType, typename First, typename Second>
		void registerBridgeRelationship(U& self, size_t relationID)
		{}

		template<typename U>
		void start(U& self)
		{}

		template<typename U>
		void end(U& self)
		{}
	};

	//Device ID must be at least 4 bytes of data.
	//Randomness must have at least 16 bytes of randomness.
	GUID generateGUID(const char * randomness, const char * deviceID, size_t model);

	template<typename Generator>
	typename boost::disable_if_c<
		boost::is_function<typename boost::remove_pointer<Generator>::type>::value && boost::is_pointer<Generator>::value, 
		GUID>::type
	generateGUID(Generator& g, const char * did, size_t model)
	{
		boost::uint32_t buffer[4];

		for (size_t i = 0; i < sizeof(buffer) / sizeof(boost::uint32_t); ++i) {
			buffer[i] = g();
		}

		return generateGUID(
			reinterpret_cast<const char*>(buffer), 
			did, 
			model);
	}

	template<typename Generator>
	typename boost::enable_if_c<
		boost::is_function<typename boost::remove_pointer<Generator>::type>::value && boost::is_pointer<Generator>::value, 
		GUID>::type
	generateGUID(Generator g, const char * did, size_t model)
	{
		boost::uint32_t buffer[4];

		for (size_t i = 0; i < sizeof(buffer) / sizeof(boost::uint32_t); ++i) {
			buffer[i] = g();
		}

		return generateGUID(
			reinterpret_cast<const char*>(buffer), 
			did, 
			model);
	}

	//GUID Utilities
	size_t getModelID(const GUID& id);
	bool isReservedGUID(const GUID& id);

	//Typedef for a variant of all types. NullType is just a placeholder for null.
	struct NullType {};
	typedef boost::variant<std::string, boost::int64_t, double, GUID, NullType> DatastoreVariant; 

	std::ostream& operator<<(std::ostream&o, const NullType& n);

	//Used to access private members of things.
	class CustomQueryOut;
	class Access
	{
	public:
		template<typename T> 
		static T create()
		{
			return T();
		}

		template<typename T>
		static T * array(size_t n)
		{
			return new T[n];
		}

		template<typename T>
		static void release_array(const T * a)
		{
			delete [] a;
		}

		template<typename T, typename U>
		static void serializeTemporary(U& u)
		{
			static T target;
			target.serialize(u);
		}

		template<typename T, typename U>
		static void setFirst(T& t, const U& u)
		{
			t.reserved_setFirst(u.getPrimaryKey());
		}

		template<typename T, typename U>
		static void setSecond(T& t, const U& u)
		{
			t.reserved_setSecond(u.getPrimaryKey());
		}
	};

	struct BridgeBridgeRelationshipID
	{
		template<boost::uint32_t id>
		struct apply
		{
			static const boost::uint32_t value = (1 << 30) | id; 
		};

		static boost::uint32_t getID(boost::uint32_t id)
		{
			return (1 << 30) | id;
		}
	};
}

//Utility Visitors
namespace ORawrM
{

	//The following are a set of utility visitors to search for certain contraints.
	//In particular, the Key and ParentReference GUID values are searchable.
	struct GetParentReferenceGUID : public ArchiveAdapter<GetParentReferenceGUID>
	{
	public:
		explicit GetParentReferenceGUID(size_t refid)
			:target(refid)
			,name(NULL)
			,result(NULL)
		{}

		template<typename T>
		void operator()(const char *, T&, const Constraints& c)
		{}

		void operator()(const char * n, boost::optional<GUID>& value, const Constraints& c)
		{
			if (c.referenceType == Constraints::REFERENCE_IS_PARENT_LINK && c.parentRelationID == target)
			{
				if (!value)
				{
					value = GUID();
				}
				result = &*value;
				name = n;
			}
		}

		void operator()(const char * n, GUID& value, const Constraints& c)
		{
			if (c.referenceType == Constraints::REFERENCE_IS_PARENT_LINK && c.parentRelationID == target)
			{
				result = &value;
				name = n;
			}
		}

		template<typename U>
		void end(U&)
		{
			assert(result != NULL && "Target did not have a valid parent link set, when one was expected");
		}

		size_t target;
	 
		const char * name;
		GUID * result;
	};

	template<typename T>
	struct GetRelationPtr : public ArchiveAdapter<GetRelationPtr<T> >
	{
		GetRelationPtr(size_t id)
			:relationID(id)
			,result(NULL)
		{}

		template<typename U>
		void operator()(const char *, U&, const Constraints&)
		{}


		void operator()(const char *, T& a, const Constraints& c)
		{
			if (c.relationID == relationID)
			{
				result = &a;
			}
		}

		size_t relationID;
		T * result;
	};

	struct ConstraintsForColumn : public ArchiveAdapter<ConstraintsForColumn>
	{
		ConstraintsForColumn(const char * columnName)
			:constraints(Constraints::none())
			,column(columnName)
		{}

		template<typename T>
		void operator()(const char * name, T&, const Constraints& c)
		{
			if (strcmp(name, column) == 0)
			{
				constraints = c;
			}
		}

		Constraints constraints;
		const char * column;
	};

	struct GetDeclarationReferencing : public ArchiveAdapter<GetDeclarationReferencing>
	{
		GetDeclarationReferencing(const char * name)
			:target(name)
			,result(NULL)
		{}

		template<typename T>
		void operator()(const char * name, T& val, const Constraints& c)
		{
			if (c.reference && strcmp(c.reference, target) == 0)
			{
				result = name;
			}
		}

		const char * target;
		const char * result;
	};

	struct GetPrimaryKeyName : public ArchiveAdapter<GetPrimaryKeyName>
	{
		GetPrimaryKeyName() 
			:name(NULL)
		{}

		template<typename T>
		void operator()(const char *, T&, const Constraints& c)
		{}

		void operator()(const char * n, boost::optional<GUID>&, const Constraints& c)
		{
			if (c.isKey) { name = n; }
		}

		void operator()(const char * n, GUID&, const Constraints& c)
		{
			if (c.isKey) { name = n; }
		}

		template<typename U>
		void end(U&)
		{
			assert(name != NULL && "Target did not have a valid primary key, when one was expected.");
		}

		const char * name;
	};

	struct GetBridgeRelationship : public ArchiveAdapter<GetBridgeRelationship>
	{
		GetBridgeRelationship(size_t rel)
			:relationID(rel)
			,bridgeName(NULL)
			,firstName(NULL)
			,secondName(NULL)
			,firstNameIsCustom(false)
		{}

		template<typename Self, typename Bridge, typename First, typename Second>
		void registerBridgeRelationship(Self& self, size_t rel)
		{
			if (rel == relationID)
			{
				bridgeName = Metadata<Bridge>::name;

				GetDeclarationReferencing firstcol(Metadata<First>::name);
				Access::serializeTemporary<Bridge>(firstcol);

				GetDeclarationReferencing secondcol(Metadata<Second>::name);
				Access::serializeTemporary<Bridge>(secondcol);

				GetPrimaryKeyName firstpk;
				Access::serializeTemporary<First>(firstpk);

				GetPrimaryKeyName secondpk;
				Access::serializeTemporary<Second>(secondpk);

				if (Metadata<Self>::modelID == Metadata<First>::modelID)
				{
					firstName = Metadata<First>::name;
					firstColName = firstcol.result;
					firstPkeyName = firstpk.name;

					secondName = Metadata<Second>::name;
					secondColName = secondcol.result;
					secondPkeyName = secondpk.name;
				}
				
				if (Metadata<Self>::modelID == Metadata<Second>::modelID)
				{
					secondName = Metadata<First>::name;
					secondColName = firstcol.result;
					secondPkeyName = firstpk.name;

					firstName = Metadata<Second>::name;
					firstColName = secondcol.result;
					firstPkeyName = secondpk.name;
				}
			}
		};

		template<typename T>
		void operator()(const char * name, T&, const Constraints& constraints)
		{}

		size_t relationID;

		const char * bridgeName;
		
		bool firstNameIsCustom;
		const char * firstName;
		const char * firstColName;
		const char * firstPkeyName;

		const char * secondName;
		const char * secondColName;
		const char * secondPkeyName;
	};

	struct GetRelationByID : public ArchiveAdapter<GetRelationByID>
	{
		GetRelationByID(size_t relationID)
			:relationID(relationID)
			,name(NULL)
			,isOneToOne(false)
			,isBridgeRelationship(false)
			,isBridgeBridgeRelationship(false)
			,result()
		{}

		template<typename Self, typename Bridge, typename First, typename Second>
		void registerBridgeRelationship(Self& self, size_t rel)
		{
			if (relationID == rel)
			{
				isBridgeRelationship = true;
			}
			
			if (relationID == BridgeBridgeRelationshipID::getID(rel))
			{
				isOneToOne = false;
				isBridgeRelationship = false;
				isBridgeBridgeRelationship = true;

				GetDeclarationReferencing firstcol(Metadata<First>::name);
				Access::serializeTemporary<Bridge>(firstcol);

				GetDeclarationReferencing secondcol(Metadata<Second>::name);
				Access::serializeTemporary<Bridge>(secondcol);

				if (Metadata<Self>::modelID == Metadata<First>::modelID)
				{
					name = firstcol.result;
				}

				if (Metadata<Self>::modelID == Metadata<Second>::modelID)
				{
					name = secondcol.result;
				}
			}
		}

		template<typename T>
		typename boost::disable_if_c<Metadata<T>::requiresLink>::type operator()(const char *, T&, const Constraints&)
		{}

		template<typename T>
		typename boost::enable_if_c<Metadata<T>::requiresLink>::type operator()(const char * n, T& a, const Constraints& c)
		{
			if (c.relationID == relationID && !name)
			{
				isOneToOne = true;
				name = n;
				result = a.getPrimaryKey();
			}
		}

		template<typename T>
		void operator()(const char * n, std::vector<T>&, const Constraints& c)
		{
			if (c.relationID == relationID && !name)
			{
				isOneToOne = false;

				GetParentReferenceGUID g(relationID);
				Access::serializeTemporary<T>(g);

				name = g.name;
				result = GUID();
			}
		}

		template<typename T>
		void operator()(const char * n, GUID& g, const Constraints& c)
		{
			if (c.relationID == relationID && !name)
			{
				isOneToOne = true;
				name = n;
				result = g;
			}
		}

		size_t relationID;

		const char * name;
		bool isOneToOne;
		bool isBridgeRelationship;
		bool isBridgeBridgeRelationship;
		GUID result;
	};

	//An iterator that wraps another forward iterator, and deferences it.  
	template<typename It>
	class DerefIterator
	{
	public:
		typedef typename std::iterator_traits<It>::iterator_category iterator_category;
		typedef typename boost::remove_pointer<typename std::iterator_traits<It>::value_type>::type value_type;
		typedef typename std::iterator_traits<It>::difference_type difference_type;
		typedef typename boost::remove_pointer<typename std::iterator_traits<It>::value_type>::type * pointer;
		typedef typename boost::remove_pointer<typename std::iterator_traits<It>::value_type>::type& reference;

		DerefIterator(It it)
			:internal(it)
		{}
		
		reference operator*() 
		{
			return *(*internal);
		}

		pointer operator->() 
		{
			return (*internal);
		}

		DerefIterator<It>& operator++()
		{
			++internal;
			return *this;
		}

		DerefIterator<It> operator++(int)
		{
			DerefIterator<It> v = internal;
			++internal;
			return v;
		}

		bool operator==(DerefIterator<It>& it) const
		{
			return internal == it.internal;
		}

		bool operator!=(DerefIterator<It>& it) const
		{
			return internal != it.internal;
		}
	private:
		It internal;
	};

	template<typename T>
	DerefIterator<T> makeDerefIterator(T it)
	{
		return DerefIterator<T>(it);
	}

	struct GUIDEqual
	{
		GUIDEqual(const GUID& guid);

		template<typename T>
		bool operator()(const T& t)
		{
			return t.getPrimaryKey() == target;
		}

		GUID target;
	};
}

//Utility Functions. Primarily, relation mapping.
namespace ORawrM
{
	//Maps elements of B onto A, based on relationships.
	template<typename AI, typename BI>
	void mapRelationship(size_t relationID, AI begin, AI end, BI cbegin, BI cend)
	{
		typedef typename std::iterator_traits<AI>::value_type A;
		typedef typename std::iterator_traits<BI>::value_type B;

		GetRelationByID relation(relationID);
		Access::serializeTemporary<A>(relation);

		if (relation.isOneToOne)
		{
			//Mapping is on the parent side, determining the child stuff.
			std::map<GUID, B *> map;
			while(cbegin != cend)
			{
				map.insert(std::make_pair(cbegin->getPrimaryKey(), &*cbegin));
				cbegin++;
			}

			while(begin != end)
			{
				GetRelationByID rel(relationID);
				GetRelationPtr<B> ptr(relationID);

				begin->serialize(ptr);
				begin->serialize(rel);

				typename std::map<GUID, B *>::iterator it = map.find(rel.result); 
				if (it != map.end())
				{
					*ptr.result = *it->second;
				}
				++begin;
			}
			return;
		} else
		{
			std::map<GUID, std::vector<B> *> map;
			while (begin != end)
			{
				GetRelationPtr<std::vector<B> > ptr(relationID);
				begin->serialize(ptr);

				ptr.result->clear();
				map.insert(std::make_pair(begin->getPrimaryKey(), ptr.result));

				++begin;
			}

			while (cbegin != cend)
			{
				GetParentReferenceGUID parent(relationID);
				cbegin->serialize(parent);

				typename std::map<GUID, std::vector<B> *>::iterator it = map.find(*parent.result);
				if (it != map.end())
				{
					it->second->push_back(*cbegin);
				}

				++cbegin;
			}
		}
	}

	template<typename A, typename B>
	void mapRelationship(size_t rel, A& a, B& b)
	{
		mapRelationship(rel, boost::begin(a), boost::end(a), boost::begin(b), boost::end(b));	 
	}
}

//Byte swapping stuff
namespace ORawrM
{
	template<size_t N>
	struct ByteSwapImpl
	{};

	template<>
	struct ByteSwapImpl<2>
	{
		static void apply(const char * in, char * out)
		{
			out[0] = in[1];
			out[1] = in[0];
		}
	};

	template<>
	struct ByteSwapImpl<4>
	{
		static void apply(const char * in, char * out)
		{
			out[0] = in[3];
			out[1] = in[2];
			out[2] = in[1];
			out[3] = in[0];
		}
	};

	template<>
	struct ByteSwapImpl<8>
	{
		static void apply(const char * in, char * out)
		{
			out[0] = in[7]; out[1] = in[6];
			out[2] = in[5]; out[3] = in[4];
			out[4] = in[3]; out[5] = in[2];
			out[6] = in[1]; out[7] = in[0];
		}
	};
	
	template<typename T>
	size_t byteSwap(T value, char * out, size_t len)
	{
		assert(len >= sizeof(value));

		ByteSwapImpl<sizeof(value)>::apply(reinterpret_cast<const char *>(&value), out);

		return sizeof(value);
	}

	template<typename T>
	size_t byteSwapOut(const char * in, size_t len, T& value)
	{
		assert(len >= sizeof(value));

		ByteSwapImpl<sizeof(value)>::apply(in, reinterpret_cast<char *>(&value));

		return sizeof(value);
	}
}

//String normalization for md5 key generation.
namespace ORawrM
{
	struct RemoveInvalidChars
	{
		RemoveInvalidChars()
			:locale("C")
		{}

		bool operator()(char ch)
		{
			//Remove all non-printable characters, non-whitespace characters
			if (!std::isprint(ch, locale) && !std::isspace(ch, locale))
			{
				return true;
			}

			//Remove all punctuation except for _ and -
			if (std::ispunct(ch, locale))
			{
				return ch != '-' && ch != '_';
			}

			//Keep everything else.
			return false;
		}

		std::locale locale;
	};

	struct NormalizeString
	{
		NormalizeString()
			:locale("C")
		{}

		char operator()(char ch)
		{
			char result = std::tolower(ch, locale);

			//Transform whitepsace into dashes
			if (std::isspace(result, locale))
			{
				result = '-';
			}

			return result;
		}

		std::locale locale;
	};

	template<typename T>
	ORawrM::GUID hashField(const char * prefix, const T& data)
	{
		std::stringstream combine; 
		combine << prefix << data;

		//Filter out all the values with the high bit set
		std::string value = combine.str();
		for (size_t i = 0; i < value.size(); ++i)
		{
			value[i] = value[i] & 0x7F; 
		}

		//Remove all non-printable characters.
		value.erase(std::remove_if(value.begin(), value.end(), RemoveInvalidChars()), value.end());
		std::transform(value.begin(), value.end(), value.begin(), NormalizeString());

		//Trim left, trim right.
		bool wasTrimmed = false;
		for (size_t i = 0; i < value.size(); ++i)
		{
			if (value[i] != '-')
			{
				value = value.substr(i);
				wasTrimmed = true;
				break;
			}
		}

		for (int i = value.size() - 1; i >= 0; --i)
		{
			if (value[i] != '-')
			{
				value = value.substr(0, i + 1);
				wasTrimmed = true;
				break;
			}
		}
		
		if (!wasTrimmed)
		{
			value = "";
		}

		bool inRun = false;
		for (size_t i = 0; i < value.size(); ++i)
		{
			if (value[i] == '-') 
			{
				if (inRun)
				{
					value[i] = 0;
				} 
				else 
				{
					inRun = true;
				}
			}
			else
			{
				inRun = false;
			}
		}

		value.erase(std::remove(value.begin(), value.end(), 0), value.end());

		md5_state_t md5state;
		ORawrM::GUID result;

		md5_init(&md5state);
		md5_append(&md5state, reinterpret_cast<const md5_byte_t *>(value.c_str()), value.size());
		md5_finish(&md5state, reinterpret_cast<md5_byte_t*>(&result));

		return result;
	}
}

#define REGISTER_MODEL(n)																\
	namespace ORawrM { template<> struct Metadata<n> { static const bool requiresLink = true; static const size_t modelID; static const char * name;	\
	static const bool isForbidden; }; }

#define REGISTER_BRIDGE_MODEL(n, f, s)													\
	namespace ORawrM { template<> struct BridgeModelType<f, s> { typedef n value; static const bool forwardOrdering = true; }; template<> struct BridgeModelType<s, f> { typedef n value; static const bool forwardOrdering = false; }; }

#endif

