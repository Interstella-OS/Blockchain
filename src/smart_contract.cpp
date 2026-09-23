#include "lib//smart_contract.h"

void SmartContract::resign_transaction(json& transaction) {
    string send_name = transaction["Send name"];
    json data_to_sign = transaction;
    data_to_sign.erase("Signature");   
    string message = data_to_sign.dump();
    ifstream privatefile(path_privatekeys);
    json private_keys = json::parse(privatefile);
    if (!private_keys.contains(send_name) || !private_keys[send_name].contains("private_key")) {
        Log_smartcontract.error(
            "Không tìm thấy private key của: ", send_name, 
            ", không thể ký lại hợp đồng !"
        );
        return; 
    }
    string priv_key = private_keys[send_name]["private_key"];
    transaction["Signature"] = RSAHelper::sign_message(message, priv_key); 
}

json SmartContract::attach_timelock(json transaction, string unlock_time) {
    transaction["contract"] = {
        {"type", "timelock"},
        {"unlock_time", unlock_time}
    };
    resign_transaction(transaction); 
    Log_smartcontract.info(
        "Đã khóa giao dịch thời gian\n", transaction.dump(4),
        "\nHạn lúc:", unlock_time
    );
    return transaction;
}

json SmartContract::attach_escrow(json transaction, string arbiter_name) {
    transaction["contract"] = {
        {"type", "escrow"},
        {"arbiter", arbiter_name},
        {"approved", false}
    };
    resign_transaction(transaction); 
    Log_smartcontract.info(
        "Đã khóa giao dịch ký quỹ\n", transaction.dump(4),
        "\nKhóa bởi: ", arbiter_name
    );
    return transaction;
}

bool SmartContract::approve_escrow(json& transaction, string arbiter_name) {
    if (!transaction.contains("contract")){
        Log_smartcontract.warning(
            "Giao dịch không có hợp đồng, bỏ qua duyệt"
        );
        return false;        
    } 
    if (transaction["contract"]["type"] != "escrow"){
        Log_smartcontract.warning(
            "Giao dịch không phải loại ký quỹ, bỏ qua duyệt"
        );
        return false;
    } 
    if (transaction["contract"]["arbiter"] != arbiter_name) {
        print_info();
        cout << "Hợp đồng" << arbiter_name << " không phải người ký quỹ của giao dịch này !" << endl;
        return false;
    }
    transaction["contract"]["approved"] = true;
    resign_transaction(transaction);
    Log_smartcontract.success(
        "Đã duyệt hợp đồng ký quỹ, mã giao dịch: ", transaction["Transaction code"],
        "\nNgười duyệt: ", arbiter_name
    );
    return true;
}

bool SmartContract::is_ready(const json& transaction) {
    if (!transaction.contains("contract")) return true;
    string type = transaction["contract"]["type"];
    if (type == "timelock") {
        string unlock_time = transaction["contract"]["unlock_time"];
        auto now = datetime::now();
        auto unlock = datetime::strptime(unlock_time, "%H:%M:%S - %d/%m/%Y");
        return now.to_time_point() >= unlock.to_time_point();
    }
    if (type == "escrow") {
        return transaction["contract"]["approved"] == true;
    }
    return true;
}

string SmartContract::describe(const json& transaction) {
    if (!transaction.contains("contract")){
        return "Giao dịch thường";        
    } 
    string type = transaction["contract"]["type"];
    if (type == "timelock") {
        string unlock_time = transaction["contract"]["unlock_time"];
        return "[Timelock] Mở khóa lúc: " + unlock_time
             + (is_ready(transaction) ? " (Đã tới hạn)" : " (Chưa tới hạn)");
    }
    if (type == "escrow") {
        string arbiter = transaction["contract"]["arbiter"];
        bool approved = transaction["contract"]["approved"];
        return "[Escrow] Người ký quỹ: " + arbiter + (approved ? " (Đã duyệt)" : " (Chờ duyệt)");
    }
    return "Hợp đồng không xác định !";
}