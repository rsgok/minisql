
#include "RM.h"
#include "Interpreter.h"
#include "IndexManager.h"
#include <cmath>
#include "RM.h"

//RM 的析构函数
RecordManager::~RecordManager()
{
}


Table RecordManager::Project(Table& table_in, vector<int>attrSelect)
{
    Attribute attrOut;
    tuper *ptrTuper=NULL;
    attrOut.num = attrSelect.size();
    for (int i = 0; i < attrSelect.size();i++){
        attrOut.flag[i] = table_in.getattribute().flag[attrSelect[i]];
        attrOut.name[i] = table_in.getattribute().name[attrSelect[i]];
        attrOut.unique[i] = table_in.getattribute().unique[attrSelect[i]];
    }
    Table tableOut(table_in.getname(), attrOut, table_in.blockNum);
    int k;
    for (int i = 0; i < table_in.T.size(); i++){//tuper的个数
        ptrTuper = new tuper;
        for (int j = 0; j < attrSelect.size();j++){
            k = attrSelect[j];
            Data *resadd =NULL;
            //return the pointer to a specified data item.
            if (table_in.T[i]->operator [](k)->flag == -1) {
                resadd = new Datai((*((Datai*)table_in.T[i]->operator [](k))).x);
            }
            else if(table_in.T[i]->operator [](k)->flag == 0){
                resadd = new Dataf((*((Dataf*)table_in.T[i]->operator [](k))).x);
            }
            else if (table_in.T[i]->operator [](k)->flag>0) {
                resadd = new Datac((*((Datac*)table_in.T[i]->operator [](k))).x);
            }

            ptrTuper->addData(resadd);//bug

        }
        tableOut.addData(ptrTuper);
    }

    return tableOut;
}

//Check a tuple 是否满足条件
//参数：这个table_infor没啥用！
//mask:用来表示受限的属性
//基本的判断函数，用于后面的Select、
bool RecordManager::isSatisfied(Table& table_infor, tuper& row, vector<int> mask, vector<where> w)
{
    for (int i = 0; i < mask.size();i++){
        if (w[i].d == NULL){ //不存在where条件
            continue;
        }
        else if (row[mask[i]]->flag == -1) { //用-1来代表这一列的属性为一个int，整数。
            switch (w[i].flag) {
                //先强制类型转换，然后再与where中的限制条件进行比较。如果有一个不符合的话，那就返回不匹配
                case eq: if ((((Datai*)row[mask[i]])->x != ((Datai*)w[i].d)->x)) return false;break;
                case leq: if ((((Datai*)row[mask[i]])->x > ((Datai*)w[i].d)->x)) return false; break;
                case l: if ((((Datai*)row[mask[i]])->x >= ((Datai*)w[i].d)->x)) return false; break;
                case geq: if (((Datai*)row[mask[i]])->x < ((Datai*)w[i].d)->x) return false; break;
                case g: if (((Datai*)row[mask[i]])->x <= ((Datai*)w[i].d)->x) return false; break;
                case neq: if (((Datai*)row[mask[i]])->x == ((Datai*)w[i].d)->x) return false; break;
                default: cout << "not qulified condition" <<endl;
                    return false;
                    break;
            }
        }
        else if (row[mask[i]]->flag == 0) { //用0来代表这个值为一个Float类型的数。
            switch (w[i].flag) {
                //道理与上面类似
                case eq: if (abs(((Dataf*)row[mask[i]])->x - ((Dataf*)w[i].d)->x)>=MIN_Theta) return false; break;
                case leq: if (((Dataf*)row[mask[i]])->x > ((Dataf*)w[i].d)->x) return false; break;
                case l: if (((Dataf*)row[mask[i]])->x >= ((Dataf*)w[i].d)->x) return false; break;
                case geq: if (((Dataf*)row[mask[i]])->x < ((Dataf*)w[i].d)->x) return false; break;
                case g: if (((Dataf*)row[mask[i]])->x <= ((Dataf*)w[i].d)->x) return false; break;
                case neq: if (((Dataf*)row[mask[i]])->x == ((Dataf*)w[i].d)->x) return false; break;
                default: cout << "not qulified condition" <<endl;
                    return false;
                    break;
            }
        }
        else if (row[mask[i]]->flag > 0){ //用一个大于0的数来代表这个值为一个字符串类型。
            switch (w[i].flag) {
                case eq: if (!(((Datac*)row[mask[i]])->x == ((Datac*)w[i].d)->x)) return false; break;
                case leq: if (!(((Datac*)row[mask[i]])->x <= ((Datac*)w[i].d)->x)) return false; break;
                case l: if (!(((Datac*)row[mask[i]])->x < ((Datac*)w[i].d)->x)) return false; break;
                case geq: if (!(((Datac*)row[mask[i]])->x >= ((Datac*)w[i].d)->x)) return false; break;
                case g: if (!(((Datac*)row[mask[i]])->x >((Datac*)w[i].d)->x)) return false; break;
                case neq: if (!(((Datac*)row[mask[i]])->x != ((Datac*)w[i].d)->x)) return false; break;
                default:  cout << "not qulified condition" <<endl;
                    return false;
                    break;
            }
        }
        else { //
            cout << "Error:RecordManager::isSatisfied" << endl;
            system("pause");
        }
    }
    return true;
}

