//  ____  _            _        _           _       
// |  _ \| |          | |      | |         (_)      
// | |_) | | ___   ___| | _____| |__   __ _ _ _ __  
// |  _ <| |/ _ \ / __| |/ / __| '_ \ / _` | | '_ \    Demo mining blockchain C++
// | |_) | | (_) | (__|   < (__| | | | (_| | | | | |   Nâng cấp P2P và RSA
// |____/|_|\___/ \___|_|\_\___|_| |_|\__,_|_|_| |_|   Tác giả: Nguyễn Trường Chinh
// 
// Bản quyền: MIT LICENSE 2026

#include "include.h"

void print_info_owner(Blockchain& blockchain){
    double balance = blockchain.get_balance(blockchain.owner_name);
    int blocks_mined = blockchain.get_blocks_mined(blockchain.owner_name);
    cout << "[Miner]" << endl;
    cout << "● Tên miner: " << blockchain.owner_name << endl;
    cout << "● Số dư: " << to_string(balance) << " (BTC)"<< endl;
    cout << "● Số block đã đào: " << blocks_mined << endl;
    cout << endl;
}

void print_connected_peer_ports(int LOCAL_PORT, P2PManager& p2p){
    cout << "[Connected]" << endl;
    cout << "● Port: " << LOCAL_PORT << endl;
    auto peer_ports = p2p.get_connected_peer_ports();
    int count_ports = (peer_ports.empty() ? 0 : peer_ports.size());
    cout << "● Số lượng kết nối: " << count_ports << " port"<< endl;
    if(count_ports){
        cout << "● Port kết nối: ";
        for (size_t i = 0; i < peer_ports.size(); i++) {
            cout << peer_ports[i];
            if (i != peer_ports.size() - 1){
                cout << ", ";                
            } 
        }
        cout << endl;
    }
    cout << endl;
}

