#ifndef TRANSACTION_MANAGER_H
#define TRANSACTION_MANAGER_H

#include "header.h"
#include "utils.h"
#include "rsa.h"
#include "path.h"

// số giao dịch tối đa 1 lần khởi tạo 
const int MAX_TS_INIT = 3;

class TransactionManager {
public:
    string clean_key(string key);
    string create_uuid_from_data(string data);
    json create_transaction();
    bool verify_transaction(json transaction);
};

#endif 