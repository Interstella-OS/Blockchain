#include "lib//color.h"

void print_error(){
    cout << brightred << "\n[Lỗi] " << reset;
}

void print_success(){
    cout << brightgreen << "\n[Thành công] " << reset;
}

void print_warning(){
    cout << brightyellow << "\n[Cảnh báo] " <<  reset;
}

void print_info(){
    cout << brightblue << "\n[Thông tin] " << reset;
}

void print_p2p(){
    cout << gray << "\n[P2P] " << reset;
}