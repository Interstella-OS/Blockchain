#ifndef SMART_CONTRACT_H
#define SMART_CONTRACT_H

#include "header.h"
#include "rsa.h"
#include "path.h"
#include "color.h"

class SmartContract {
private:
    static void resign_transaction(json& transaction);
public:
    static json attach_timelock(json transaction, string unlock_time);
    static json attach_escrow(json transaction, string arbiter_name);
    static bool approve_escrow(json& transaction, string arbiter_name);
    static bool is_ready(const json& transaction);
    static string describe(const json& transaction);
};

#endif