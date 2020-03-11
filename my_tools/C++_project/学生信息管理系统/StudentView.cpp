#include <iostream>
#include "StudentView.h"
#include "StudentManage.h"

StudentView::StudentView(/* args */)
{
}

StudentView::~StudentView()
{
}

void StudentView::welcome()
{
    system("clear");
    cout<<"*******************************"<<endl;
    cout<<"*请选择以下操作的序号：         "<<endl;
    cout<<"*    1，添加学生               "<<endl;
    cout<<"*    2，删除学生               "<<endl;
    cout<<"*    3，修改学生信息            "<<endl;
    cout<<"*    4，查询学生信息            "<<endl;
    cout<<"*    5，显示所有学生信息        "<<endl;
    cout<<"*******************************"<<endl;
    
}
        
void StudentView::showMenu(map<int, Students>& Mstu)
{
    cout<<"ID"<<"    "<<"NAME"<<"     "<<"AGE"<<endl;
    for(auto p : Mstu)
    {
        cout<<p.second<<endl;
    }
}

void StudentView::addStuView(map<int, Students>& Mstu)
{
    StudentManage stuMan;
    Students stu;

    cout<<"请输入学生信息"<<endl;
    cin>>stu;
    stuMan.AddStudent(Mstu, stu);

    welcome();

    cout<<"                       "<<endl;
    cout<<"                       "<<endl;
    cout<<"********添加成功********"<<endl;
}

void StudentView::deleteStuView(map<int, Students>& Mstu)
{
    StudentManage stuMan;
    int id = 0;
    bool status;

    cout<<"请输入学生ID"<<endl;
    cin>>id;
    status = stuMan.DeleteStudent(Mstu, id);
    welcome();
    if(status){
        cout<<"                       "<<endl;
        cout<<"                       "<<endl;
        cout<<"********删除成功********"<<endl;
    }else{
        cout<<"                       "<<endl;
        cout<<"                       "<<endl;
        cout<<"********查无此人********"<<endl;
    }
}

void StudentView::updateStuView(map<int, Students>& Mstu)
{
    StudentManage stuMan;
    Students stu;

    cout<<"请输入更新学生信息"<<endl;
    cin>>stu;
    stuMan.UpdateStudent(Mstu, stu);

    welcome();

    cout<<"                       "<<endl;
    cout<<"                       "<<endl;
    cout<<"********更新成功********"<<endl;
}

void StudentView::findStuByIdview(map<int, Students>& Mstu)
{
    StudentManage stuMan;
    Students stu;
    int id = 0;

    cout<<"请输入查找学生ID"<<endl;
    cin>>id;
    stu = stuMan.FindStudentById(Mstu, id);
    welcome();
    if(stu.GetId() != -1){
        cout<<"ID"<<"    "<<"NAME"<<"     "<<"AGE"<<endl;
        cout<<stu<<endl;
    }else{
        cout<<"                       "<<endl;
        cout<<"                       "<<endl;
        cout<<"********查无此人********"<<endl; 
    }   
}