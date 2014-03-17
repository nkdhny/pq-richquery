#include "pool.h"
#include "gtest/gtest.h"

static int _fake_valid = 1;
static int _fake_invalid = 0;

static PGconn* fake_valid_connection    = reinterpret_cast<PGconn*>(&_fake_valid);
static PGconn* fake_invalid_connection  = reinterpret_cast<PGconn*>(&_fake_invalid);
static PGconn* fake_null_connection = NULL;

static const int pool_size = 10;
static const int idle_size = 2;
static const int retry_count = 3;
static const long timeout = 100;

struct FakeConnectionCreator: std::unary_function<void, PGconn*> {

  enum FakeFactoryState{
    VALID,
    INVALID,
    NULLC
  };

  static FakeFactoryState state;
  static int counter;

  PGconn* operator()() {

    ++counter;

    switch(state) {
    case VALID:
      return fake_valid_connection;
    case INVALID:
      return fake_invalid_connection;
    default:
      return fake_null_connection;
    }

  }

  static void somethingVeryBadHappend() {
    state = INVALID;
  }

  static void somethingVeryGoodHappend() {
    state = VALID;
  }

  static void faktoryDisapeared() {
    state = NULLC;
  }

};
FakeConnectionCreator::FakeFactoryState FakeConnectionCreator::state = FakeConnectionCreator::VALID;
int FakeConnectionCreator::counter = 0;

struct Counter: std::unary_function<PGconn*, void> {

  int counter;
  static int staticCounter;

  Counter():
    counter(0)
  {}

  void operator ()(PGconn* c){
    ++counter;
    ++staticCounter;
  }
};
int Counter::staticCounter = 0;

struct StaticCounter: std::unary_function<PGconn*, void> {

  static int counter;

  StaticCounter()
  {}

  void operator ()(PGconn*){
    ++StaticCounter::counter;
  }
};
int StaticCounter::counter = 0;

struct FakeConnectionValidator: std::unary_function<PGconn*, bool> {

  int counter;
  static int staticCounter;

  FakeConnectionValidator():
    counter(0)
  {}

  bool operator()(PGconn* c) {
    ++counter;
    ++staticCounter;
    return *(reinterpret_cast<int*>(c)) == 1;
  }
};

int FakeConnectionValidator::staticCounter = 0;

class TestPool: public nkdhny::db::Pool<FakeConnectionCreator, FakeConnectionValidator, Counter, Counter, StaticCounter> {
public:
  TestPool():
    nkdhny::db::Pool<FakeConnectionCreator, FakeConnectionValidator, Counter, Counter, StaticCounter>(nkdhny::db::PoolParams(pool_size, idle_size, pool_size, retry_count-1, timeout), FakeConnectionCreator(), FakeConnectionValidator(), Counter(), Counter(), StaticCounter())
  {}

  int countCreated() {
    return createAction.counter;
  }

  int countValidated() {
    return validateAction.counter;
  }

  int countActivated() {
    return activateAction.counter;
  }

  int countPassivated() {
    return passivateAction.counter;
  }
};

TEST(PoolTest, happyPass) {

  FakeConnectionCreator::somethingVeryGoodHappend();
  FakeConnectionCreator::counter = 0;
  FakeConnectionValidator::staticCounter = 0;
  Counter::staticCounter = 0;
  StaticCounter::counter = 0;

  {
    TestPool p;
    StaticCounter::counter = 0;

    EXPECT_EQ(p.countCreated() , idle_size);
    EXPECT_EQ(p.countValidated() , idle_size);
    EXPECT_EQ(p.countActivated() , 0);
    EXPECT_EQ(p.countPassivated() , 0);

    {
      volatile TestPool::PooledConnection c = p.borrow();

      EXPECT_EQ(p.countCreated() , idle_size+1);
      EXPECT_EQ(p.countValidated() , idle_size+2); //one more is for validating borrowed object
      EXPECT_EQ(p.countActivated() , 1);
      EXPECT_EQ(p.countPassivated() , 0);
      EXPECT_EQ(StaticCounter::counter , 0);
    }

    EXPECT_EQ(p.countCreated() , idle_size+1);
    EXPECT_EQ(p.countValidated() , idle_size+2);
    EXPECT_EQ(p.countActivated() , 1);
    EXPECT_EQ(p.countPassivated() , 1);
    EXPECT_EQ(StaticCounter::counter , 0);
  }

  EXPECT_EQ(StaticCounter::counter , idle_size+1);

}

