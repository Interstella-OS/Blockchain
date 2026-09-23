// P2P Dùng win socket tạo validator trên port 
// Demo validator như thật kết nối qua mạng 
// Khi giao dịch phát tán và đào block giữa các validator
// 
//  _____                 _          _____               
// |  __ \               | |        |  __ \              
// | |__) |__  ___ _ __  | |_ ___   | |__) |__  ___ _ __ 
// |  ___/ _ \/ _ \ '__| | __/ _ \  |  ___/ _ \/ _ \ '__|
// | |  |  __/  __/ |    | || (_) | | |  |  __/  __/ |   
// |_|   \___|\___|_|     \__\___/  |_|   \___|\___|_|   
//
// Bản quyền: MIT LICENSE 2026              

#ifndef P2P_H
#define P2P_H

#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #define NOMINMAX
    #include <winsock2.h>
    #include <ws2tcpip.h>
    typedef SOCKET SocketType;
    #define INVALID_SOCKET_VAL INVALID_SOCKET
    #define SOCKET_ERROR_VAL SOCKET_ERROR
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    typedef int SocketType;
    #define INVALID_SOCKET_VAL -1
    #define SOCKET_ERROR_VAL -1
#endif

#include "header.h"
#include "color.h"
#include "blockchain.h"
#include "transaction_manager.h"

const string CONNECT_IP = "127.0.0.1";

struct PeerNode {
    SocketType socket;
    int listen_port; 
};

class P2PManager {
private:
    int local_port;
    SocketType server_fd;
    vector<PeerNode> active_peers;
    mutex peers_mutex;
    bool is_running;

    Blockchain* blockchain;
    TransactionManager* ts_manager;

    void server_listen_loop();
    void peer_receive_loop(SocketType peer_socket);
    void process_message(const string& raw_message, SocketType sender_socket);

public:
    P2PManager(int port, Blockchain* bc, TransactionManager* tm);
    ~P2PManager();

    void relay(const json& message, SocketType exclude_socket);
    void start_mining_race(json block_payload);
    
    bool start_node(); 
    bool connect_to_peer(const string& ip, int peer_port); 
    void broadcast(const json& message); 
    vector<int> get_connected_peer_ports();
    void print_connected_peer_ports(int LOCAL_PORT, P2PManager& p2p);
    void close();
};

#endif