/* Copyright (c) 2011 Ironclad Mobile, Inc. All rights reserved */

//Global header file for ORawrM.
//Use this instead of including files one by one.
//Note that the order of the includes is significant.

#ifndef ICM_ORM_HPP
#define ICM_ORM_HPP

//The core must be included first
#include "ormexception.hpp"
#include "ormcore.hpp"
#include "ormlogger.hpp"

//Followed by all the database adapters.
#include "ormpostgres.hpp"
#include "ormsqlite3.hpp"

//Then the datastore itself
#include "ormstore.hpp"

//Then the backings.
#include "ormbacking.hpp"

//The other utilities.
#include "ormjson.hpp"
#include "ormpredicate.hpp"

//Then all the models.
#include "Mutation.hpp"

#endif
















