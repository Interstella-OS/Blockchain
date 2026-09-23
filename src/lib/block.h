#ifndef BLOCK_H
#define BLOCK_H

#include "header.h"

class Block {
private:
    int block_id;
    json block_data;
    string block_prev_hash;
    string block_curr_hash;
    string block_time;
    int block_nonce;
    int mining_time;
    bool isvalid;
    string block_merkle_root;

public:
    Block(json data);
    string merkle_root() const;
    string hash_sha256();

    friend class Blockchain;
};

#endif 
