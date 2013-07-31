#ifndef LISTENER_H
#define LISTENER_H

#include <boost/asio.hpp>
#include <list>

#include "Config.hpp"
#include "Network.hpp"
#include "Peer.hpp"

using namespace boost;
using boost::asio::ip::tcp;

class Listener {

public:
  Listener(shared_ptr<asio::io_service> io_service, shared_ptr<Network> _network);

private:
  void start_accept();

  void handle_accept(list<Peer*>::iterator new_peer_it,
                     const system::error_code& error);


  tcp::acceptor acceptor;
  shared_ptr<Network> network;
  list<Peer*> pending_peers;
};

#endif