Table RecordManager::Select(Table& table_in, vector<int>attrSelect, vector<int>mask, vector<where>& w)
{
    if (mask.size() == 0){
        return Select(table_in,attrSelect);
    }//如果没有限制条件的话
    string row_str;

    string file_name = table_in.getname() + ".table";
    string indexfile_name;
    int length = table_in.dataSize() + 1;//加上一个回车键
    const int record_num = BLOCKSIZE / length;

    for (int blockOffset = 0; blockOffset < table_in.blockNum; blockOffset++){
        int bufferNum = buf_ptr->getIfIsInBuffer(file_name, blockOffset);
        if (bufferNum == -1){//如果没在内存中
            bufferNum = buf_ptr->getEmptyBuffer();
            buf_ptr->readBlock(file_name, blockOffset, bufferNum);
        }
        for (int offset = 0; offset < record_num;offset++){
            int position = offset * length;
            row_str = buf_ptr->bufferBlock[bufferNum].getvalues(position, position + length);
            if (row_str.c_str()[0] == EMPTY) continue;//该行是空的
            int c_pos = 1;//当前在数据流中指针的位置，0表示该位是否有效，因此数据从第一位开始
            tuper *temp_tuper = new tuper;
            for (int attr_index = 0; attr_index < table_in.getattribute().num; attr_index++){
                if (table_in.getattribute().flag[attr_index] == -1){//是一个整数
                    int value;
                    memcpy(&value, &(row_str.c_str()[c_pos]), sizeof(int));
                    c_pos += sizeof(int);
                    temp_tuper->addData(new Datai(value));
                }
                else if (table_in.getattribute().flag[attr_index] == 0){//float
                    float value;
                    memcpy(&value, &(row_str.c_str()[c_pos]), sizeof(float));
                    c_pos += sizeof(float);
                    temp_tuper->addData(new Dataf(value));
                }
                else{
                    char value[MAXSTRINGLEN];
                    int strLen = table_in.getattribute().flag[attr_index]+1;
                    memcpy(value, &(row_str.c_str()[c_pos]), strLen);
                    c_pos += strLen;
                    temp_tuper->addData(new Datac(string(value)));
                }
            }

            if (isSatisfied(table_in,*temp_tuper,mask,w)){
                table_in.addData(temp_tuper); //记得把条件设置好
            }
            else delete temp_tuper;
        }
    }
    return Project(table_in,attrSelect);
}

