#include "lib//merkle_tree.h"

json MerkleTree::get_transactions(const json& block_data) {
    if (block_data.is_array()) {
        return block_data;
    }
    if (block_data.is_object()
        && block_data.contains("transactions")
        && block_data["transactions"].is_array()) {
        return block_data["transactions"];
    }
    return json::array();
}

string MerkleTree::double_sha256(const string& data) {
    return sha256(sha256(data).hexdigest()).hexdigest();
}

string MerkleTree::hash_sha256_merkletree(const json& transaction) {
    return double_sha256(transaction.dump());
}

string MerkleTree::node_hash_merkletree(const string& left, const string& right) {
    return double_sha256(left + right);
}

string MerkleTree::create_merkleroot(const json& block_data) {
    json transactions = get_transactions(block_data);
    if (transactions.empty()) {
        return double_sha256("");
    }
    vector<string> level;
    level.reserve(transactions.size());
    for (const auto& tx : transactions) {
        level.push_back(hash_sha256_merkletree(tx));
    }
    while (level.size() > 1) {
        vector<string> parent;
        parent.reserve((level.size() + 1) / 2);
        for (size_t i = 0; i < level.size(); i += 2) {
            string left = level[i];
            string right = (i + 1 < level.size()) ? level[i + 1] : level[i];
            parent.push_back(node_hash_merkletree(left, right));
        }
        level = move(parent);
    }
    return level[0];
}

string MerkleTree::get_leaf_hash(const json& block_data, const string& tx_code) {
    json transactions = get_transactions(block_data);
    for (const auto& tx : transactions) {
        if (tx.is_object()
            && tx.contains("Transaction code")
            && tx["Transaction code"] == tx_code) {
            return hash_sha256_merkletree(tx);
        }
    }
    return "";
}

vector<MerkleProofStep> MerkleTree::get_merkle_proof(const json& block_data, const string& tx_code) {
    json transactions = get_transactions(block_data);
    vector<MerkleProofStep> proof;
    if (transactions.empty()) {
        return proof;
    }
    vector<string> level;
    level.reserve(transactions.size());
    for (const auto& tx : transactions) {
        level.push_back(hash_sha256_merkletree(tx));
    }
    size_t tx_pos = 0;
    bool found = false;
    for (size_t i = 0; i < transactions.size(); i++) {
        if (transactions[i].is_object()
            && transactions[i].contains("Transaction code")
            && transactions[i]["Transaction code"] == tx_code) {
            tx_pos = i;
            found = true;
            break;
        }
    }
    if (!found) {
        return proof;
    }
    while (level.size() > 1) {
        size_t sibling_pos = (tx_pos % 2 == 0) ? tx_pos + 1 : tx_pos - 1;
        if (sibling_pos >= level.size()) {
            sibling_pos = tx_pos;
        }
        MerkleProofStep step;
        step.hash = level[sibling_pos];
        step.is_left = (sibling_pos < tx_pos);
        proof.push_back(step);
        vector<string> parent;
        parent.reserve((level.size() + 1) / 2);
        for (size_t i = 0; i < level.size(); i += 2) {
            string left = level[i];
            string right = (i + 1 < level.size()) ? level[i + 1] : level[i];
            parent.push_back(node_hash_merkletree(left, right));
        }
        tx_pos /= 2;
        level = move(parent);
    }
    return proof;
}

bool MerkleTree::verify_merkle_proof(const string& leaf_hash, const vector<MerkleProofStep>& proof, const string& merkle_root) {
    string current = leaf_hash;
    for (const auto& step : proof) {
        if (step.is_left) {
            current = node_hash_merkletree(step.hash, current);
        } else {
            current = node_hash_merkletree(current, step.hash);
        }
    }
    return current == merkle_root;
}