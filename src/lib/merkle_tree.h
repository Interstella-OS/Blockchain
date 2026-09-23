#ifndef MERKLE_TREE_H
#define MERKLE_TREE_H

#include "header.h"

struct MerkleProofStep {
    string hash;
    bool is_left;
};

class MerkleTree {
public:
    static string hash_sha256_merkletree(const json& transaction);
    static string node_hash_merkletree(const string& left, const string& right);
    static string create_merkleroot(const json& block_data);
    static string get_leaf_hash(const json& block_data, const string& tx_code);
    static vector<MerkleProofStep> get_merkle_proof(const json& block_data, const string& tx_code);
    static bool verify_merkle_proof(const string& leaf_hash, const vector<MerkleProofStep>& proof, const string& merkle_root);

private:
    static json get_transactions(const json& block_data);
    static string double_sha256(const string& data);
};

#endif