#ifndef CTHREAD_H
#define CTHREAD_H

#include <QThread>

class CThread : public QThread {
    Q_OBJECT
public:
    explicit CThread();
    void run() override;
    ~CThread();

public slots:
    void copyWithThread();
    void moveWithThread();
    void deleteWithThread();
    std::string getTime();

private:
    int type = 1; // 1 copy with thread
                  // 2 move with thread
                  // 3 delete with thread
    QTimer*     timer; 
};

#endif // CTHREAD_H
