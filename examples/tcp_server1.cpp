#include "tcp_comm_socket.h"
#include "tcp_server_socket.h"

#include <iostream>
#include <lyra/lyra.hpp>

#define UNUSED(x) (void)(x)

using server_socket_type = Eventr::tcp_server_socket<2048>;
using comm_socket_type  = typename server_socket_type::comm_socket_type;

void on_receive(comm_socket_type &comm_socket, const comm_socket_type::buffer_type &buffer,
                const ssize_t &size)
{
  static int  count = 5;
  std::string data(buffer.data(), size);
  std::cout << "received : " << data << " on " << comm_socket << std::endl;

  // eacho back what is received
  comm_socket.send(buffer.data(), size);

  if (count-- == 0)
  {
    comm_socket.stop();
  }
}

void on_accept(server_socket_type &server, comm_socket_type comm_socket)
{
  UNUSED(server);
  std::cout << "New client connected: " << comm_socket << std::endl;

  comm_socket.set_on_receive(
      std::bind(on_receive, std::ref(comm_socket), std::placeholders::_1, std::placeholders::_2));
}

int main(int argc, char *argv[])
{
  uint32_t    server_port = 5000;
  std::string server_ip   = "0.0.0.0";

  auto cli = lyra::cli_parser() |
             lyra::opt(server_port, "port")["-p"]["--port"]("TCP port to listen on") |
             lyra::opt(server_ip, "ip")["-i"]["--ip"]("Local IP address");
  ;

  auto result = cli.parse({argc, argv});
  if (!result)
  {
    std::cerr << "Error in command line: " << result.errorMessage() << std::endl;
    exit(1);
  }

  Eventr::io_handler io(10);
  server_socket_type  server(io);

  server.start();

  server.bind(server_ip, server_port);
  server.listen();
  server.set_on_accept(std::bind(on_accept, std::ref(server), std::placeholders::_1));

  io.run();

  server.stop();

  return 0;
}