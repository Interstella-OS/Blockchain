#include "lib//transaction_manager.h"

string TransactionManager::clean_key(string key) {
    if(key.length() > 30) {
        return key.substr(0, 15) + "..." + key.substr(key.length() - 15);
    }
    return key;
}

string TransactionManager::create_uuid_from_data(string data){
    uuid_name_generator gen(uuid_namespace_dns);
    return to_string(gen(data));
}

json TransactionManager::create_transaction() {
    json transactions_list = json::array();
    for(int i = 0; i < MAX_TS_INIT; i++){
        string send_name = choice(names);
        string receive_name = choice(names);
        while (send_name == receive_name) {
            receive_name = choice(names);
        }
        ifstream publicfile(path_publickeys);
        json public_keys = json::parse(publicfile);
        
        string sender_public_key = public_keys[send_name]["public_key"];
        string receiver_public_key = public_keys[receive_name]["public_key"];
        double amount = round(uniform(100000, 10000000) * 1000.0) / 1000.0;
        double fee = round((amount * 0.005) * 1000000.0) / 1000000.0;
        string t_time = time_now();
        string code_data = send_name + receive_name + to_string(amount) + t_time;
        string ts_code = md5(code_data).hexdigest(); 
        string ts_data = sender_public_key + receiver_public_key 
                        + to_string(amount) + to_string(fee) + t_time 
                        + ts_code; 
        string ts_uuid = create_uuid_from_data(ts_data);

        json ts_data_json;
        ts_data_json["Send name"] = send_name;
        ts_data_json["Id send"] = clean_key(sender_public_key);
        ts_data_json["Receive name"] = receive_name;
        ts_data_json["Id receive"] = clean_key(receiver_public_key);
        ts_data_json["Amount"] = amount;
        ts_data_json["Fee"] = fee;
        ts_data_json["Transaction code"] = ts_code;
        ts_data_json["Time"] = t_time;
        ts_data_json["Uuid"] = ts_uuid;
        
        ifstream privatefile(path_privatekeys);
        json private_keys = json::parse(privatefile);
        string send_priv_b64 = private_keys[send_name]["private_key"];
        string message = ts_data_json.dump(); 
        ts_data_json["Signature"] = RSAHelper::sign_message(message, send_priv_b64);
        Log_transaction.info(
            "Khởi tạo giao dịch\n",
            "Từ: ", send_name, " ID: ", clean_key(sender_public_key),
            "Đến: ", receive_name, " ID: ", clean_key(receiver_public_key),
            "Số tiền: ", amount, " Phí: ", to_string(fee) + "\n",
            "Mã giao dịch: ", ts_code + "\n",
            "Uuid: ", ts_uuid + "\n\n"
        );
        transactions_list.push_back(ts_data_json);
    }
    
    return transactions_list;
}

bool TransactionManager::verify_transaction(json transaction) {
    string send_name = transaction["Send name"];
    string hex_signature = transaction["Signature"];
    json verify_data = transaction;
    verify_data.erase("Signature");
    string message = verify_data.dump();
    ifstream pubfile(path_publickeys);
    json public_keys = json::parse(pubfile);
    string send_pub_b64 = public_keys[send_name]["public_key"];
    return RSAHelper::verify_signature(message, hex_signature, send_pub_b64);
}