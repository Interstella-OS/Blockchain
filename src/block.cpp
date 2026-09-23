#include "lib//block.h"
#include "lib//merkle_tree.h"

Block::Block(json data) {
    this->block_data = data;
    this->block_merkle_root = MerkleTree::create_merkleroot(data);
    this->block_id = 0;
    this->block_nonce = 0;
    this->isvalid = false;
    this->mining_time = 0;
}

string Block::merkle_root() const {
    return this->block_merkle_root;
}

string Block::hash_sha256() {
    string hash_data = to_string(this->block_id)
                     + this->block_data.dump()
                     + this->block_merkle_root
                     + this->block_prev_hash
                     + this->block_time
                     + to_string(this->block_nonce);
    return sha256(hash_data).hexdigest();
}