Table RecordManager::Select(Table& table_in, vector<int>attrSelect)
{
    string row_str;	//读取的时候是用字符串读取的
    string file_name = table_in.getname() + ".table";	//file_name用来表示这个表的名字（因为在内存中是这样子存的）
    tuper* temp_tuper;	//用来存储返回的元组
    int length = table_in.dataSize() + 1; //一个元组的信息在文档中的长度
    const int record_num = BLOCKSIZE / length; //一个block中存储的记录条数
    for (int blockOffset = 0; blockOffset < table_in.blockNum;blockOffset++){//读取整个文件中的所有内容，一块一块读
        int bufferNum = buf_ptr->checkExist(file_name, blockOffset);//看某一块的值在不在buffer中
        if (bufferNum == -1){ //该块不在内存中，从硬盘中读取
            bufferNum = buf_ptr->getUnoccupiedBlock();
            buf_ptr->readBlock(file_name, blockOffset, bufferNum);
        }
        for (int offset = 0; offset < record_num;offset++){//对于一块中的内容，读取每一条的数据（元组）
            int position = offset * length;//这条数据在block中的位置
            row_str = buf_ptr->blocks[bufferNum].getDataStr(position, position + length);//从缓冲区中读取这一块内容
            if(row_str.c_str()[0]==EMPTY) continue;//该行是空的
            int c_pos = 1;//当前在数据流中指针的位置，0表示该位是否有效，因此数据从第一位开始
            temp_tuper = new tuper;
            for (int attr_index = 0; attr_index < table_in.getattribute().num;attr_index++){//对于这个元组的每一列，都操作一下
                if (table_in.getattribute().flag[attr_index] == -1){//是一个整数
                    int value;
                    memcpy(&value, &(row_str.c_str()[c_pos]), sizeof(int));
                    c_pos += sizeof(int);
                    temp_tuper->addData(new Datai(value));
                }
                else if (table_in.getattribute().flag[attr_index]==0){//float
                    float value;
                    memcpy(&value, &(row_str.c_str()[c_pos]), sizeof(float));
                    c_pos += sizeof(float);
                    temp_tuper->addData(new Dataf(value));
                }
                else{
                    char value[MAXSTRINGLEN];
                    int strLen = table_in.getattribute().flag[attr_index]+1;
                    memcpy(value, &(row_str.c_str()[c_pos]), strLen);
                    c_pos += strLen;
                    temp_tuper->addData(new Datac(string(value)));
                }
            }
            table_in.addData(temp_tuper); //可能会存在问题;solved!
        }
    }
    return Project( table_in, attrSelect);
}

int RecordManager::FindWithIndex(Table& table_in, tuper& row, int mask)
{
    IndexManager indexMA;
    for (int i = 0; i < table_in.index.num; i++) {
        if (table_in.index.location[i] == mask) { //找到索引
            Data* ptrData;
            ptrData = row[mask];
            int pos = indexMA.Find(table_in.getname() + to_string(mask) + ".index", ptrData);
            return pos;
        }
    }
    return -1;
}

