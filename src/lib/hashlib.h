//  _    _           _____ _    _ _      _____ ____  
// | |  | |   /\    / ____| |  | | |    |_   _|  _ \ 
// | |__| |  /  \  | (___ | |__| | |      | | | |_) |
// |  __  | / /\ \  \___ \|  __  | |      | | |  _ < 
// | |  | |/ ____ \ ____) | |  | | |____ _| |_| |_) |
// |_|  |_/_/    \_\_____/|_|  |_|______|_____|____/ 

////////////////////////////////////////////////////
//  HASHLIB REDUCED FROM OPENSSL BY TRUONG CHINH  //
////////////////////////////////////////////////////

#ifndef HASHLIB_HPP
#define HASHLIB_HPP

#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <fstream>
#include <algorithm>

namespace hashlib {

// ================== Utils ==================
inline std::string to_hex(const std::vector<unsigned char>& data) {
    std::stringstream ss;
    for (auto c : data)
        ss << std::hex << std::setw(2)
           << std::setfill('0') << (int)c;
    return ss.str();
}

inline std::string normalize(std::string name) {
    std::transform(name.begin(), name.end(), name.begin(), ::tolower);
    return name;
}

// ================== Hash Class ==================
class Hash {
private:
    EVP_MD_CTX* ctx;
    const EVP_MD* md;
    bool finalized;
    std::vector<unsigned char> result;

public:
    explicit Hash(std::string name) : finalized(false) {
        name = normalize(name);
        md = EVP_get_digestbyname(name.c_str());
        if (!md) throw std::runtime_error("Unknown hash: " + name);

        ctx = EVP_MD_CTX_new();
        if (!ctx) throw std::runtime_error("CTX alloc failed");

        if (EVP_DigestInit_ex(ctx, md, nullptr) != 1)
            throw std::runtime_error("Init failed");
    }

    // ===== move only =====
    Hash(const Hash&) = delete;
    Hash& operator=(const Hash&) = delete;

    Hash(Hash&& other) noexcept {
        ctx = other.ctx;
        md = other.md;
        finalized = other.finalized;
        result = std::move(other.result);
        other.ctx = nullptr;
    }

    Hash& operator=(Hash&& other) noexcept {
        if (this != &other) {
            EVP_MD_CTX_free(ctx);
            ctx = other.ctx;
            md = other.md;
            finalized = other.finalized;
            result = std::move(other.result);
            other.ctx = nullptr;
        }
        return *this;
    }

    ~Hash() {
        if (ctx) EVP_MD_CTX_free(ctx);
    }

    // ===== update =====
    Hash& update(const std::string& data) {
        if (finalized) throw std::runtime_error("Already finalized");
        if (EVP_DigestUpdate(ctx, data.data(), data.size()) != 1)
            throw std::runtime_error("Update failed");
        return *this;
    }

    // ===== finalize =====
    void finalize() {
        if (!finalized) {
            unsigned char hash[EVP_MAX_MD_SIZE];
            unsigned int len = 0;

            if (EVP_DigestFinal_ex(ctx, hash, &len) != 1)
                throw std::runtime_error("Finalize failed");

            result.assign(hash, hash + len);
            finalized = true;
        }
    }

    // ===== outputs =====
    std::string hexdigest() {
        finalize();
        return to_hex(result);
    }

    std::vector<unsigned char> digest() {
        finalize();
        return result;
    }

    // ===== reset =====
    void reset() {
        finalized = false;
        result.clear();
        if (EVP_DigestInit_ex(ctx, md, nullptr) != 1)
            throw std::runtime_error("Reset failed");
    }
};

// ================== Factory ==================
inline Hash new_(const std::string& name) {
    return Hash(name);
}

// ================== Shortcut ==================
#define HASH_SHORTCUT(name) \
inline Hash name(const std::string& s) { \
    Hash h(#name); \
    h.update(s); \
    return h; \
}

HASH_SHORTCUT(md5)
HASH_SHORTCUT(sha1)
HASH_SHORTCUT(sha224)
HASH_SHORTCUT(sha256)
HASH_SHORTCUT(sha384)
HASH_SHORTCUT(sha512)

//SHA3
inline Hash sha3_224(const std::string& s) {
    Hash h("sha3-224");
    h.update(s);
    return h;
}

inline Hash sha3_256(const std::string& s) {
    Hash h("sha3-256");
    h.update(s);
    return h;
}

inline Hash sha3_384(const std::string& s) {
    Hash h("sha3-384");
    h.update(s);
    return h;
}

inline Hash sha3_512(const std::string& s) {
    Hash h("sha3-512");
    h.update(s);
    return h;
}

// BLAKE2
inline Hash blake2b(const std::string& s) {
    Hash h("blake2b512");
    h.update(s);
    return h;
}

inline Hash blake2s(const std::string& s) {
    Hash h("blake2s256");
    h.update(s);
    return h;
}

#define HASH_SHORTCUT_CUSTOM(func_name, algo_name) \
inline Hash func_name(const std::string& s) { \
    Hash h(algo_name); \
    h.update(s); \
    return h; \
}

// HASH_SHORTCUT_CUSTOM(sha3_224, "sha3-224")
// HASH_SHORTCUT_CUSTOM(sha3_256, "sha3-256")
// HASH_SHORTCUT_CUSTOM(sha3_384, "sha3-384")
// HASH_SHORTCUT_CUSTOM(sha3_512, "sha3-512")

// HASH_SHORTCUT_CUSTOM(blake2b, "blake2b512")
// HASH_SHORTCUT_CUSTOM(blake2s, "blake2s256")

// ================== Hash File ==================
inline std::string hash_file(const std::string& path, const std::string& algo="sha256") {
    std::ifstream file(path, std::ios::binary);
    if (!file) throw std::runtime_error("Cannot open file");

    Hash h(algo);
    char buffer[4096];

    while (file.read(buffer, sizeof(buffer)))
        h.update(std::string(buffer, file.gcount()));

    if (file.gcount() > 0)
        h.update(std::string(buffer, file.gcount()));

    return h.hexdigest();
}

// ================== HMAC ==================
inline std::string hmac(const std::string& key,
                        const std::string& data,
                        const std::string& algo="sha256") {
    const EVP_MD* md = EVP_get_digestbyname(algo.c_str());
    if (!md) throw std::runtime_error("Unknown algo");

    unsigned char result[EVP_MAX_MD_SIZE];
    unsigned int len = 0;

    HMAC(md,
         key.data(), key.size(),
         reinterpret_cast<const unsigned char*>(data.data()), data.size(),
         result, &len);

    return to_hex(std::vector<unsigned char>(result, result + len));
}

} // namespace hashlib

#endif