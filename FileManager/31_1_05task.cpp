#include <stdio.h>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include "os_file.h"

using namespace std;

typedef struct node{
    bool dir;
    int size;
    string absPath;
    node* parent;
}node;

vector<node*> child;
node* current;
int isCreated = 0;
int freeSpace = 0;

node* addNode(bool dir, int size, string path, node* parent)
{
    child.push_back(new node);
    child.back() -> dir = dir;
    child.back() -> size = size;
    child.back() -> absPath = path;
    child.back() -> parent = parent;
    return child.back();
}

int my_create(int disk_size)
{
    if(isCreated)
        return 0;
    isCreated = 1;
    freeSpace = disk_size;
    addNode(true, 0, "/", NULL);
    current = child[0];
    child[0] -> parent = NULL;
    return 1;
}

string pathToAbs(const char* path)
{
    string s = path, s2 = "", str = "";
    int length = s.size();
    if(s == ".")
        return current -> absPath;
    if(s == "..") 
    {
        if(current -> absPath != "/")
            return current -> parent -> absPath;
        else
            return "&&&";//нельзя возвращаться к родителю корня
    } 

    if(s[0] != '/')//преобразуем путь в абсолютный
    {
        s2 = current -> absPath;
        if(s2 != "/")
          s2 += '/';
        s2 += s;
        length = s2.size();
     }
     else s2 = s;

     int lastSlash;
     vector<string> names;
     for(int i = length - 1; i >= 0; i--)
         if(s2[i] == '/')//находим последний /
         {
            lastSlash = i;
            break;
         }
     if(!lastSlash)// / первый и последний
         return s2;

     int i = 1, p = 0;// i от 1, так как первым всегда /
     while(i <= lastSlash)// идем до последнего / и собираем имена папок или файлов в str и добавляем в вектор имен
     {
        if(s2[i] != '/')
        {
            str += s2[i];
            i++;
            continue;
        }
  
        if(str == "..")
        {
            if(names.empty())//нельзя возвращаться к родителю корня
               return "&&&";
            names.pop_back();//идем к родителю
            i++;
            str = "";
            continue;
        }
        if(str == ".")
        {
            i++;
            str = "";
            continue;
        }
    
        names.push_back(str);
        str = "";
        i++;
      }
     names.push_back(s2.substr(lastSlash + 1));//собирает то, что после последнего /
     
     s = "";
     for(int i = 0; i < (int)names.size(); i++)//собираем весь путь
     {
        s += '/';
        s += names[i];
     }
     return s;
}

node* pathExists(string path)
{
    for(int i = 0; i < (int)child.size(); i++)
        if(child[i] -> absPath == path)
            return child[i];
    return NULL;
}

int pathCorrect(const char* path)
{
    string s = path;
    for(int i = 0; i < (int)s.size(); i++)//запрещается создавать файлы или директории с именами . и ..
        if( (s[i] == '.' && s[i+1] == '\0') || (s[i] == '.' && s[i+1] == '.' && s[i+2] == '\0') )//проверка на точки в конце пути
            return 0;
    for(int i = 0; i < (int)s.size(); i++)
        if( ((s[i] >= 65 && s[i] <= 90) || (s[i] >= 97 && s[i] <= 122) || (s[i] >= 48 && s[i] <= 57) || s[i] == '.' || s[i] == '/' || s[i] == '_') == false)
            return 0;
    return 1;
}

int my_create_dir(const char* path)
{
    if(!isCreated || pathCorrect(path) == 0)
        return 0;
    string absPath = pathToAbs(path);
    if(absPath == "&&&")
        return 0;
    string s = absPath;
    int i = s.size() - 1;
    while(s[i] != '/')//убираем /smth
    {
        s.pop_back();
        i--;
    }
    if(s.size() > 1)
        s.pop_back();//убираем /, если он лишний
    node* temp = pathExists(s);
    if(temp != NULL && pathExists(absPath) == NULL)//если сущ путь без создаваемой папки и НЕ существует с создаваемой
    {
        if(temp -> dir == false)
            return 0;//нельзя создавать папку внутри файла
        addNode(true, 0, absPath, temp);
        return 1;
    }
    return 0;
}