void RecordManager::Insert(Table& table_in, tuper& singleTuper)
{
    //check Redundancy using index
    for (int i = 0; i < table_in.attr.num; i++) {
        if (table_in.attr.unique[i] == 1) {	//找到主键
            int addr = FindWithIndex(table_in, singleTuper, i);
            if (addr >= 0) { //already in the table
                throw QueryException("Unique Value Redundancy occurs, thus insertion failed");
                return;
            }
        }
    }

    for (int i = 0; i < table_in.attr.num;i++) {
        if (table_in.attr.unique[i]){  //如果这个是uniqeue的话，每一个unique的属性都要判断一下
            vector<where> w;
            vector<int> mask;
            where *uni_w = new where;
            uni_w->flag = eq;
            switch (singleTuper[i]->flag) {
                case -1:uni_w->d = new Datai(((Datai*)singleTuper[i])->x); break;
                case 0:uni_w->d = new Dataf(((Dataf*)singleTuper[i])->x); break;
                default:uni_w->d = new Datac(((Datac*)singleTuper[i])->x); break;
            }
            w.push_back(*uni_w);
            mask.push_back(i);

            if(!UNIQUE(table_in, w[0], i)){//因为每次都只要一个值，判断完之后就没有必要存在了，也就删除了
                throw QueryException("Unique Value Redundancy occurs, thus insertion failed");
            }
            delete uni_w->d;
            delete uni_w;
        }
    }

    char *charTuper;
    charTuper = Tuper2Char(table_in, singleTuper);//把一个元组转换成字符串
    //判断是否unique
    insertPos iPos = buf_ptr->getInsertPosition(table_in);//获取插入位置

    buf_ptr->bufferBlock[iPos.bufferNUM].values[iPos.position] = NOTEMPTY;
    memcpy(&(buf_ptr->bufferBlock[iPos.bufferNUM].values[iPos.position + 1]), charTuper, table_in.dataSize());
    int length = table_in.dataSize() + 1; //一个元组的信息在文档中的长度
    //insert tuper into index file，更新索引的值
    IndexManager indexMA;
    int blockCapacity = BLOCKSIZE / length;
    for (int i = 0; i < table_in.index.num; i++) {
        int tuperAddr = buf_ptr->blocks[iPos.bufferNUM].blockOffset*blockCapacity + iPos.position / length; //the tuper's addr in the data file
        for (int j = 0; j < table_in.index.num; j++) {
            indexMA.Insert(table_in.getname() + to_string(table_in.index.location[j]) + ".index", singleTuper[table_in.index.location[i]], tuperAddr);
        }
    }
    buf_ptr->WriteBack(iPos.bufferNUM);
    delete[] charTuper;
}

void RecordManager::InsertWithIndex(Table& table_in, tuper& singleTuper)
{
    //check Redundancy using index
    for (int i = 0; i < table_in.attr.num; i++) {
        if (table_in.attr.unique[i] == 1) {
            int addr = FindWithIndex(table_in, singleTuper, i);//根据第i个主键进行操作
            if (addr >= 0) { //already in the table
                throw QueryException("Unique Value Redundancy occurs, thus insertion failed");
                return;
            }
        }
    }
    //楼上和楼下有什么区别？
    for (int i = 0; i < table_in.attr.num; i++) {
        if (table_in.attr.unique[i]) {//先找到一个主键
            vector<where> w;
            vector<int> mask;
            where *uni_w = new where;
            uni_w->flag = eq;
            switch (singleTuper[i]->flag) {
                case -1:uni_w->d = new Datai(((Datai*)singleTuper[i])->x); break;
                case 0:uni_w->d = new Dataf(((Dataf*)singleTuper[i])->x); break;
                default:uni_w->d = new Datac(((Datac*)singleTuper[i])->x); break;
            }
            w.push_back(*uni_w);
            mask.push_back(i);//类似蒙版的作用，选择某一列，然后对看这一列
            Table temp_table = Select(table_in, mask, mask, w);


            if (temp_table.T.size() != 0) {
                throw QueryException("Unique Value Redundancy occurs, thus insertion failed");
            }

            delete uni_w->d;
            delete uni_w;
        }
    }
    //如果没有重复的话，就直接加入元素
    table_in.addData(&singleTuper);
    char *charTuper;
    charTuper = Tuper2Char(table_in, singleTuper);//把一个元组转换成字符串
    //判断是否unique
    InsPos iPos = buf_ptr->getInsertPosition(table_in);//获取插入位置

    buf_ptr->blocks[iPos.blockAddr].getDataChar[iPos.position] = NOTEMPTY;
    memcpy(&(buf_ptr->blocks[iPos.blockAddr].getDataStr[iPos.position + 1]), charTuper, table_in.dataSize());
    int length = table_in.dataSize() + 1; //一个元组的信息在文档中的长度
    //insert tuper into index file
    IndexManager indexMA;
    int blockCapacity = BLOCKSIZE / length;
    for (int i = 0; i < table_in.index.num; i++) {
        int tuperAddr = buf_ptr->blocks[iPos.blockAddr].blockOffset*blockCapacity + iPos.position / length; //the tuper's addr in the data file
        for (int j = 0; j < table_in.index.num; j++) {
            indexMA.Insert(table_in.getname() + to_string(table_in.index.location[j]) + ".index", singleTuper[table_in.index.location[i]], tuperAddr);
        }
    }
    buf_ptr->WriteBack(iPos.blockAddr);
    delete[] charTuper;
}

