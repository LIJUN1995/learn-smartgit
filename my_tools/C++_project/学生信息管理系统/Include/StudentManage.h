#ifndef __STUDENTMANAGE_H__
#define __STUDENTMANAGE_H__

#include <map>
#include "Student.h"

class StudentManage {
    public:
        StudentManage();

        ~StudentManage();

        Students AddStudent(map<int, Students>& Mstu, Students& Stu);

        bool DeleteStudent(map<int, Students>& Mstu, const int& Id);

        Students UpdateStudent(map<int, Students>& Mstu, const Students& Stu);

        Students FindStudentById(map<int, Students>& Mstu, const int& Id) const;
};

#endif