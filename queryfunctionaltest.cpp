#include "query.h"
#include <gtest/gtest.h>
#include "connection.h"
#include "row.h"

using namespace nkdhny::db;

PGconn * getConnection() {

        PGconn *conn = NULL;
        conn = PQconnectdb("user=\'credentials\' password=\'credentials\' dbname=\'richquery\' hostaddr=\'127.0.0.1\' port=\'5432\' connect_timeout=5");
        assert(conn != NULL);
        assert(PQstatus(conn) == CONNECTION_OK);
        return conn;
}


TEST(QueryTemplateTest, MustPerformDDLAndCRUD) {

    Connection<> c(getConnection());
    Query create(c, "create table t(i bigint);");
    create();

    Query insert(c, "insert into t values ($1);");
    insert.pushParameter(static_cast<long>(1));
    insert();
    insert.pushParameter(static_cast<long>(2));
    insert();

    Query select(c, "select i as _long from t");
    Result result = select();


    long i = 1;

    for(Row r = result.begin(); r < result.end(); ++r) {
        EXPECT_EQ(r.get<long>("_long"), i);
        ++i;
    }

    EXPECT_EQ(i, 3);

    Query remove(c, "delete from t where i=$1");
    remove.pushParameter(static_cast<long>(2));
    remove();

    Result result2 = select();

    i = 1;

    for(Row r = result2.begin(); r < result2.end(); ++r) {
        EXPECT_EQ(r.get<long>("_long"), i);
        ++i;
    }

    EXPECT_EQ(i, 2);

    Query drop(c, "drop table t;");
    drop();
}

int main(int argc, char **argv) {

  srand (time(NULL));

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

