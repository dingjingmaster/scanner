//
// Created by dingjing on 2/26/25.
//
#include "../app/pf/utils.h"

#include <QDebug>


int main (int argc, char *argv[])
{
    const char path1[] = "/////////a/////////d//////////f/////////c////////f//////";
    const char path2[] = "////////////a/////////b//////c/////////d////////////e/////f.txt";
    const char path3[] = "";
    const char path4[] = "/a/b/c/d";
    const char path5[] = "/a/b/c/d/";
    const char path6[] = "/a/b/c/d/a.txt";

    qInfo() << path1 << " ==> " << Utils::formatPath(path1);
    qInfo() << path2 << " ==> " << Utils::formatPath(path2);
    qInfo() << path3 << " ==> " << Utils::formatPath(path3);
    qInfo() << path4 << " ==> " << Utils::formatPath(path4);
    qInfo() << path5 << " ==> " << Utils::formatPath(path5);
    qInfo() << path6 << " ==> " << Utils::formatPath(path6);


    return 0;
}