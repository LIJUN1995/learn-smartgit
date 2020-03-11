#include <iostream>
#include "main.h"
#include "Student.h"

// 学生默认id是-1，说明这暂时是一个无效的学生。

Students::Students()
{
    this->id = -1;
}

Students::~Students()
{
    
}

int Students::GetId() const { return id; }

void Students:: SetId(int val) { id = val; }

string Students:: GetName() const { return name; }

void Students:: SetName(string val) { name = val; }

int Students:: GetAge() const { return age; }

void Students:: SetAge(int val) { age = val; }

 

 

ostream & operator<<(ostream & os, Students & stu) 
{
    os<<stu.GetId()<<"     "<<stu.GetName()<<"     "<<stu.GetAge();

    return os;
}

 

istream & operator>>(istream & is, Students & stu) 
{
    cout << "id:";
    is>>stu.id;
    cout << "name:";
    is>>stu.name;
    cout << "age:";
    is>>stu.age;

    return is;
}