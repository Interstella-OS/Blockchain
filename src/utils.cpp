#include "lib//utils.h"

const vector<string> names = {
    "Truong Chinh", "Truong An", "Duy Minh", "Duy Quang", "Ngoc Tu",
    "Thanh Phuc", "Manh Hung", "The An", "Thien An", "Gia Bao", 
    "Khanh Dang", "Hai Dang", "Khanh Duy", "Bao Khanh", "Manh Cuong",
    "Minh Khoi", "Hoang Long", "Quoc Anh", "Bao Long", "Gia Huy",
    "Tuan Anh", "Duc Anh", "Huu Phuoc", "Minh Triet", "Viet Anh",
    "Thanh Tung", "Cong Vinh", "Van Duc", "Quang Hai", "Dinh Trong",
    "Minh Quan", "Nhat Minh", "Gia Minh", "Huu Thang", "Minh Duc",
    "Tien Dat", "Xuan Truong", "Hoang Minh", "Thanh Nam", "Duc Luong",
    "Bao Nam", "Anh Dung", "Huu Dat", "Chi Kien", "Dang Khoa",
    "Hoang Nam", "Phi Long", "Gia Khiem", "Trung Kien", "Bao Khang",
    "Phuc Lam", "Ha Long", "Gia Binh", "Duc Khiem", "Minh Nhat",
    "Tien Dung", "Quang Minh", "Hung Cuong", "Huu Trong", "Minh Hieu",
    "Tuan Kiet", "Tran Minh", "Hoang An", "Thien Nhan", "Khanh Linh"
};

string time_now() {
    return datetime::now().strftime("%H:%M:%S - %d/%m/%Y");
}

void loading_bar(int seconds) {
    show_console_cursor(false);
    cout << endl;
    ProgressBar bar{
        BarWidth{40},
        Start{"["},
        Fill{"#"},
        Lead{"#"},
        Remainder{"-"},
        End{"]"},
        ShowPercentage{true},
        PrefixText{"Đang kết nối "},
    };
    const int total = seconds * 25;
    for(int i = 0; i <= total; i++){
        bar.set_progress(i*100/total);
        this_thread::sleep_for(milliseconds(40));
    }
    show_console_cursor(true);
}

void pause_program(){
    cout << "\nNhấn phím bất kỳ để tiếp tục..." << endl;
    getch();
}

void cin_ignore(){
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

// void main_menu(string& input){
//     cout << "[Menu]" << endl;
//     cout << "[01] Tạo/phát tán giao dịch" << endl;
//     cout << "[02] Kiểm tra hàng chờ" << endl;
//     cout << "[03] Bắt đầu đào block" << endl;
//     cout << "[04] Xem lịch sử chuỗi" << endl;
//     cout << "[05] Tạo hợp đồng thời gian" << endl;
//     cout << "[06] Tạo hợp đồng ký quỹ" << endl;
//     cout << "[07] Duyệt hợp đồng ký quỹ" << endl;
//     cout << "[08] Thoát" << endl;
//     cout << "\n[?] Nhập lựa chọn: ";
//     getline(cin, input);
    
// }

// void main_menu(string& input){
//         cout << "[Menu]" << endl;
//         cout << "[01] Tạo/phát tán giao dịch" << endl;
//         cout << "[02] Kiểm tra hàng chờ" << endl;
//         cout << "[03] Bắt đầu đào block" << endl;
//         cout << "[04] Xem lịch sử chuỗi" << endl;
//         cout << "[05] Duyệt hợp đồng ký quỹ" << endl;
//         cout << "[06] Thoát" << endl;
//         cout << "\n[?] Nhập lựa chọn: ";
//         getline(cin, input);
// }

// void choice_1_menu(int& choice_1_menu){
//     cout << "[Menu]" << endl;
//     cout << "[01] Tạo giao dịch ngẫu nhiên" << endl;
//     cout << "[02] Tạo hợp đồng thời gian" << endl;
//     cout << "[03] Tạo hợp đồng ký quỹ" << endl;
//     cout << "[04] Quay lại" << endl;
//     cout << "\n[?] Nhập lựa chọn: ";
// }