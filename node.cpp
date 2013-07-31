#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <list>

#include "Config.hpp"
#include "Peer.hpp"
#include "Network.hpp"

using namespace boost;
using boost::asio::ip::tcp;






class Local_Listener {

public:
  Local_Listener(shared_ptr<asio::io_service> io_service, shared_ptr<Network> _network)
    : acceptor(*io_service, tcp::endpoint(tcp::v4(), 1337)),
      network(_network)
  {
    start_accept();
  }

private:
  void start_accept()
  {
    shared_ptr<tcp::socket> socket(new tcp::socket(acceptor.get_io_service()));

    Peer* new_peer(new Peer(socket));

    list<Peer*>::iterator new_peer_it;
    new_peer_it = pending_peers.insert(pending_peers.begin(), new_peer);

    acceptor.async_accept(new_peer->get_socket(),
			  bind(&Local_Listener::handle_accept, this, new_peer_it,
			       asio::placeholders::error) );
  }

  void handle_accept(list<Peer*>::iterator new_peer_it,
                     const system::error_code& error)
  {
    if (!error) {
      shared_ptr<Peer> new_peer(*new_peer_it);

      network->add_peer(new_peer);

      pending_peers.erase(new_peer_it);

      DEBUG("Peer at address " << new_peer->get_address() << " has joined!");

      start_accept();
    }
    else {
      DEBUG("Local_Listener::handle_accept: " << error.message());
    }
        
  }

  tcp::acceptor acceptor;
  shared_ptr<Network> network;
  list<Peer*> pending_peers;
};


void parse_input(string& input, string& command, string& argument) {

  int space = input.find(' ');
  int l;

  char buf[input.size()];

  l = input.copy(buf, space, 0);
  buf[l] = '\0';
  command=string(buf);

  l = input.copy(buf, input.size() - space, space+1);
  buf[l] = '\0';
  argument=string(buf);
}

int main() {

  // setup and start local_listener in other thread
  shared_ptr<asio::io_service> io_service(new asio::io_service);
  shared_ptr<tcp::resolver> resolver(new tcp::resolver(*io_service));


  Local_Peer local_peer;
  shared_ptr<Network> network(new Network(io_service, resolver, local_peer));
  Local_Listener llistener(io_service, network);

  
  thread io_service_thread(bind(&asio::io_service::run, io_service));

  cout << "What to do?" << endl
       << "send <message> - Send <message> to peers" << endl
       << "add <address> - Add peer with address <address>" << endl;

  for(;;) {

    string input, command, argument;
    getline(cin, input);
    parse_input(input, command, argument);


    if ( command.compare(COMMAND_SEND)==0 ) {

      network->send_all(argument);
    }
    else if ( command.compare(COMMAND_ADD)==0 ) {

      network->add(argument);
    }

  }

  return 0;
}
