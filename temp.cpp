#include <iostream>
using namespace std;

void test(int &t){
    cout << "int&" << endl;
}
void test(const int &t){
    cout << "const int &t" << endl;
}
void test(int &&t){
    cout << "int &&t" << endl;
}
void test(const int &&t){
    cout << "const int &&t" << endl;
}
void forward(int &&t){
    test(t);
}

int main(){
    int t=0;
    forward(move(t));
    return 0;
}