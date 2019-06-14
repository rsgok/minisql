//
// Created by wangjianwei on 2019/6/12.
//

#ifndef RECORDMANAGE_WEIRD_RM_H
#define RECORDMANAGE_WEIRD_RM_H
#include "base.h"
#include "const.h"
#include "MyBufferManager.hpp"
class RecordManager
{
public:
    RecordManager(BufferManager *bf):buf_ptr(bf){}//
    ~RecordManager();//析构函数
    bool isSatisfied(Table&, tuper&, vector<int> mask, vector<where> w);//判断单元组是否满足输入条件
    Table Select(Table&, vector<int>attrSelect, vector<int>mask, vector<where>& w);//有条件选择
    Table Select(Table&, vector<int>attrSelect);//无条件选择
    bool CreateIndex(Table&, int attr);//创建索引
    int FindWithIndex(Table&, tuper& row, int mask);//通过索引查找
    void Insert(Table&, tuper& singleTuper);//插入数据
    void InsertWithIndex(Table&, tuper& singleTuper);//通过索引插入
    char* Tuper2Char(Table&, tuper& singleTuper);//将一条数据元组转换为字符类型类型，用于数据存储
    int Delete(Table&, vector<int>mask, vector<where> w);//删除有条件
    bool DropTable(Table&);//删除一张表
    bool CreateTable(Table&);//创建一张表
    Table Project(Table&, vector<int>attrSelect);//将选择出来的元组进行删除，使得具有特定的属性
    tuper String2Tuper(Table&, string stringRow);//将字符串转换为元组，用于数据的读取
    bool UNIQUE(Table&, where w, int loca);
private:
    RecordManager(){} //产生rm的时候必须把bufferManager的指针赋值给它
    BufferManager *buf_ptr;//
};
#endif //RECORDMANAGE_WEIRD_RM_H
