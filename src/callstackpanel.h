#ifndef CALLSTACKPANEL_H
#define CALLSTACKPANEL_H

#include <QWidget>
#include <QTreeWidget>
#include <QVector>
#include "debugger_structs.h"

class CallStackPanel : public QWidget
{
    Q_OBJECT

public:
    explicit CallStackPanel(QWidget *parent = nullptr);
    
    // Stack management
    void setCallStack(const QVector<StackFrame> &frames);
    void clear();
    
    QVector<StackFrame> getCallStack() const { return callStack; }
    int getSelectedFrameLevel() const { return selectedFrameLevel; }
    
signals:
    void frameSelected(int level);
    void frameDoubleClicked(const QString &file, int line);

private slots:
    void onItemClicked(QTreeWidgetItem *item, int column);
    void onItemDoubleClicked(QTreeWidgetItem *item, int column);

private:
    void setupUI();
    void refreshStack();
    
    QTreeWidget *stackTree;
    QVector<StackFrame> callStack;
    int selectedFrameLevel;
};

#endif // CALLSTACKPANEL_H
