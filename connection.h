#ifndef CONNECTION_H
#define CONNECTION_H

#include <postgresql/libpq-fe.h>
#include <functional>
#include <utility>
#include <assert.h>

namespace nkdhny {
namespace db {


/** @brief
  * Action to be executed when connection goes out of the scope
  * this is the very naive action - just close the connection
  * One could define action that returns connection to a pool
  * or doing nothing with it */
class CloseConnection: std::unary_function<PGconn*, void> {
public:
    void operator()(PGconn* connection);
};

template <typename CloseAction = CloseConnection>
class Connection
{
private:
    PGconn* connection;
    CloseAction closeAction;
    bool owner;

public:    
    Connection(const Connection& other);
    const Connection& operator = (const Connection& other);

    Connection(PGconn* _connection, CloseAction _closeAction = CloseAction());
    ~Connection();

    operator PGconn* ();
};


/** The Connection class is a
  * wrapper class ontop of the point to PG connection
  * typeparameter `CloseAction` is an action to be
  * executed when `Connection` object goes out of the scope.
  * For example it could be `CloseConnection`
  * Connection objects owns underlyied connection.
  * Ownership is transferred when object is copyied
*/
template <typename CloseAction>
Connection<CloseAction>::Connection(PGconn* _connection, CloseAction _closeAction):
    connection(_connection),
    closeAction(_closeAction),
    owner(true)
{}

template <typename CloseAction>
Connection<CloseAction>::~Connection()
{  
  if(owner){
    closeAction(connection);  
  }
}

template <typename CloseAction>
Connection<CloseAction>::operator PGconn*() {
    return connection;
}

template <typename CloseAction>
Connection<CloseAction>::Connection(const Connection &other):
  owner(true),
  connection(other.connection),
  closeAction(other.closeAction)
{
  assert(other.owner);

  const_cast<Connection&>(other).owner = false;
  const_cast<Connection&>(other).connection = NULL;
  const_cast<Connection&>(other).closeAction = CloseAction();
}

template <typename CloseAction>
const Connection<CloseAction>& Connection<CloseAction>::operator = (const Connection &other) {
  assert(const_cast<Connection&>(other).owner && !owner);

  std::swap(const_cast<Connection&>(other).owner, owner);
  std::swap(const_cast<Connection&>(other).connection, connection);
  std::swap(const_cast<Connection&>(other).closeAction, closeAction);

  return *this;
}

}
}

#endif // CONNECTION_H
