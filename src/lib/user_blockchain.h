#ifndef USER_BLOCKCHAIN_H
#define USER_BLOCKCHAIN_H

#include "header.h"
#include "utils.h"
#include "rsa.h"
#include "path.h"
#include "color.h"

class UserBlockchain {
public:
    void save_all_user_keys();
    void check_and_initialize_keys();
};

#endif 