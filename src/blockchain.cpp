#include "lib//blockchain.h"
#include "lib//merkle_tree.h"

Blockchain::Blockchain(int difficulty) {
    this->difficulty = difficulty;
    this->str_difficulty = string(this->difficulty, '0');
    this->genesis_block();
}

void Blockchain::genesis_block() {
    json genesis_data;
    genesis_data["msg"] = "Đây là block khởi tạo !"; 
    genesis_data["time"] = "00:00:00 - 26/07/2026";
    Block genesis_block(genesis_data);
    genesis_block.block_time = "00:00:00 - 27/07/2026";
    genesis_block.block_prev_hash = string(64, '0');
    genesis_block.block_curr_hash = genesis_block.hash_sha256();
    genesis_block.isvalid = true;
    this->chain.push_back(genesis_block);
    Log_blockchain.success(
        "Đã thêm block khởi tạo\n" 
    );
    //this->print_block(genesis_block);
    print_success();
    cout << "Thêm block khởi tạo vào chain !" << endl;
}

bool Blockchain::add_block(json data) {
    if (mining_in_progress) {
        print_info();
        cout << "Đang đào block khác, bỏ qua yêu cầu này !" << endl;
        return false;
    }
    mining_in_progress = true;
    Block block(data);
    Block prev_block(json::object());
    {
        lock_guard<mutex> lock(state_mutex);
        prev_block = this->chain.back();
        block.block_id = chain.size();
    }
    block.block_time = time_now();
    block.block_prev_hash = prev_block.block_curr_hash;
    block.block_curr_hash = block.hash_sha256();
    stop_mining = false;
    auto start_time = datetime::now();
    while (block.block_curr_hash.substr(0, this->difficulty) != this->str_difficulty) {
        if (stop_mining) {
            //cout << "[Mining] Đã hủy đào vì có block khác từ mạng đến trước !" << endl;
            mining_in_progress = false;
            return false;
        }
        block.block_nonce++;
        //cout << "\rNonce: " << block.block_nonce;
        block.block_curr_hash = block.hash_sha256();
    }
    auto end_time = datetime::now();
    block.mining_time = (end_time - start_time).total_seconds();
    cout << endl;
    block.isvalid = this->isvalid_block(block, prev_block);
    string miner_name;
    bool has_miner;
    {
        lock_guard<mutex> lock(state_mutex);
        if (block.block_prev_hash != this->chain.back().block_curr_hash) {
            print_warning();
            cout << "Chain đã thay đổi trong lúc đào, hủy kết quả này !" << endl;
            mining_in_progress = false;
            return false;
        }
        this->total_time += block.mining_time;
        this->chain.push_back(block);
        this->save_validblock(block);
        if(block.block_data.contains("miner")){
            miner_name = block.block_data["miner"];
            has_miner = true;
        }
    }
    if(has_miner){
        reward_miner(miner_name, MINING_REWARD);
    }
    Log_blockchain.success(
        "Đã thêm block: ", block.block_id, "\n"
    );
    this->print_block(block);
    mining_in_progress = false;
    return true;
}

bool Blockchain::accept_external_block(json block_json) {
    {
        lock_guard<mutex> lock(state_mutex);
        Block prev_block = this->chain.back();
        if (block_json["prev_hash"] != prev_block.block_curr_hash) {
            Log_blockchain.error(
                "[Blockchain] Block từ Peer không khớp prev_hash, bỏ qua !"
            );
            return false;
        }
        Block block(block_json["data"]);
        block.block_id = block_json["id"];
        block.block_prev_hash = block_json["prev_hash"];
        block.block_curr_hash = block_json["curr_hash"];
        block.block_time = block_json["time"];
        block.block_nonce = block_json["nonce"];
        block.mining_time = block_json["mining_time"];
        if (block_json.contains("merkle_root")
            && block_json["merkle_root"] != block.block_merkle_root) {
            Log_blockchain.error(
                "[Blockchain] Block từ Peer có Merkle root không khớp với giao dịch !"
            );
            return false;
        }
        if (block.block_curr_hash != block.hash_sha256()
            || block.block_curr_hash.substr(0, this->difficulty) != this->str_difficulty) {
            Log_blockchain.error(
                "[Blockchain] Block từ Peer không hợp lệ !"
            );
            return false;
        }
        block.mining_time = block_json["mining_time"];
        block.isvalid = true;
        this->chain.push_back(block);
        //this->save_validblock(block);
        Log_blockchain.success(
            "[Blockchain] Đã chấp nhận block: ", block.block_id, " từ mạng"
        );
        this->print_accepted_block(block);
    }
    return true;
}