char* RecordManager::Tuper2Char(Table& table_in, tuper& singleTuper)
{
    char* ptrRes;
    int pos = 0;//当前的插入位置
    ptrRes = new char[(table_in.dataSize() + 1)*sizeof(char)];
    for (int i = 0; i < table_in.getattribute().num;i++){
        if (table_in.getattribute().flag[i] == -1){ //int
            int value = ((Datai*)singleTuper[i])->x;
            memcpy(ptrRes + pos, &value, sizeof(int));
            pos += sizeof(int);
        }
        else if (table_in.getattribute().flag[i] == 0){
            float value = ((Dataf*)singleTuper[i])->x;
            memcpy(ptrRes + pos, &value, sizeof(float));
            pos += sizeof(float);
        }
        else{ //string
            string value(((Datac*)singleTuper[i])->x);
            int strLen = table_in.getattribute().flag[i] + 1;
            memcpy(ptrRes + pos, value.c_str(), strLen);//多加1，拷贝最后的'\0';
            pos += strLen;
        }
    }
    ptrRes[table_in.dataSize()] = '\0';
    return ptrRes;
}


int RecordManager::Delete(Table& table_in, vector<int>mask, vector<where> w)
{
    //算法：delete的操作相当于在找到合适的值，然后在buffer中相应的位置把对应位置的值设为无效就OK
    //根据名字可以读出buffer里面的数据
    string file_name = table_in.getname() + ".table";
    string row_str;

    int count = 0;
    int length = table_in.dataSize() + 1;
    const int record_num = BLOCKSIZE / length;
    for (int blockOffset = 0; blockOffset < table_in.blockNum; blockOffset++){
        int bufferNum = buf_ptr->checkExist(file_name, blockOffset);
        if (bufferNum == -1){
            bufferNum = buf_ptr->getUnoccupiedBlock();
            buf_ptr->readBlock(file_name, blockOffset, bufferNum);
        }
        for (int offset = 0; offset < record_num; offset++){
            int position = offset * length;
            row_str = buf_ptr->blocks[bufferNum].getDataStr(position, position + length);
            if (row_str.c_str()[0] == EMPTY) continue;//该行是空的
            int c_pos = 1;//当前在数据流中指针的位置，0表示该位是否有效，因此数据从第一位开始
            tuper *temp_tuper = new tuper;
            for (int attr_index = 0; attr_index < table_in.getattribute().num; attr_index++){
                if (table_in.getattribute().flag[attr_index] == -1){//是一个整数
                    int value;
                    memcpy(&value, &(row_str.c_str()[c_pos]), sizeof(int));
                    c_pos += sizeof(int);
                    temp_tuper->addData(new Datai(value));
                }
                else if (table_in.getattribute().flag[attr_index] == 0){//float
                    float value;
                    memcpy(&value, &(row_str.c_str()[c_pos]), sizeof(float));
                    c_pos += sizeof(float);
                    temp_tuper->addData(new Dataf(value));
                }
                else{
                    char value[MAXSTRINGLEN];
                    int strLen = table_in.getattribute().flag[attr_index] + 1;
                    memcpy(value, &(row_str.c_str()[c_pos]), strLen);
                    c_pos += strLen;
                    temp_tuper->addData(new Datac(string(value)));
                }
            }//以上内容先从文件中生成一行tuper，以下判断是否满足要求

            if (isSatisfied(table_in,*temp_tuper,mask,w)){
                buf_ptr->blocks[bufferNum].data[position] = DELETED; //DELETED==EMYTP
                buf_ptr->WriteBack(bufferNum);
                count++;
            }
        }
    }
    return count;
}