TEST(PoolTest, shouldThrowAnExceptionWhenDBIsBroken) {
  FakeConnectionCreator::faktoryDisapeared(); //now will create only null connections
  FakeConnectionCreator::counter = 0;
  FakeConnectionValidator::staticCounter = 0;
  Counter::staticCounter = 0;
  StaticCounter::counter = 0;

  EXPECT_THROW(TestPool p, nkdhny::db::PoolCouldNotCreateValidConnection);
  EXPECT_EQ(retry_count, FakeConnectionCreator::counter);
  EXPECT_EQ(0, Counter::staticCounter); //no was activated or passivated
  EXPECT_EQ(0, StaticCounter::counter); //no was destroyed
  EXPECT_EQ(0, FakeConnectionValidator::staticCounter); //no was validated
  FakeConnectionCreator::somethingVeryGoodHappend();

  FakeConnectionCreator::somethingVeryBadHappend();
  FakeConnectionCreator::counter = 0;
  FakeConnectionValidator::staticCounter = 0;
  Counter::staticCounter = 0;
  StaticCounter::counter = 0;

  EXPECT_THROW(TestPool p, nkdhny::db::PoolCouldNotCreateValidConnection);
  EXPECT_EQ(retry_count, FakeConnectionCreator::counter); //created invalid connections
  EXPECT_EQ(retry_count, StaticCounter::counter); //was validated
  EXPECT_EQ(retry_count, FakeConnectionValidator::staticCounter); //and destroyed
  EXPECT_EQ(0, Counter::staticCounter);
  FakeConnectionCreator::somethingVeryGoodHappend();


}

TEST(PoolTest, shouldTrowAnExceptionWhenExhausted) {

  FakeConnectionCreator::somethingVeryGoodHappend();
  FakeConnectionCreator::counter = 0;
  FakeConnectionValidator::staticCounter = 0;
  Counter::staticCounter = 0;
  StaticCounter::counter = 0;
  {
    TestPool p;

    std::vector<TestPool::PooledConnection> all;

    for(int i = 0; i < pool_size; i++) {
      all.push_back(p.borrow());
    }

    EXPECT_EQ(p.countCreated() , pool_size);
    EXPECT_EQ(p.countValidated() , pool_size*2);
    EXPECT_EQ(p.countActivated() , pool_size);
    EXPECT_EQ(p.countPassivated() , 0);
    EXPECT_EQ(StaticCounter::counter , 0);

    long start = nkdhny::gettime_ms();

    EXPECT_THROW(p.borrow(), nkdhny::db::PoolIsEmpty);
    long end = nkdhny::gettime_ms();
    EXPECT_TRUE(end-start >= timeout);
    EXPECT_TRUE(end-start < 2*timeout);

    all.pop_back();

    EXPECT_EQ(p.countCreated() , pool_size);
    EXPECT_EQ(p.countValidated() , pool_size*2);
    EXPECT_EQ(p.countActivated() , pool_size);
    EXPECT_EQ(p.countPassivated() , 1);
    EXPECT_EQ(StaticCounter::counter , 0);

    {
      volatile TestPool::PooledConnection c = p.borrow();

      EXPECT_EQ(p.countCreated() , pool_size);
      EXPECT_EQ(p.countValidated() , pool_size*2+1);
      EXPECT_EQ(p.countActivated() , pool_size+1);
      EXPECT_EQ(p.countPassivated() , 1);
      EXPECT_EQ(StaticCounter::counter , 0);

      while(!all.empty()) {
        all.pop_back();
      }
    }
  }

  EXPECT_EQ(FakeConnectionCreator::counter , pool_size);
  EXPECT_EQ(FakeConnectionValidator::staticCounter , pool_size*2+1);
  EXPECT_EQ(Counter::staticCounter , 2*(pool_size+1));
  EXPECT_EQ(StaticCounter::counter , pool_size);

}

int main(int argc, char **argv) {

  srand (time(NULL));

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
