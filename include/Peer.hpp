#ifndef PEER_H
#define PEER_H

#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <string>

#include "Config.hpp"

using namespace boost;
using boost::asio::ip::tcp;

class Peer
{
public:

  enum State {
    CONNECTING = 0,
    ALIVE,
    DEAD
  };

  Peer(shared_ptr<tcp::socket> _socket);

  void set_state(State new_state);

  void start_listening();

  void listen();

  void handle_incoming_message(const system::error_code& error);
  
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

  void send(string message);

  void finish_write();

  bool is_alive() {
    return (state!=DEAD);
  }


private:
  string id;
  shared_ptr<tcp::socket> socket;
  char last_message[MESSAGE_SIZE];
  State state;
};


#endif