bool RecordManager::DropTable(Table& table_in)
{
    //类似delete，这里是直接把一张表设为无效
    string file_name = table_in.getname() + ".table";
    if (remove(file_name.c_str()) != 0){
        throw TableException("Can't delete the file!\n");
    }
    else{
        buf_ptr->setInvalid(file_name);
    }
    return true;
}

bool RecordManager::CreateTable(Table& table_in)
{
    //不需要传给Buffer？这个得要再稍微确认一下
    string file_name = table_in.getname() + ".table";
    fstream fout(file_name.c_str(), ios::out);
    fout.close();
    table_in.blockNum = 1;
    //CataManager Ca;
    //Ca.changeblock(table_in.getname(), 1);
    return true;
}


tuper RecordManager::String2Tuper(Table& table_in, string row_str)
{
    tuper temp_tuper;
    if (row_str.c_str()[0] == EMPTY) return temp_tuper;//该行是空的
    int c_pos = 1;//当前在数据流中指针的位置，0表示该位是否有效，因此数据从第一位开始
    for (int attr_index = 0; attr_index < table_in.getattribute().num; attr_index++){
        if (table_in.getattribute().flag[attr_index] == -1){//是一个整数
            int value;
            memcpy(&value, &(row_str.c_str()[c_pos]), sizeof(int));
            c_pos += sizeof(int);
            temp_tuper.addData(new Datai(value));
        }
        else if (table_in.getattribute().flag[attr_index] == 0){//float
            float value;
            memcpy(&value, &(row_str.c_str()[c_pos]), sizeof(float));
            c_pos += sizeof(float);
            temp_tuper.addData(new Dataf(value));
        }
        else{
            char value[MAXSTRINGLEN];
            int strLen = table_in.getattribute().flag[attr_index] + 1;
            memcpy(value, &(row_str.c_str()[c_pos]), strLen);
            c_pos += strLen;
            temp_tuper.addData(new Datac(string(value)));
        }
    }//以上内容先从文件中生成一行tuper
    return temp_tuper;
}

bool RecordManager::UNIQUE(Table& table_in, where w, int loca){
    int length = table_in.dataSize() + 1; //一个元组的信息在文档中的长度
    const int record_num = BLOCKSIZE / length; //一个block中存储的记录条数
    string row_str;
    string file_name = table_in.getname() + ".table";
    int attroff = 1;
    for(int i=0;i<loca-1;i++){
        if(table_in.attr.flag[i]==-1){
            attroff += sizeof(int);
        }
        else if(table_in.attr.flag[i]==0){
            attroff += sizeof(float);
        }
        else{
            attroff += sizeof(char)*table_in.attr.flag[i];
        }
    }
    int inflag = table_in.attr.flag[loca];
    for (int blockOffset = 0; blockOffset < table_in.blockNum;blockOffset++){//读取整个文件中的所有内容
        int bufferNum = buf_ptr->checkExist(file_name, blockOffset);
        if (bufferNum == -1){ //该块不再内存中，读取之
            bufferNum = buf_ptr->getUnoccupiedBlock();
            buf_ptr->readBlock(file_name, blockOffset, bufferNum);
        }
        for (int offset = 0; offset < record_num;offset++){
            int position = offset * length + attroff;
            if(inflag==-1){
                int value;
                memcpy(&value, &(buf_ptr.bufferBlock[bufferNum].values[position+4]), sizeof(int));
                if(value==((Datai*)(w.d))->x)
                    return false;
            }
            else if(inflag==0){
                float value;
                memcpy(&value, &(bf.bufferBlock[bufferNum].values[position+4]), sizeof(float));
                if(value==((Dataf*)(w.d))->x)
                    return false;
            }
            else{
                char value[100];
                memcpy(value, &(bf.bufferBlock[bufferNum].values[position+4]), table_in.attr.flag[loca]+1);
                if(string(value)==((Datac*)(w.d))->x)
                    return false;
            }

        }
    }
    return true;
}

