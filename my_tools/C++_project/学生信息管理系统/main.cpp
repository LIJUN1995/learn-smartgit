#include <iostream>
#include "main.h"
#include "StudentManage.h"
#include "StudentView.h"

int main() {
    map<int, Students> mstu;
    StudentView stuView;
    int condition = 0;

    stuView.welcome();
    while (1)
    {
        cout<<"请输入操作序号：";
        cin>>condition;
        switch (condition)
        {
        case 1:
            stuView.addStuView(mstu);
            break;
        
        case 2:
            stuView.deleteStuView(mstu);
            break;

        case 3:
            stuView.updateStuView(mstu);
            break;

        case 4:
            stuView.findStuByIdview(mstu);
            break;

        case 5:
            stuView.showMenu(mstu);
            break;
        
        default:
            cout<<"命令错误"<<endl;
            stuView.welcome();
            break;
        }
    }
    

    return 0;
}