void Blockchain::remove_mined_transactions(const json& block_data) {
    json mined_ts = block_data.contains("transactions") ? block_data["transactions"] : block_data;
    for (auto& mined_tx : mined_ts) {
        string mined_code = mined_tx["Transaction code"];
        this->mempool.erase(
            remove_if(this->mempool.begin(), this->mempool.end(),
                [&mined_code](json& mem_tx) { 
                    return mem_tx["Transaction code"] == mined_code; 
            }),
            this->mempool.end()
        );
    }
}

json Blockchain::get_last_block_json() {
    lock_guard<mutex> lock(state_mutex);
    Block& block = this->chain.back();
    json b;
    b["id"] = block.block_id;
    b["data"] = block.block_data;
    b["merkle_root"] = block.block_merkle_root;
    b["prev_hash"] = block.block_prev_hash;
    b["curr_hash"] = block.block_curr_hash;
    b["time"] = block.block_time;
    b["nonce"] = block.block_nonce;
    b["mining_time"] = block.mining_time;
    return b;
}

bool Blockchain::isvalid_block(Block block, Block prev_block) {
    if(block.block_prev_hash != prev_block.block_curr_hash) {
        Log_blockchain.error(
            "Lỗi quá trình xác thực tại block: ", block.block_id,
            "Do mã băm block: ", prev_block.block_id,
            "Không khớp vói mã băm block: ", block.block_id 
        );
        return false;
    }
    if(MerkleTree::create_merkleroot(block.block_data) != block.block_merkle_root) {
        Log_blockchain.error(
            "Lỗi quá trình xác thực tại block: ", block.block_id,
            "Do Merkle root không khớp với giao dịch trong block"
        );
        return false;
    }
    if(block.block_curr_hash != block.hash_sha256()) {
        Log_blockchain.error(
            "Lỗi quá trình xác thực tại block: ", block.block_id,
            "Do mã băm hiện tại không đúng với mã băm ban đầu"
        );
        return false;
    }
    return true;
}

bool Blockchain::isvalid_chain() {
    string str_isvalid = "";
    for(size_t i = 0; i < this->chain.size(); i++) {
        Block block = this->chain[i];
        if(MerkleTree::create_merkleroot(block.block_data) != block.block_merkle_root) {
            print_warning();
            cout << "Block: " << i << " có Merkle root không khớp với giao dịch" << endl;
            Log_blockchain.error(
                "Lỗi quá trình xác thực tại block: ", block.block_id,
                "Do Merkle root không khớp với giao dịch trong block"
            );
            return false;
        }
        if(!block.isvalid) {
            print_warning();
            cout << "Block: " << i << " không thể xác thực" << endl;
            Log_blockchain.error(
                "Lỗi quá trình xác thực tại block: ", block.block_id,
                "Do không thể xác minh"
            );
            return false;
        } else {
            str_isvalid += "Block: " + to_string(i) + " | "
                    + block.block_curr_hash + " | "
                    + "[Đã xác thực]\n";  
        }
    }
    print_success();
    cout << "Toàn bộ block đã được xác thực" << endl;
    cout << str_isvalid << endl;
    return true;
}

bool Blockchain::is_ts_in_mempool(string tscode){
    lock_guard<mutex> lock(state_mutex);
    for(const auto& tx : mempool){
        if(tx["Transaction code"] == tscode){
            return true;
        }
    }
    return false;
}

