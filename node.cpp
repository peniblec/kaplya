#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <iostream>
#include <string>
#include <vector>

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
void display_incoming_message(shared_ptr<Peer> p);

class Peer
  : public enable_shared_from_this<Peer>
{
public:
  typedef shared_ptr<Peer> pointer;

  // indirection needed because sockets suck
  static pointer create(asio::io_service& io_service) {
    return pointer(new Peer(io_service));
  }

  Peer(shared_ptr<tcp::socket> _socket)
    : socket(_socket) {}

  void start_listening() {
    socket->async_read_some(asio::buffer(last_message),
			    bind(&display_incoming_message, shared_from_this()));

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
		      bind(&Peer::finish_write, shared_from_this()));
  }

  void finish_write() {
  }


private:
  Peer(asio::io_service& io_service)
    : socket(new tcp::socket(io_service)) {
  }

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
  
  void add_peer(Peer::pointer p) {

    peers.push_back(p);
  }

  vector<Peer::pointer> get_peers() {
    return peers;
  }

  void add(string peer_name) {

    tcp::resolver::query query(peer_name, "1337");
    tcp::resolver::iterator endpoint_iterator = resolver->resolve(query);

    shared_ptr<tcp::socket> socket(new tcp::socket(*io_service));
    asio::connect(*socket, endpoint_iterator);

    Peer::pointer new_peer = shared_ptr<Peer>(new Peer(socket));
    add_peer(new_peer);
    new_peer->start_listening(); // TODO: factorize stuff?
  }

  void send_all(string message) {

    for (uint p=0 ; p<peers.size() ; p++) {

      peers[p]->send(message);
    }
  }

private:
  shared_ptr<asio::io_service> io_service;
  shared_ptr<tcp::resolver> resolver;

  vector<Peer::pointer> peers;
  Local_Peer local_peer;
};


class Local_Listener {

public:
  Local_Listener(shared_ptr<asio::io_service> io_service, Network& _network)
    : acceptor(*io_service, tcp::endpoint(tcp::v4(), 1337)),
      network(_network)
  {
    start_accept();
  }

private:
  void start_accept()
  {
    new_peer = Peer::create(acceptor.get_io_service());

    network.add_peer(new_peer);

    acceptor.async_accept(new_peer->get_socket(),
			  bind(&Local_Listener::handle_accept, this, new_peer,
			       asio::placeholders::error) );
  }

  void handle_accept(Peer::pointer new_peer,
                     const system::error_code& error)
  {
    if (!error) {
      new_peer->start_listening();

      cout << "Peer at address " << new_peer->get_address() << " has joined!" << endl;

      start_accept();
    }
  }

  tcp::acceptor acceptor;
  Peer::pointer new_peer;
  Network network;
};

// function to bind socket readings to
void display_incoming_message(Peer::pointer p) {
  
  cout << "- " << p->get_address() << ": " << p->get_last_message() << endl;
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
  Network network(io_service, resolver, local_peer);
  Local_Listener llistener(io_service, network);

  
  thread io_service_thread(bind(&asio::io_service::run, io_service));

  for(;;) {

    cout << "What to do?" << endl
	 << "send <message> - Send <message> to peers" << endl
	 << "add <address> - Add peer with address <address>" << endl;

    string input, command, argument;
    getline(cin, input);
    parse_input(input, command, argument);


    if ( command.compare(COMMAND_SEND)==0 ) {

      network.send_all(argument);
    }
    else if ( command.compare(COMMAND_ADD)==0 ) {

      network.add(argument);
    }

  }

  return 0;
}
