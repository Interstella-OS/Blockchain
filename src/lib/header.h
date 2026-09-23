//  ____  _            _        _           _       
// |  _ \| |          | |      | |         (_)      
// | |_) | | ___   ___| | _____| |__   __ _ _ _ __  
// |  _ <| |/ _ \ / __| |/ / __| '_ \ / _` | | '_ \    Demo mining blockchain C++
// | |_) | | (_) | (__|   < (__| | | | (_| | | | | |   Nâng cấp P2P và RSA
// |____/|_|\___/ \___|_|\_\___|_| |_|\__,_|_|_| |_|   Tác giả: Nguyễn Trường Chinh
// 
// Bản quyền: MIT LICENSE 2026

#ifndef HEADER_H
#define HEADER_H

#include <iostream>
#include <vector>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <thread>
#include <atomic>
#include <conio.h>
#include <mutex>
#include <filesystem>

// thư viện tóm tắt dùng hash trong openssl gọn như python
// xem hướng dẫn và tải tại: https://github.com/trgchinhh/hashlib-cpp
#include "hashlib.h"

// Thư viện phụ dùng cho json 
#include "json.hpp"

// Thư viện phụ dùng chrono sinh ngày tháng năm 
#include "datetime.h"

// Thư viện phụ dùng cho random 
#include "random.h"

// Thư viện phụ dùng cho uuid
#include "uuid.h"

// Thư viện phụ dùng cho logging
#include "minilog.hpp"

// Thư viện phù dùng lấy path cho sẵn
#include "path.h"

// Thư viện lấy ra banner
#include "banner.h"

// Thư viện Crypto++ cho RSA
// Cài từ Ming64
// Lệnh cài: pacman -S mingw-w64-x86_64-cryptopp
#include <cryptopp/rsa.h>
#include <cryptopp/osrng.h>
#include <cryptopp/base64.h>
#include <cryptopp/hex.h>
#include <cryptopp/pssr.h>

// Thư viện indicator cho progress bar
// Tải thư viện tại: https://github.com/p-ranav/indicators
#include "indicators/progress_bar.hpp"
#include "indicators/cursor_control.hpp"

namespace fs = std::filesystem;
using namespace std;
using namespace dt;
using namespace hashlib;
using namespace random;
using namespace uuids;
using namespace minilog;
using namespace color;
using namespace chrono;
using namespace this_thread;
using namespace CryptoPP;
using namespace indicators;
using namespace indicators::option;
using json = nlohmann::ordered_json;

// Khởi tạo log (không in log ra terminal)
inline Minilog Log_blockchain       = Minilog(path_logging_blockchain, loglevel::TRACE, false);
inline Minilog Log_p2p              = Minilog(path_logging_p2p, loglevel::TRACE, false);
inline Minilog Log_transaction      = Minilog(path_logging_transaction, loglevel::TRACE, false);
inline Minilog Log_user             = Minilog(path_logging_user, loglevel::TRACE, false);
inline Minilog Log_smartcontract    = Minilog(path_logging_smart_contract, loglevel::TRACE, false); 
inline Minilog Log_rewardminer      = Minilog(path_logging_rewardminer, loglevel::TRACE, false);

#endif