#include <stdint.h>
#include <netinet/in.h>

//good reference for this link header: https://web.cs.wpi.edu/~cs4514/b98/week3-dll/week3-dll.html#:~:text=By%20convention%2C%20we'll%20use,it%20with%20the%20received%20value.
struct LinkHeader {
  uint32_t checksum; //Why 32 bits: https://en.wikipedia.org/wiki/Frame_check_sequence
};

//References for the types: https://www.man7.org/linux/man-pages/man3/sockaddr.3type.html
//https://sites.uclouvain.be/SystInfo/usr/include/netinet/in.h
struct NetworkHeader {
   struct in_addr src_addr; //Para ipv4
   struct in_addr dst_addr; //Para ipv4
   // struct in6_addr src_addr; //Para ipv6
   // struct in6_addr dst_addr; //Para ipv6
};

// Reference for the types: https://en.wikipedia.org/wiki/Transmission_Control_Protocol#Checksum_computation
struct TransportHeader {
  int src_port;
  int dst_port;
  uint32_t seq_num;
  uint32_t ack_num;
  uint8_t flags; //ACK flag is bit 4 (0x10)
};

struct AppHeader {
  uint8_t msg_type; //Assuming we have less than 256 message types
  uint32_t session_id; //Assuming we have less than 2^32 sessions
};

/*[ Checksum (Enlace) | Endereço Origem, Endereço Destino (Rede) | Porta Origem, Porta Destino, Num Seq, Num ACK, Flag ACK (Transporte) | Tipo Mensagem, ID Sessão (Aplicação) | DADOS REAIS ]*/