// Filename: queuedConnectionListener.h
// Created by:  drose (09Feb00)
//
////////////////////////////////////////////////////////////////////

#ifndef QUEUEDCONNECTIONLISTENER_H
#define QUEUEDCONNECTIONLISTENER_H

#include <pandabase.h>

#include "connectionListener.h"
#include "connection.h"
#include "netAddress.h"
#include "queuedReturn.h"

#include <prlock.h>
#include <deque>


class EXPCL_PANDA ConnectionListenerData {
public:
  // We need these methods to make VC++ happy when we try to
  // instantiate the template, below.  They don't do anything useful.
  INLINE bool operator == (const ConnectionListenerData &other) const;
  INLINE bool operator != (const ConnectionListenerData &other) const;
  INLINE bool operator < (const ConnectionListenerData &other) const;

  PT(Connection) _rendezvous;
  NetAddress _address;
  PT(Connection) _new_connection;
};

EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, QueuedReturn<ConnectionListenerData>);

////////////////////////////////////////////////////////////////////
//       Class : QueuedConnectionListener
// Description : This flavor of ConnectionListener will queue up all
//               of the TCP connections it established for later
//               detection by the client code.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA QueuedConnectionListener : public ConnectionListener,
                                 public QueuedReturn<ConnectionListenerData> {
PUBLISHED:
  QueuedConnectionListener(ConnectionManager *manager, int num_threads);
  virtual ~QueuedConnectionListener();

  bool new_connection_available();
  bool get_new_connection(PT(Connection) &rendezvous,
                          NetAddress &address,
                          PT(Connection) &new_connection);
  bool get_new_connection(PT(Connection) &new_connection);
 
protected:
  virtual void connection_opened(const PT(Connection) &rendezvous,
                                 const NetAddress &address,
                                 const PT(Connection) &new_connection);
};

#include "queuedConnectionListener.I"

#endif

