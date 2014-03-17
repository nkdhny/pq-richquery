pq-richquery
============

C++ wrapper library for libpq, with connection pooling

Inspired by JDBC template class for Java, but built ontop of libpq under linux.

Wrapper classes
---------------

Common wrapper classes provided:
1) `Connection` - wraps the `PGconn*` pointer in such a way it is automatically closed or returned to a pool after work is done
2) `Query`, `QueryTemplate` - query abstraction, both for one-time queries and prepared statemants.
3) `ParamBuilder` - parameter list builder for a query, parameters could be pushed in query in a typed way
4) `Result` - wraps the pointer to `PGresult` and frees the result after work is done
5) `Row` - typed accessor to a data associated with query execution result. One could iterate throug rows

Pool
----

Just another implementation of connection pooling. 
Main loop is as follows:
1) Client asks pool for a connection
2) If there is a good (i.e. validated with some validation query) connection available pool returns it to a client
3) After work is done client (implicitly) returns connection to a pool
4) Pool check whether transaction associated with connection is commited, if not - rollback the transaction


