#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <iostream>

#include "Config.hpp"
#include "Listener.hpp"
#include "Network.hpp"
#include "Peer.hpp"
#include "Utils.hpp"

using namespace boost;
using boost::asio::ip::tcp;


int main() {

  // setup and start local_listener in other thread
  shared_ptr<asio::io_service> io_service(new asio::io_service);
  shared_ptr<tcp::resolver> resolver(new tcp::resolver(*io_service));


  Local_Peer local_peer;
  shared_ptr<Network> network(new Network(io_service, resolver, local_peer));
  Listener listener(io_service, network);

  
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
