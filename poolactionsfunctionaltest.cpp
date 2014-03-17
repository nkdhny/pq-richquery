#include <gtest/gtest.h>
#include "poolactions.h"
#include <string>
#include "transaction.h"
#include "querytemplate.h"

static const std::string host = "127.0.0.1";
static const std::string database = "richquery";
static const std::string role = "credentials";
static const std::string password = "credentials";
static const int port = 5432;

static nkdhny::db::poolactions::PostgreCreate create(host, database, role, password, port);
static nkdhny::db::poolactions::QueryValidate validate_true("select 1");
static nkdhny::db::poolactions::QueryValidate validate_fasle("select 1 where 1 = 0");
static nkdhny::db::poolactions::CheckTransactionStatusPassivate passivate;
static nkdhny::db::poolactions::FreeConnectionDestroy destroy;

TEST(PoolActions, shouldCreateAndFreeConnectionProperly) {
  PGconn* conn = create();

  EXPECT_TRUE(conn!=NULL);
  EXPECT_EQ(CONNECTION_OK, PQstatus(conn));
  destroy(conn);
  EXPECT_EQ(CONNECTION_BAD, PQstatus(conn));
}

TEST(PoolActions, shouldValidateConnection) {
  PGconn* conn = create();

  EXPECT_TRUE(validate_true(conn));
  EXPECT_FALSE(validate_fasle(conn));

  destroy(conn);
}

TEST(PoolActions, sholdRollbackUnfinishedTransaction) {
  PGconn* c = create();

  using namespace nkdhny::db;


  Query create(c, "create table t(i bigint);");
  create();


  Query begin(c, "begin transaction;");
  begin();

  Query insert(c, "insert into t values ($1);");
  insert.pushParameter(static_cast<long>(1));
  insert();

  passivate(c);

  Query select(c, "select i as _long from t");
  Result result = select();

  EXPECT_EQ(result.begin(), result.end());

  Query drop(c, "drop table t;");
  drop();

  destroy(c);
}

TEST(PoolActions, passivateShouldDoNothingWhenCommitted) {
  PGconn* c = create();

  using namespace nkdhny::db;


  Query create(c, "create table t(i bigint);");
  create();

  Transaction t(c);

  Query insert(c, "insert into t values ($1);");
  insert.pushParameter(static_cast<long>(1));
  insert();

  t.commit();
  passivate(c);

  Query select(c, "select i as _long from t");
  Result result = select();

  EXPECT_NE(result.begin(), result.end());

  Query drop(c, "drop table t;");
  drop();

  destroy(c);
}

TEST(PoolActions, passivateShouldRestoreConnectionAfterAFail) {
  PGconn* c = create();

  using namespace nkdhny::db;


  Query create(c, "create table t(i bigint);");
  create();

  Transaction t(c);

  QueryTemplate insert(c, "insert into t values ($1);", "insert");

  PQclear(PQexec(c, "select * qwerty;"));

  insert.pushParameter(static_cast<long>(1));
  insert();


  Result result = Result(PQexec(c, "select i as _long from t;"));
  EXPECT_EQ(result.begin(), result.end());

  t.commit();

  passivate(c);

  Query select(c, "select i as _long from t;");

  insert.pushParameter(static_cast<long>(1));
  insert();

  Result result_passivated = select();
  EXPECT_NE(result_passivated.begin(), result_passivated.end());

  Query drop(c, "drop table t;");
  drop();

  destroy(c);
}


int main(int argc, char **argv) {

  srand (time(NULL));

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

