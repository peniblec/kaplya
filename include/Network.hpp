#ifndef NETWORK_H
#define NETWORK_H

#include <boost/asio.hpp>

#include "Config.hpp"
#include "Peer.hpp"

using namespace boost;
using boost::asio::ip::tcp;

class Local_Peer {
public:
  Local_Peer() {
    char name[HOST_NAME_MAX + 1];
    name[HOST_NAME_MAX] = '\0';
    // gethostname is not guaranteed to add \0 if it needs to truncate host name

    gethostname(name, sizeof(name) -1);

    id = string(name);
  }
private:
  string id;
};

class Network
{
public:

  Network(shared_ptr<asio::io_service> _ios,
          shared_ptr<tcp::resolver> _resolver,
	  Local_Peer& p);
  
  void add_peer(shared_ptr<Peer> p);
  
  vector<shared_ptr<Peer> > get_peers() {
    return peers;
  }

  void add(string peer_name);
  
  void send_all(string message);

  void check_peers();

private:
  shared_ptr<asio::io_service> io_service;
  shared_ptr<tcp::resolver> resolver;

  vector<shared_ptr<Peer> > peers;
  Local_Peer local_peer;
};


#endif
