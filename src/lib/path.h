#ifndef PATH_H
#define PATH_H

#include "string"
using namespace std;

// Path rsa key 
const string path_data                          = "..\\data\\"; 
const string path_publickeys                    = "..\\data\\public_keys.json";
const string path_privatekeys                   = "..\\data\\private_keys.json";

// Path valid block
const string path_folder_valid_block            = "..\\validblock\\";
const string path_valid_block                   = "..\\validblock\\validblock.json"; 

// Path logging 
const string path_logging_blockchain            = "..\\log\\blockchain.log";
const string path_logging_p2p                   = "..\\log\\p2p.log";
const string path_logging_transaction           = "..\\log\\transaction.log";
const string path_logging_user                  = "..\\log\\user.log";
const string path_logging_rewardminer           = "..\\log\\rewardminer.log";

// Path smart contract 
const string path_logging_smart_contract        = "..\\log\\smartcontract.log"; 

// Path wallet balance 
const string path_wallet_balance                = "..\\data\\wallet_balance.json"; 

#endif