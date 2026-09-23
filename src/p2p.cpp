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

#include "lib/p2p.h"

#ifdef _WIN32
    #pragma comment(lib, "ws2_32.lib")
#endif

P2PManager::P2PManager(int port, Blockchain* bc, TransactionManager* tm)
    : local_port(port), server_fd(INVALID_SOCKET_VAL), is_running(false), blockchain(bc), ts_manager(tm) {
    #ifdef _WIN32
        WSADATA wsaData;
        WSAStartup(MAKEWORD(2, 2), &wsaData);
    #endif
}

P2PManager::~P2PManager() {
    close();
    #ifdef _WIN32
        WSACleanup();
    #endif
}

bool P2PManager::start_node() {
    is_running = true;
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == INVALID_SOCKET_VAL) {
        Log_p2p.error(
            "[P2P] Lỗi khởi tạo socket !"
        );
        return false;
    }
    int opt = 1;
    #ifdef _WIN32
        setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));
    #else
        setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    #endif
    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(local_port);
    if (::bind(server_fd, (struct sockaddr*)&address, sizeof(address)) == SOCKET_ERROR_VAL) {
        Log_p2p.error(
            "[P2P] Lỗi gán Port !"
        );
        return false;
    }
    if (listen(server_fd, 5) == SOCKET_ERROR_VAL) {
        Log_p2p.error(
            "[P2P] Lỗi thiết lập lắng nghe !"
        );
        return false;
    }
    print_p2p();
    cout << "Đang chạy ngầm và lắng nghe kết nối trên Port: " << local_port << endl;
    Log_p2p.success(
        "[P2P] Khởi động Server thành công trên Port: ", local_port
    );
    thread(&P2PManager::server_listen_loop, this).detach();
    return true;
}

void P2PManager::server_listen_loop() {
    while (is_running) {
        sockaddr_in address;
        int addrlen = sizeof(address);
        #ifdef _WIN32
            SocketType client_socket = accept(server_fd, (struct sockaddr*)&address, &addrlen);
        #else
            SocketType client_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
        #endif
        if (client_socket != INVALID_SOCKET_VAL) {
            {
                lock_guard<mutex> lock(peers_mutex);
                active_peers.push_back({client_socket, 0});
            }
            loading_bar(3);
            print_p2p();
            cout << "Nhận kết nối thành công từ một Peer mới !" << endl;
            thread(&P2PManager::peer_receive_loop, this, client_socket).detach();
        }
    }
}

bool P2PManager::connect_to_peer(const string& ip, int peer_port) {
    SocketType sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET_VAL){        
        return false;
    } 
    sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(peer_port);
    #ifdef _WIN32
        serv_addr.sin_addr.s_addr = inet_addr(ip.c_str());
    #else
        inet_pton(AF_INET, ip.c_str(), &serv_addr.sin_addr);
    #endif
    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == SOCKET_ERROR_VAL) {
        print_p2p();
        cout << "Kết nối tới Peer tại " << ip << ":" << peer_port << " thất bại !" << endl;
        return false;
    }
    {
        lock_guard<mutex> lock(peers_mutex);
        active_peers.push_back({sock, peer_port});
    }
    print_p2p();
    cout << "Đã chủ động kết nối thành công tới Peer tại " << ip << ":" << peer_port << " !" << endl;
    json hello_msg;
    hello_msg["type"] = "GET_CHAIN";
    hello_msg["my_port"] = local_port;
    string payload = hello_msg.dump() + "\n";
    send(sock, payload.c_str(), payload.length(), 0);
    thread(&P2PManager::peer_receive_loop, this, sock).detach();
    return true;
}

void P2PManager::peer_receive_loop(SocketType peer_socket) {
    char buffer[4096];
    string raw_data = "";
    while (is_running) {
        int bytes_received = recv(peer_socket, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received <= 0) {
            if(is_running){
                print_p2p();
                cout << "Một Peer đã ngắt kết nối !" << endl;
            }
            {
                lock_guard<mutex> lock(peers_mutex);
                active_peers.erase(
                    remove_if(active_peers.begin(), active_peers.end(), 
                        [peer_socket](const PeerNode& p){
                            return p.socket == peer_socket;
                        }),
                    active_peers.end()
                );
            }
        #ifdef _WIN32
            closesocket(peer_socket);
        #else
            close(peer_socket);
        #endif
            break;
        }
        buffer[bytes_received] = '\0';
        raw_data += buffer;
        size_t pos;
        while ((pos = raw_data.find('\n')) != string::npos) {
            string single_message = raw_data.substr(0, pos);
            raw_data.erase(0, pos + 1);
            process_message(single_message, peer_socket);
        }
    }
}

void P2PManager::relay(const json& message, SocketType exclude_socket) {
    string payload = message.dump() + "\n";
    lock_guard<mutex> lock(peers_mutex);
    for (const auto& peer : active_peers) {
        if (peer.socket != exclude_socket) { 
            send(peer.socket, payload.c_str(), payload.length(), 0);
        }
    }
}

