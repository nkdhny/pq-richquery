pq-richquery
============

C++ wrapper library for libpq, with connection pooling

Inspired by JDBC template class for Java, but built ontop of libpq under linux.

Wrapper classes
---------------

Common wrapper classes provided:

*  `Connection` - wraps the `PGconn*` pointer in such a way it is automatically closed or returned to a pool after work is done
*  `Query`, `QueryTemplate` - query abstraction, both for one-time queries and prepared statemants.
*  `ParamBuilder` - parameter list builder for a query, parameters could be pushed in query in a typed way
*  `Result` - wraps the pointer to `PGresult` and frees the result after work is done
*  `Row` - typed accessor to a data associated with query execution result. One could iterate throug rows

Pool
----

Just another implementation of connection pooling. 
Main loop is as follows:
*  Client asks pool for a connection
*  If there is a good (i.e. validated with some validation query) connection available pool returns it to a client
*  After work is done client (implicitly) returns connection to a pool
*  Pool check whether transaction associated with connection is commited, if not - rollback the transaction

Build
-----

Use cmake to build, `libpq` and `gtest` are required. 

	cd /path/to/pq-richquery
	cd ..
	mkdir pq-richquery-build
	cmake -DDEBUG=yes ../pq-richquery
	make
	ctest

Also there is a couple of functional tests, one should costumize connection properties in all of these.

Library was tested under Ubuntu 13.10x64. Library is platform specific in part of converting postgre binary data representations in a platform data formats.


More documentation
------------------

See in-code documentation for details on how to extend query data getters and costumize pool actions.


