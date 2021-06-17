#include "CThread.hpp"

#include <QDebug>
#include <QDir>
#include <QThread>

CThread::CThread() : QThread() {}
CThread::~CThread() {}

void CThread::run() {
//    qDebug() << "Running.. \n";
    switch(type){
        case 1:  copyWithThread();  break;
        case 2:  moveWithThread();  break;
        case 3:  deleteWithThread();break;
    }
}

void CThread::copyWithThread() {
    qDebug() << "Copy with threads not implemented\n";
}

void CThread::moveWithThread() {
    qDebug() << "Move with threads not implemented\n";
}

void CThread::deleteWithThread(){
    qDebug() << "Delete with threads not implemented\n";
}

std::string CThread::getTime() {
    qDebug() << "Show the time ";
    auto nowTime = std::chrono::system_clock::now();
    std::time_t sleepTime = std::chrono::system_clock::to_time_t(nowTime);
    return std::ctime(&sleepTime);
}