void Blockchain::add_transaction_to_mempool(json tx) {
    lock_guard<mutex> lock(state_mutex);
    this->mempool.push_back(tx);
}

void Blockchain::print_block(Block block) {
    json display_data = block.block_data;
    if (display_data.contains("transactions") && display_data["transactions"].is_array()) {
        for (auto& tx : display_data["transactions"]) {
            if (tx.contains("Signature")) {
                string sig = tx["Signature"];
                if (sig.length() > 20) {
                    tx["Signature"] = sig.substr(0, 10) + "..." + sig.substr(sig.length() - 10);
                }
            }
        }
    } 
    else if (display_data.is_array()) {
        for (auto& tx : display_data) {
            if (tx.contains("Signature")) {
                string sig = tx["Signature"];
                if (sig.length() > 20) {
                    tx["Signature"] = sig.substr(0, 10) + "..." + sig.substr(sig.length() - 10);
                }
            }
        }
    }
    else if (display_data.contains("Signature")) {
        string sig = display_data["Signature"];
        if (sig.length() > 20) {
            display_data["Signature"] = sig.substr(0, 10) + "..." + sig.substr(sig.length() - 10);
        }
    }
    cout << "Index: " << block.block_id << endl;
    cout << "Data: " << display_data.dump(4) << endl;
    cout << "Merkle root: " << block.block_merkle_root << endl;
    cout << "Prev hash: " << block.block_prev_hash << endl;
    cout << "Curr hash: " << block.block_curr_hash << endl;
    cout << "Time: " << block.block_time << endl;
    cout << "Nonce: " << block.block_nonce << endl;
    cout << "Mining time: " << block.mining_time << " s" << endl;
    cout << "\n";
}

void Blockchain::print_history(){
    cout << "Lịch sử xác thực block\n" << endl;
    cout << "Tổng cộng: " << chain.size() << " block trong hệ thống" << endl;
    for(int i = 0; i < chain.size(); i++){
        cout << "Block " << i << ": [" << (chain[i].isvalid ? "Hợp lệ" : "Không hợp lệ") 
             << "]" << " - Hash: " << chain[i].block_curr_hash << endl;
    }
}

void Blockchain::print_accepted_block(Block block){
    print_info();
    cout << "[Thông báo dừng đào]\nBlock: " << block.block_id 
         << " đã được node khác đào xong trước !" << endl;
    cout << "Hash: " << block.block_curr_hash << endl;
    cout << "Merkle root: " << block.block_merkle_root << endl;
    cout << "Nonce: " << block.block_nonce << endl;
    cout << "Thời gian: " << block.block_time << endl;
    cout << "Thời gian đào: " << block.mining_time << " s" << endl;
    cout << "Xác thực hash: [";
    if(block.hash_sha256() == block.block_curr_hash){
        cout << "Hợp lệ";
    } 
    else cout << "Không hợp lệ";
    cout << "]\n\n";
}

json Blockchain::get_all_blocks_json(){
    json all_blocks = json::array();
    for(auto& block : chain){
        json b;
        b["id"] = block.block_id;
        b["data"] = block.block_data;
        b["merkle_root"] = block.block_merkle_root;
        b["prev_hash"] = block.block_prev_hash;
        b["curr_hash"] = block.block_curr_hash;
        b["time"] = block.block_time;
        b["nonce"] = block.block_nonce;
        b["mining_time"] = block.mining_time;
        all_blocks.push_back(b);
    }
    return all_blocks;
}

void Blockchain::sync_chain(json json_chain) {
    lock_guard<mutex> lock(state_mutex); 
    if (json_chain.size() <= this->chain.size()) {
        return;
    }
    vector<Block> new_chain;
    for (auto& b_json : json_chain) {
        Block b(b_json["data"]);
        b.block_id = b_json["id"];
        b.block_prev_hash = b_json["prev_hash"];
        b.block_curr_hash = b_json["curr_hash"];
        b.block_time = b_json["time"];
        b.block_nonce = b_json["nonce"];
        b.mining_time = b_json["mining_time"];
        bool merkle_ok = true;
        if (b_json.contains("merkle_root")
            && b_json["merkle_root"] != b.block_merkle_root) {
            merkle_ok = false;
        }
        b.isvalid = merkle_ok
                    && (b.block_curr_hash == b.hash_sha256()) && 
                    (b.block_curr_hash.substr(0, this->difficulty) == this->str_difficulty);        
        new_chain.push_back(b);
    }
    this->chain = new_chain;
    Log_blockchain.success(
        "[Blockchain] Đã đồng bộ chuỗi thành công với ", this->chain.size(), " block"
    );
}

