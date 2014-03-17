#include "querytemplate.h"
#include <gtest/gtest.h>
#include "connection.h"
#include "row.h"
#include "transaction.h"

using namespace nkdhny::db;

PGconn * getConnection() {

        PGconn *conn = NULL;
        conn = PQconnectdb("user=\'credentials\' password=\'credentials\' dbname=\'richquery\' hostaddr=\'127.0.0.1\' port=\'5432\' connect_timeout=5");
        assert(conn != NULL);
        assert(PQstatus(conn) == CONNECTION_OK);
        return conn;
}

TEST(QueryTemplateTest, MustReadSomethingWithoutParameters) {
    Connection<> c(getConnection());
    QueryTemplate query(c, "select 1 as _int, 'text' as _str;");
    Result result = query();

    Row r = result.begin();

    EXPECT_EQ(r.get<int>(0), 1);
    EXPECT_EQ(r.get<std::string >("_str"), "text");
}

TEST(QueryTemplateTest, MustReadListWithoutParameters) {
    Connection<> c(getConnection());
    QueryTemplate query(c, "select 1 as _int union select 2 as _int;");
    Result result = query();

    int i = 1;

    for(Row r = result.begin(); r < result.end(); ++r) {
        EXPECT_EQ(r.get<int>("_int"), i);
        ++i;
    }

    EXPECT_EQ(i, 3);

}

TEST(QueryTemplateTest, MustReadListWithParameters) {
    Connection<> c(getConnection());
    QueryTemplate query(c, "select 1 as _int union select 2 as _int where $1=1 and $2='text'");

    query.pushParameter<int>(1);
    query.pushParameter<std::string>("text");

    Result result = query();

    int i = 1;

    for(Row r = result.begin(); r < result.end(); ++r) {
        EXPECT_EQ(r.get<int>("_int"), i);
        ++i;
    }

    EXPECT_EQ(i, 3);

}

TEST(QueryTemplateTest, MustPerformDDLAndCRUD) {

    Connection<> c(getConnection());
    QueryTemplate create(c, "create table t(i integer);");
    create();

    QueryTemplate insert(c, "insert into t values ($1);");
    insert.pushParameter(1);
    insert();
    insert.pushParameter(2);
    insert();

    QueryTemplate select(c, "select i as _int from t");
    Result result = select();


    int i = 1;

    for(Row r = result.begin(); r < result.end(); ++r) {
        EXPECT_EQ(r.get<int>("_int"), i);
        ++i;
    }

    EXPECT_EQ(i, 3);

    QueryTemplate remove(c, "delete from t where i=$1");
    remove.pushParameter(2);
    remove();

    Result result2 = select();

    i = 1;

    for(Row r = result2.begin(); r < result2.end(); ++r) {
        EXPECT_EQ(r.get<int>("_int"), i);
        ++i;
    }

    EXPECT_EQ(i, 2);

    QueryTemplate drop(c, "drop table t;");
    drop();
}


TEST(QueryTemplateTest, MustRollbackTranWhenUncommited) {

    Connection<> c(getConnection());
    QueryTemplate create(c, "create table t(i int);");
    create();
    {
        volatile Transaction t(c);

        QueryTemplate insert(c, "insert into t values ($1);");
        insert.pushParameter(1);
        insert();
        insert.pushParameter(2);
        insert();
    }

    QueryTemplate select(c, "select i as _int from t;");
    Result result = select();
    EXPECT_EQ(result.begin(), result.end());


    QueryTemplate drop(c, "drop table t;");
    drop();
}

TEST(QueryTemplateTest, MustBeCommited) {

    Connection<> c(getConnection());
    QueryTemplate create(c, "create table t(i int);");
    create();
    {
        Transaction t(c);

        QueryTemplate insert(c, "insert into t values ($1);");
        insert.pushParameter(1);
        insert();
        insert.pushParameter(2);
        insert();

        t.commit();
    }

    QueryTemplate select(c, "select i as _int from t;");
    Result result = select();
    EXPECT_NE(result.begin(), result.end());


    QueryTemplate drop(c, "drop table t;");
    drop();
}

int main(int argc, char **argv) {

  srand (time(NULL));

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
