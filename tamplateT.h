#ifndef TEMPLATE_H_
#define TEMPLATE_H_
#include <iostream>
using namespace std;

template<class T>
class Test {
public:
	Test(const T& t) :data(t) {}

	//���������������ʵ��
	friend ostream& operator<<(ostream& out, Test<T>& t) {
		return out << "data is :" << t.data;
	} 
	//����������������ʵ��
	friend istream& operator>>(istream& in, Test<T>& t) {
		return in >> t.data;
	}
private:
	T data;
};    
#endif // !TEMPLATE_H_  