void Blockchain::save_validblock(Block block){
    if(!fs::exists(path_folder_valid_block)){
        fs::create_directories(path_folder_valid_block);
    }
    json all_valid_blocks = json::array();
    if (fs::exists(path_valid_block) && fs::file_size(path_valid_block) > 0) {
        ifstream file(path_valid_block);
        try {
            file >> all_valid_blocks;
        } catch (...) {
            Log_blockchain.warning(
                "[Blockchain] File: ", path_valid_block,
                " bị lỗi định dạng, sẽ ghi đè lại từ đầu !"
            );
            all_valid_blocks = json::array();
        }
        file.close();
    }
    json block_json;
    block_json["id"] = block.block_id;
    block_json["data"] = block.block_data;
    block_json["merkle_root"] = block.block_merkle_root;
    block_json["prev_hash"] = block.block_prev_hash;
    block_json["curr_hash"] = block.block_curr_hash;
    block_json["time"] = block.block_time;
    block_json["nonce"] = block.block_nonce;
    block_json["mining_time"] = block.mining_time;
    block_json["isvalid"] = block.isvalid;
    all_valid_blocks.push_back(block_json);
    ofstream outfile(path_valid_block);
    outfile << all_valid_blocks.dump(4);
    outfile.close();
    Log_blockchain.success(
        "[Blockchain] Đã lưu block hợp lệ: ", block.block_id, 
        "vào file ", path_valid_block
    );
}

void Blockchain::reward_miner(const string& miner_name, double amount){
    lock_guard<mutex> lock(state_mutex);
    if(!fs::exists(path_data)){
        fs::create_directories(path_data);
    }
    json balances = json::object();
    if (fs::exists(path_wallet_balance) && fs::file_size(path_wallet_balance) > 0) {
        ifstream file(path_wallet_balance);
        try { 
            file >> balances; 
        } catch (...) { 
            balances = json::object(); 
        }
    }
    double current_balance = 0.0;
    int current_blocks = 0;
    if (balances.contains(miner_name)) {
        current_balance = balances[miner_name].value("balance", 0.0);
        current_blocks = balances[miner_name].value("blocks_mined", 0);
    }
    current_balance += amount;
    current_blocks += 1;  
    balances[miner_name]["balance"] = current_balance;
    balances[miner_name]["blocks_mined"] = current_blocks;
    ofstream out(path_wallet_balance);
    out << balances.dump(4);
    Log_rewardminer.success(
        "Đã thưởng ", amount, " (BTC) cho miner: ", miner_name,
        " -> Số dư mới: ", current_balance,
        " | Tổng số block đào: ", current_blocks
    );
}

double Blockchain::get_balance(const string& name){
    if(!fs::exists(path_wallet_balance) || fs::file_size(path_wallet_balance) == 0){
        return 0.0;
    }
    ifstream file(path_wallet_balance);
    json balances;
    try {
        file >> balances;
    } catch(...){
        return 0.0;
    };
    if(!balances.contains(name)){
        return 0.0;
    }
    return balances[name].value("balance", 0.0);
}

int Blockchain::get_blocks_mined(const string& name){
    if(!fs::exists(path_wallet_balance) || fs::file_size(path_wallet_balance) == 0){
        return 0;
    }
    ifstream file(path_wallet_balance);
    json balances;
    try {
        file >> balances;
    } catch(...){
        return 0;
    };
    if (!balances.contains(name)) 
        return 0;
    return balances[name].value("blocks_mined", 0);
}