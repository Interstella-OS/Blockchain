#ifndef BLOCKCHAIN_H
#define BLOCKCHAIN_H

#include "header.h"
#include "block.h"
#include "utils.h"
#include "color.h"

// tiền thưởng khi đào được 1 block
const double MINING_REWARD = 0.005;

class Blockchain {
private:
    int difficulty;
    vector<Block> chain;
    string str_difficulty;

public:
    vector<json> mempool;
    atomic<bool> stop_mining{false};
    atomic<bool> mining_in_progress{false};
    mutex state_mutex; 

    string owner_name;

    int total_time = 0;
    Blockchain(int difficulty);

    void genesis_block();
    bool add_block(json data); 
    bool accept_external_block(json block_json); 
    bool isvalid_block(Block block, Block prev_block);
    bool isvalid_chain();
    bool is_ts_in_mempool(string tscode);
    void add_transaction_to_mempool(json tx);
    void remove_mined_transactions(const json& block_data);
    void print_block(Block block);
    void print_accepted_block(Block block);
    void print_history();
    json get_all_blocks_json();
    json get_last_block_json();
    void sync_chain(json chain_json);
    void save_validblock(Block block);

    void reward_miner(const string& miner_name, double amount);
    double get_balance(const string& name);
    int get_blocks_mined(const string& name);
};

#endif 
