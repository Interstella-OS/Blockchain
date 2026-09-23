#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #define NOMINMAX
    #include <winsock2.h>
    #include <ws2tcpip.h>
#endif
#include "lib//header.h"
#include "lib//user_blockchain.h"
#include "lib//transaction_manager.h"
#include "lib//blockchain.h"
#include "lib//p2p.h"
#include "lib//color.h"
#include "lib//utils.h"
#include "lib//smart_contract.h"