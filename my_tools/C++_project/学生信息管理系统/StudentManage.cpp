 #include "StudentManage.h"

 StudentManage::StudentManage()
 {

 }

StudentManage::~StudentManage()
{

}

Students StudentManage::AddStudent(map<int, Students>& Mstu, Students& Stu)
{
    Mstu.insert(make_pair(Stu.GetId(),Stu));

    return Stu;
}

bool StudentManage::DeleteStudent(map<int, Students>& Mstu, const int& Id)
{
    map<int, Students>::iterator iter;
    iter = Mstu.find(Id);
    if(iter != Mstu.end())
    {
        Mstu.erase(iter);

        return true;
    }

    return false;
}

Students StudentManage::UpdateStudent(map<int, Students>& Mstu, const Students& Stu)
{
    map<int, Students>::iterator iter;
    iter = Mstu.find(Stu.GetId());
    if(iter != Mstu.end())
    {
        iter->second = Stu;
    }

    return Stu;
}

Students StudentManage::FindStudentById(map<int, Students>& Mstu, const int& Id) const
{
    Students Stu;

    map<int, Students>::iterator iter;
    iter = Mstu.find(Id);
    if(iter != Mstu.end())
    {
        Stu = iter->second;
    }

    return Stu;
}