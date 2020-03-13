#ifndef __STUDENT_H__
#define __STUDENT_H__

#include <string>

using namespace std;

class Students {

    friend ostream & operator<<(ostream & os, Students & stu);
    friend istream & operator>>(istream & is, Students & stu); 

    public:
        Students();
        ~Students();

        int GetId() const;
        void SetId(int val);

        string GetName() const;
        void SetName(string val);

        int GetAge() const;
        void SetAge(int val);

    private:
        int id;
        int age;
        string name;
};

#endif