void P2PManager::process_message(const string& raw_message, SocketType sender_socket) {
    try {
        json msg = json::parse(raw_message);
        string type = msg["type"];
        if (msg.contains("my_port")) {
            int sender_port = msg["my_port"];
            string source_info = "peer:" + to_string(sender_port);
            lock_guard<mutex> lock(peers_mutex);
            for (auto& peer : active_peers) {
                if (peer.socket == sender_socket) {
                    peer.listen_port = sender_port;
                    break;
                }
            }
        }
        if (type == "GET_CHAIN") {
            json response = {
                {"type", "CHAIN_DATA"}, 
                {"data", blockchain->get_all_blocks_json()},
                {"my_port", local_port}
            };
            string payload = response.dump() + "\n";
            send(sender_socket, payload.c_str(), payload.length(), 0);
        }
        else if (type == "CHAIN_DATA") {
            print_p2p();
            cout << "Nhận dữ liệu chuỗi, đang đồng bộ..." << endl;
            blockchain->sync_chain(msg["data"]);
        }
        else if (type == "BROADCAST_TRANSACTION") {
            json ts_list = msg["data"].contains("transactions") ? msg["data"]["transactions"] : msg["data"];
            bool has_new_tx = false;
            for (auto& single_ts : ts_list) {
                string tx_code = single_ts["Transaction code"];
                string source_info = "peer:" + to_string(local_port);
                if (!blockchain->is_ts_in_mempool(tx_code) 
                    && ts_manager->verify_transaction(single_ts)) {
                    blockchain->add_transaction_to_mempool(single_ts);
                    has_new_tx = true;
                }
            }
            if (has_new_tx) {
                print_p2p();
                cout << "Đã nhận giao dịch mới và thêm vào mempool" << endl;
                relay(msg, sender_socket);
            }
        }
        else if (type == "BROADCAST_BLOCK") {
            json block_json = msg["data"];
            print_p2p();
            cout << "Nhận block: " << block_json["id"] << " từ mạng, đang xác thực..." << endl;
            blockchain->stop_mining = true;
            string source_info = "peer:" + to_string(local_port);
            if (blockchain->accept_external_block(block_json)) {
                blockchain->remove_mined_transactions(block_json["data"]);
                relay(msg, sender_socket);
            }
        }
        else if (type == "START_MINING") {
            json block_payload = msg["data"];
            print_p2p();
            cout << "Nhận yêu cầu đào từ mạng, bắt đầu tham gia đua !" << endl;
            start_mining_race(block_payload);
            relay(msg, sender_socket);
        }
        else if (type == "UPDATE_TRANSACTION") {
            json updated_tx = msg["data"];
            string ts_code = updated_tx["Transaction code"];
            bool found = false;
            for (auto& tx : blockchain->mempool) {
                if (tx["Transaction code"] == ts_code) {
                    tx = updated_tx;  
                    found = true;
                    break;
                }
            }
            if (found) {
                print_p2p();
                cout << "Đã cập nhật trạng thái hợp đồng: " << ts_code << endl;
                relay(msg, sender_socket);
            }
        }
        else if (type == "NODE_LEAVING") {
            int leaving_port = msg["port"];
            print_p2p();
            cout << "Node tại port " << leaving_port << " đã chủ động rời khỏi mạng !" << endl;
        }
    }
    catch (const exception& e) {
        Log_p2p.error(
            "\n[Lỗi P2P] ", e.what()
        );
    }
}

void P2PManager::start_mining_race(json block_payload) {
    thread([this, block_payload]() {
        json payload_with_miner = block_payload;
        payload_with_miner["miner"] = blockchain->owner_name;
        print_info();
        cout << "Đang tham gia đào block" << endl;
        bool mined = blockchain->add_block(payload_with_miner);
        if (mined) {
            json last_block = blockchain->get_last_block_json();
            json block_msg;
            block_msg["type"] = "BROADCAST_BLOCK";
            block_msg["data"] = last_block;
            this->broadcast(block_msg);
            blockchain->remove_mined_transactions(block_payload);
            print_info();
            cout << "Node này đào được block: " << last_block["id"] 
                 << ", đã phát tán cho toàn mạng !" << endl;
        }
    }).detach();
}

vector<int> P2PManager::get_connected_peer_ports() {
    vector<int> ports;
    lock_guard<mutex> lock(peers_mutex);
    for(const auto& peer : active_peers) {
        if(peer.listen_port > 0){
            ports.push_back(peer.listen_port);
        }
    }
    return ports;
}

void P2PManager::broadcast(const json& message) {
    string payload = message.dump() + "\n";
    lock_guard<mutex> lock(peers_mutex);
    for (const auto& peer : active_peers) {
        send(peer.socket, payload.c_str(), payload.length(), 0);
    }
}

void P2PManager::close() {
    is_running = false;
    json leave_msg;
    leave_msg["type"] = "NODE_LEAVING";
    leave_msg["port"] = local_port;
    broadcast(leave_msg);
    this_thread::sleep_for(chrono::milliseconds(300));
    if (server_fd != INVALID_SOCKET_VAL) {
    #ifdef _WIN32
        closesocket(server_fd);
    #else
        close(server_fd);
    #endif
        server_fd = INVALID_SOCKET_VAL;
    }

    lock_guard<mutex> lock(peers_mutex);
    for (const auto& peer : active_peers) {
    #ifdef _WIN32
        closesocket(peer.socket);
    #else
        close(peer.socket);
    #endif
    }
    active_peers.clear();
}