int my_create_file(const char* path, int file_size)
{
    if(!isCreated || freeSpace < file_size)
        return 0;
    if(pathCorrect(path) == 0)
        return 0;
    string absPath = pathToAbs(path);
    if(absPath == "&&&")
        return 0;
    string s = absPath;
    int i = s.size() - 1;
    while(s[i] != '/')//убираем /smth
    {
        s.pop_back();
        i--;
    }
    if(s.size() > 1)
        s.pop_back();//убираем /
    node* temp = pathExists(s);
    if(temp != NULL && pathExists(absPath) == NULL)//если сущ путь без создаваемой папки и НЕ существует путь с создаваемой папкой
    {
        if(temp -> dir == false)
            return 0;//нельзя создавать файл внутри файла
        freeSpace -= file_size;
        addNode(false, file_size, absPath, temp);
        return 1;
    }
    return 0;
}

int my_change_dir(const char* path)
{
    if(!isCreated)
        return 0;
    string absPath = pathToAbs(path);
    if(absPath == "&&&")
        return 0;
    node* temp = pathExists(absPath);
    if(temp != NULL)
    {
        current = temp;
        return 1;
    }
    return 0;
}

void my_get_cur_dir(char *dst)
{
    memcpy(dst, current -> absPath.c_str(), current -> absPath.size());//копирует путь memcpy(куда, что, какой длины)
    dst[current -> absPath.size()] = NULL;//укажем конец строки в dst
}

int my_remove(const char* path, int recursive)
{
    if(!isCreated)
        return 0;
    if(pathCorrect(path) == 0 || path == "/")// корневая директория '/' не может быть удалена
        return 0;
    string s = pathToAbs(path);
    if(s == "&&&")
        return 0;
    int index;
    node* temp = NULL;
    for(int i = 0; i < (int)child.size(); i++)
        if(child[i] -> absPath == s)
        {
            temp = child[i];
            index = i;
            break;
        }
    if(temp == NULL)
        return 0;

    if(temp -> dir == false)//удаляем файл
    {
        freeSpace += temp -> size;
        delete child[index];
        child.erase(child.begin() + index);
        return 1;
    }
    
    string ss = s + '/';
    bool empty = true;
    for(int i = 0; i < (int)child.size(); i++)
        if(child[i] -> absPath.substr(0, ss.size()) == ss)
            empty = false;//папка непуста
    
    if(recursive == 0 && empty == false)
        return 0;
    if(recursive == 0 && empty == true)
    {
        if(current -> absPath == s)//текущая директория была удалена - текущей становится корневая директория
            current = child[0];
        delete child[index];
        child.erase(child.begin() + index);
        return 1;
    }
    for(int i = 0; i < (int)child.size(); i++)
        if(child[i] -> absPath.substr(0, s.size()) == s)
        {
            if(current == child[i])//текущая директория была удалена - текущей становится корневая директория
                current = child[0];
            freeSpace += child[i] -> size;
            delete child[i];
            child.erase(child.begin() + i);
        }
    return 1;
}

vector<node*> Sort(vector<node*> v)//сортирует имена в алфавитном порядке
{
    node* temp;
    for(int i = 0; i < (int)v.size(); i++)
        for(int j = i + 1; j < (int)v.size(); j++)
            if(v[i] -> absPath > v[j] -> absPath)
            {
                temp = v[i];
                v[i] = v[j];
                v[j] = temp;
            }
    return v;
}

string name(string s)//возвращает имя папки или файла по абсолютному пути
{
    int i = s.size() - 1;
    string name = "";
    while(s[i] != '/')
    {
        name = s[i] + name;
        i--;
    }
    return name;
}