int main() {
    system("cls");
    printbanner(banner);

    show_console_cursor(false);
    UserBlockchain userblockchain;
    userblockchain.check_and_initialize_keys();

    // Nhập Port cho từng validator 
    int LOCAL_PORT, PEER_PORT;
    char choice_connect;

    show_console_cursor(true);

    cout << "(?) Nhập tên miner (chủ node): ";
    string OWNER_NAME; getline(cin, OWNER_NAME);
    if(OWNER_NAME.empty()){
        OWNER_NAME = "[Unknow]"; 
    }

    // Nhập port 1
    cout << "(?) Nhập port chạy Node này (ví dụ: 3000): ";
    cin >> LOCAL_PORT;
    cout << "(?) Bạn có muốn kết nối tới Node khác không? (y/n): ";
    cin >> choice_connect;

    cout << endl;
    int difficulty = 4;
    Blockchain blockchain(difficulty);
    blockchain.owner_name = OWNER_NAME;
    TransactionManager ts_manager;

    // Nhập port 2
    P2PManager p2p = P2PManager(LOCAL_PORT, &blockchain, &ts_manager);
    p2p.start_node();
    if (choice_connect == 'y' || choice_connect == 'Y') {
        cout << "(?) Nhập port của Node muốn kết nối (ví dụ: 3000): ";
        cin >> PEER_PORT;
        if(p2p.connect_to_peer(CONNECT_IP, PEER_PORT)){
            json sync_msg;
            sync_msg["type"] = "GET_CHAIN";
            p2p.broadcast(sync_msg);
            print_info();
            cout << "Đã gửi yêu cầu đồng bộ chuỗi..." << endl;
            loading_bar(3);
        } else {
            print_error();
            cout << "Không thể kết nối tới Node tại port: " << PEER_PORT << endl;
        }
    }

    show_console_cursor(true); 

    while (true) {
        system("cls");
        printbanner(banner);
        print_info_owner(blockchain);
        print_connected_peer_ports(LOCAL_PORT, p2p);

        cout << "[Menu]" << endl;
        cout << "[01] Tạo/phát tán giao dịch" << endl;
        cout << "[02] Kiểm tra hàng chờ" << endl;
        cout << "[03] Bắt đầu đào block" << endl;
        cout << "[04] Xem lịch sử chuỗi" << endl;
        cout << "[05] Tạo hợp đồng thời gian" << endl;
        cout << "[06] Tạo hợp đồng ký quỹ" << endl;
        cout << "[07] Duyệt hợp đồng ký quỹ" << endl;
        cout << "[08] Thoát" << endl;
        cout << "\n[?] Nhập lựa chọn: ";

        string input; getline(cin, input);
        int menu_choice;
        try {
            if(input.empty()){
                //print_error();
                throw invalid_argument("Vui lòng nhập đúng !");
            }
            size_t pos;
            menu_choice = stoi(input, &pos);
            if(pos != input.size()){
                //print_warning();
                throw invalid_argument("Không nhập ký tự thừa !");
            }
        } catch(...){
            //print_error();
            //cout << "Vui lòng nhập số !" << endl;
            pause_program();
            continue;
        }
        cout << "\n";
        if (menu_choice == 1) {
            print_info();
            cout << "Đang sinh giao dịch..." << endl;
            json new_block_data = ts_manager.create_transaction();
            json tx_msg;
            tx_msg["type"] = "BROADCAST_TRANSACTION";
            tx_msg["data"] = new_block_data;
            p2p.broadcast(tx_msg);
            json ts_list = new_block_data.contains("transactions") ?
                           new_block_data["transactions"] : new_block_data;
            for (auto& ts : ts_list) {
                //blockchain.mempool.push_back(tx);
                blockchain.add_transaction_to_mempool(ts);
            }
            print_success();
            cout << "Đã phát tán giao dịch thành công !" << endl;
        } 
        else if (menu_choice == 2) {
            cout << "Chi tiết giao dịch trong mempool" << endl;
            if (blockchain.mempool.empty()) {
                print_warning();
                cout << "Không có dữ liệu chờ !" << endl;
            } else {
                for (size_t i = 0; i < blockchain.mempool.size(); i++) {
                    json summary;
                    summary["Người gửi"] = blockchain.mempool[i]["Send name"];
                    summary["Người nhận"] = blockchain.mempool[i]["Receive name"];
                    summary["Số tiền"] = blockchain.mempool[i]["Amount"];
                    summary["Hợp đồng"] = SmartContract::describe(blockchain.mempool[i]);
                    cout << "\n[Giao dịch " << i + 1 << "]:" << summary.dump(4) << endl;
                }
                cout << "Tổng: " << blockchain.mempool.size() << " giao dịch đang chờ xử lý" << endl;
            }
        }
        else if (menu_choice == 3) {
            json ready_tx = json::array();
            json pending_tx = json::array();
            for (auto& tx : blockchain.mempool) {
                if (SmartContract::is_ready(tx)) ready_tx.push_back(tx);
                else pending_tx.push_back(tx);
            }
            if (ready_tx.empty()) {
                print_warning();
                cout << "Không có giao dịch nào đủ điều kiện để đào !" << endl;
                if (!pending_tx.empty())
                    cout << "(Có " << pending_tx.size() << " hợp đồng đang chờ điều kiện: chưa tới hạn hoặc chưa được duyệt)" << endl;
            } else {
                json block_payload;
                block_payload["transactions"] = ready_tx;
                p2p.start_mining_race(block_payload);
                blockchain.mempool = vector<json>(pending_tx.begin(), pending_tx.end());
                json start_msg;
                start_msg["type"] = "START_MINING";
                start_msg["data"] = block_payload;
                p2p.broadcast(start_msg);
                print_info();
                cout << "Đã gửi yêu cầu đào " << ready_tx.size() << " giao dịch sẵn sàng tới toàn mạng !" << endl;
            }
        }
        else if (menu_choice == 4) {
            blockchain.print_history();
            cout << "\n(?) Bạn có muốn xem xác thực toàn bộ blockchain (y/n): ";
            char view_detail; cin >> view_detail;
            cin_ignore();
            if (view_detail == 'y' || view_detail == 'Y') {
                blockchain.isvalid_chain();
            }
        } 
        else if (menu_choice == 5) {
            cout << "Nhập thời gian mở khóa (VD: 20:00:00 - 27/07/2026)" << endl;
            cout << "(?) Hoặc nhập enter để mở sau 1 phút nữa: ";
            string unlock_time; 
            getline(cin, unlock_time);
            if (unlock_time.empty()) {
                auto now = datetime::now();
                datetime floor_to_minute(now.year, now.month, now.day, now.hour, now.minute, 0);
                auto unlock = floor_to_minute + timedelta(0, 60, 0);
                unlock_time = unlock.strftime("%H:%M:%S - %d/%m/%Y");
                cout << "(Đã tự động chọn: " << unlock_time << ")" << endl;
            }
            json normal_tx = ts_manager.create_transaction(); 
            json single_tx = normal_tx[0];
            single_tx = SmartContract::attach_timelock(single_tx, unlock_time);
            blockchain.add_transaction_to_mempool(single_tx);
            json tx_msg;
            tx_msg["type"] = "BROADCAST_TRANSACTION";
            tx_msg["data"] = json::array({single_tx});
            p2p.broadcast(tx_msg);
            print_info();
            cout << "Đã tạo hợp đồng thời gian, chỉ được đào sau: " << unlock_time << endl;
        }
        else if (menu_choice == 6) {
            cout << "(?) Nhập tên người ký quỹ (VD: Chinh): ";
            //cin_ignore();
            string arbiter; getline(cin, arbiter);
            if(arbiter.empty()){
                print_warning();
                cout << "Vui lòng nhập tên !" << endl;
                pause_program();
                continue;
            }
            json normal_tx = ts_manager.create_transaction();
            json single_tx = normal_tx[0];
            single_tx = SmartContract::attach_escrow(single_tx, arbiter);
            blockchain.add_transaction_to_mempool(single_tx);
            json tx_msg;
            tx_msg["type"] = "BROADCAST_TRANSACTION";
            tx_msg["data"] = json::array({single_tx});
            p2p.broadcast(tx_msg);
            print_info();
            cout << "Đã tạo hợp đồng ký quỹ, cần " << arbiter << " duyệt mới được đào !" << endl;
        }
        else if (menu_choice == 7) {
            if (blockchain.mempool.empty()) {
                print_warning();
                cout << "Mempool hiện tại đang rỗng !" << endl;
                pause_program();
                continue;
            }
            bool has_escrow = false;
            for (const auto& tx : blockchain.mempool) {
                if (tx.contains("contract") 
                    && tx["contract"].contains("type") 
                    && tx["contract"]["type"] == "escrow"
                    && tx["contract"].contains("approved")
                    && !tx["contract"]["approved"]) {
                    has_escrow = true;
                    break;
                }
            }
            if (!has_escrow) {
                print_warning();
                cout << "Không có giao dịch ký quỹ nào đang chờ duyệt!" << endl;
                pause_program();
                continue;
            }
            cout << "Nhập tên người ký quỹ: ";
            string arbiter; getline(cin, arbiter);
            bool found = false;
            for (auto& tx : blockchain.mempool) {
                if (tx.contains("contract") && tx["contract"]["type"] == "escrow"
                    && tx["contract"]["arbiter"] == arbiter && !tx["contract"]["approved"]) {
                    SmartContract::approve_escrow(tx, arbiter);
                    print_success();
                    cout << "Đã duyệt hợp đồng ký quỹ: " << tx["Transaction code"] << endl;
                    json update_msg;
                    update_msg["type"] = "UPDATE_TRANSACTION";
                    update_msg["data"] = tx;
                    p2p.broadcast(update_msg);
                    found = true;
                }
            }
            if (!found){
                print_info();
                cout << "Không tìm thấy hợp đồng nào đang chờ bạn duyệt !" << endl;
            } 
        }
        else if (menu_choice == 8) {
            print_info();
            cout << "Port: " << LOCAL_PORT << " đã rời khỏi mạng !" << endl;
            break;
        } 
        else {
            print_warning();
            cout << "Vui lòng nhập lại !" << endl;
        }
        pause_program();
    }
    p2p.close();
    return 0;
}