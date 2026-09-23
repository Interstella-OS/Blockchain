#ifndef UTILS_H
#define UTILS_H

#include "header.h"

extern const vector<string> names;

string time_now();
void loading_bar(int seconds);
void pause_program();
void cin_ignore();
void main_menu(string& input);
void choice_1_menu(int& choice_1_input);
#endif