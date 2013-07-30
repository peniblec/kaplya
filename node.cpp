#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <list>

using namespace std;
using namespace boost;
using boost::asio::ip::tcp;

#ifdef DEBUG_ON
#define DEBUG(x) do { cerr << "DBG: " << x << endl; } while (0)
#else
#define DEBUG(x)
#endif


#define MESSAGE_SIZE 128

const string COMMAND_SEND = "send";
const string COMMAND_ADD = "add";

class Peer;
void display_incoming_message(Peer* p);

class Peer
{
public:

  Peer(shared_ptr<tcp::socket> _socket)
    : socket(_socket) {}

  void start_listening() {
    memset(last_message, 0, MESSAGE_SIZE);
    
    socket->async_read_some(asio::buffer(last_message),
			    bind(&display_incoming_message, this));
  }

  tcp::socket& get_socket() {
    return *socket;
  }

  string get_address() {
    return socket->remote_endpoint().address().to_string();
  }

  string get_last_message() {
    string ret(last_message);
    return ret;
  }

  void send(string message) {
    asio::async_write(*socket, asio::buffer(message),
		      bind(&Peer::finish_write, this));
  }

  void finish_write() {
  }


private:
  string id;
  shared_ptr<tcp::socket> socket;
  char last_message[MESSAGE_SIZE];
};


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

  Network(shared_ptr<asio::io_service> _ios, shared_ptr<tcp::resolver> _resolver,
	  Local_Peer& p)
    : io_service(_ios), resolver(_resolver), local_peer(p) {}
  
  void add_peer(shared_ptr<Peer> p) {

    peers.push_back(p);
  }

  vector<shared_ptr<Peer> > get_peers() {
    return peers;
  }

  void add(string peer_name) {

    tcp::resolver::query query(peer_name, "1337");
    tcp::resolver::iterator endpoint_iterator = resolver->resolve(query);

    shared_ptr<tcp::socket> socket(new tcp::socket(*io_service));
    asio::connect(*socket, endpoint_iterator);

    shared_ptr<Peer> new_peer(new Peer(socket));
    add_peer(new_peer);
    new_peer->start_listening(); // TODO: factorize stuff?
  }

  void send_all(string message) {

    DEBUG("Sending \"" << message << "\" to " << peers.size() << " peers.");

    for (uint p=0 ; p<peers.size() ; p++) {

      peers[p]->send(message);
    }
  }

private:
  shared_ptr<asio::io_service> io_service;
  shared_ptr<tcp::resolver> resolver;

  vector<shared_ptr<Peer> > peers;
  Local_Peer local_peer;
};


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
      new_peer->start_listening();

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

// function to bind socket readings to
void display_incoming_message(Peer* p) {
  
  cout << "- " << p->get_address() << ": " << p->get_last_message() << endl;

  p->start_listening();
}

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
