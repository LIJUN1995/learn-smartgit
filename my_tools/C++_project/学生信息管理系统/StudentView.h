#ifndef __STUDENTVIEW_H__
#define __STUDENTVIEW_H__

#include <map>
#include "Student.h"

class StudentView
{
    public:
        StudentView(/* args */);

        ~StudentView();

        void welcome();
        
        void showMenu(map<int, Students>& Mstu);

        void addStuView(map<int, Students>& Mstu);

        void deleteStuView(map<int, Students>& Mstu);
        
        void updateStuView(map<int, Students>& Mstu);

        void findStuByIdview(map<int, Students>& Mstu);
};

#endif