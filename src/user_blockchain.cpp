#include "lib//user_blockchain.h"

void UserBlockchain::save_all_user_keys() {
    RSAHelper::generate_and_save_keys(names, path_publickeys, path_privatekeys);
}

void UserBlockchain::check_and_initialize_keys() {
    if (!fs::exists(path_data)) {
        string message_warning = "[Hệ thống] Không tìm thấy thư mục data. Đang tiến hành tạo thư mục " + path_data;
        Log_user.warning(message_warning); 
        print_warning();
        cout << message_warning << endl;
        fs::create_directories(path_data);
    }

    auto ensure_json_file_exists = [](const string& file_path) {
        if (!fs::exists(file_path) || fs::file_size(file_path) == 0) {
            ofstream create_file(file_path);
            create_file << "{}"; 
            create_file.close();
        }
    };

    ensure_json_file_exists(path_publickeys);
    ensure_json_file_exists(path_privatekeys);

    bool need_regenerate = false;
    ifstream publickeys_check(path_publickeys);
    ifstream privatekeys_check(path_privatekeys);
    if (!publickeys_check.good() || !privatekeys_check.good()) {
        need_regenerate = true;
    } else {
        try {
            json pub_json, priv_json;
            publickeys_check >> pub_json;
            privatekeys_check >> priv_json;
            for (const string& name : names) {
                if (!pub_json.contains(name) || !pub_json[name].contains("public_key")) {
                    need_regenerate = true;
                    break;
                }
                if (!priv_json.contains(name) || !priv_json[name].contains("private_key")) {
                    need_regenerate = true;
                    break;
                }
            }
        } catch (...) {
            need_regenerate = true;
        }
    }

    if (publickeys_check.is_open()){
        publickeys_check.close();        
    } 
    if (privatekeys_check.is_open()){
        privatekeys_check.close();
    } 
    if (need_regenerate) {
        string message_warning = "[Hệ thống] Khóa người dùng bị thiếu hoặc không đầy đủ. Đang khởi tạo lại cặp khóa mới";
        Log_user.warning(message_warning);
        print_warning();
        cout << message_warning << endl;
        save_all_user_keys();
        string message_success = "[Hệ thống] Đã khởi tạo thành công cặp khóa mới tại thư mục " + path_data;
        Log_user.success(message_success);
        print_success();
        cout << message_success << endl;
    } else {
        string message_complete = "[Hệ thống] Kiểm tra tính toàn vẹn hoàn tất. Tất cả khóa người dùng đều hợp lệ";
        Log_user.success(message_complete);
        print_success();
        cout << message_complete << endl;
    }
}