int recursion(const char* path, int dir_first, string firstPath)
{
    string s = pathToAbs(path);
    if(s == "&&&")
        return 0;
    node* temp = pathExists(s);
    if(temp == NULL)
        return 0;

    if(temp -> dir == false)
        return 1;
    
    string ss;
    if(s.size() > 1)
        ss = s + '/';
    else ss = s;
    bool empty = true;
    for(int i = 0; i < (int)child.size(); i++)
    {
        if(child[i] -> absPath.substr(0, ss.size()) == ss)
            empty = false;//папка непуста
    }

    if(empty == true)
    {
        if(firstPath != path)
            firstPath += '/' + name(path);
        cout << firstPath << ": " << endl;
        return 1;
    }

    if(!dir_first)
    {
        vector<node*> subdir;
        for(int i = 1; i < (int)child.size(); i++)
            if((child[i] -> parent -> absPath) == s)
            {
                subdir.push_back(child[i]);
                subdir = Sort(subdir);
            }
        if(firstPath != path)
            firstPath += '/' + name(path);
        cout << firstPath << ": " << endl;
        for(int i = 0; i < (int)subdir.size(); i++)
            cout << name(subdir[i] -> absPath) << endl;
        for(int i = 0; i < (int)subdir.size(); i++)
            recursion(subdir[i] -> absPath.c_str(), dir_first, firstPath);
    }
    else
    {
        vector<node*> subdir, subfile;
        for(int i = 1; i < (int)child.size(); i++)
            if((child[i] -> parent -> absPath) == s)
            {
                if(child[i] -> dir == true)
                    subdir.push_back(child[i]);
                else
                    subfile.push_back(child[i]);
                subdir = Sort(subdir);
                subfile = Sort(subfile);
            }
        if(firstPath != path)
            firstPath += '/' + name(path);
        cout << firstPath << ": " << endl;
        for(int i = 0; i < (int)subdir.size(); i++)
            cout << name(subdir[i] -> absPath) << endl;
        for(int i = 0; i < (int)subfile.size(); i++)
            cout << name(subfile[i] -> absPath) << endl;
        for(int i = 0; i < (int)subdir.size(); i++)
            recursion(subdir[i] -> absPath.c_str(), dir_first, firstPath);
    }
    return 1;
}

int my_list_recursive(const char* path, int dir_first)
{
    if(!isCreated)
        return 0;
    if(pathCorrect(path) == 0)
        return 0;
    string firstPath = path;
    int rtn = recursion(path, dir_first, firstPath);
    cout << endl;
    return rtn;
}
    
int my_destroy()
{
    if(!isCreated)
        return 0;
    else
    {
        for(int i = 0; i < (int)child.size(); i++)
            delete child[i];//чистим память, удаляем указатель
        child.clear();//удаляем сам вектор
        freeSpace = 0;
        current = NULL;
        isCreated = 0;
        return 1;
    }
}

void setup_file_manager(file_manager_t *fm)
{
    fm->change_dir = my_change_dir;
    fm->create = my_create;
    fm->destroy = my_destroy;
    fm->create_dir = my_create_dir;
    fm->create_file = my_create_file;
    fm->remove = my_remove;
    fm->list_recursive = my_list_recursive;
    fm->get_cur_dir = my_get_cur_dir;
}

int main()
{
    file_manager_t fm;
    setup_file_manager(&fm);

     fm.create(1000);
     fm.create_dir("dir1");
     fm.create_dir("dir1/dir11");
     fm.change_dir("dir1");
     fm.create_dir("../dir2");
     fm.create_dir("../dir2/dir3");
     fm.remove("/dir2/dir3", 0);
     fm.create_dir("/dir3/dir31");//
     fm.create_dir("../dir3/dir31");//
     fm.create_dir("../dir2");//
     fm.create_file("/dir2/file1", 1);
     fm.create_dir("/dir2/dir21");
     fm.create_dir("/dir2/file1");//
     fm.create_dir("../dir2/file1");//
     fm.create_dir("../dir2/file1/dir");//
     fm.create_dir("../dir2/dir22");
     fm.change_dir("dir2");//
     fm.change_dir("dir11");
     fm.remove("../../dir2/file1", 0);
     fm.create_dir("../../dir2/file1");
     fm.remove("../../dir2/file1", 0);
     fm.create_file("../../dir2/file1", 1);
     fm.change_dir(".");
     fm.change_dir("/dir1/dir11");
     fm.change_dir("/dir1/dir11/dir3");//
     fm.change_dir("/dir1");
     fm.change_dir("./dir11");
     fm.change_dir("..");
     fm.create_file("./dir11/file.txt", 1);
     fm.remove("dir11", 0);//
     fm.remove("./dir11", 0);//
     fm.remove("./dir11", 1);
     fm.create_file("file11.txt",1);
     fm.create_file("../dir2/a_file22.txt",1);
     fm.create_file("../dir2/dir22/file222.txt",1);
     fm.list_recursive("../dir2",0);

    getchar();
    return 0;
}