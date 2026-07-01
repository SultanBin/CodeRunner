#include "callstackpanel.h"
#include <QVBoxLayout>
#include <QHeaderView>

CallStackPanel::CallStackPanel(QWidget *parent)
    : QWidget(parent), selectedFrameLevel(-1)
{
    setupUI();
}

void CallStackPanel::setupUI()
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    
    stackTree = new QTreeWidget();
    stackTree->setHeaderLabel(tr("Call Stack"));
    stackTree->setColumnCount(4);
    stackTree->setHeaderLabels({tr("Frame"), tr("Function"), tr("File"), tr("Line")});
    stackTree->setColumnWidth(0, 50);
    stackTree->setColumnWidth(1, 150);
    stackTree->setColumnWidth(2, 150);
    stackTree->setColumnWidth(3, 50);
    stackTree->setAlternatingRowColors(true);
    stackTree->setSelectionBehavior(QAbstractItemView::SelectRows);
    stackTree->setSelectionMode(QAbstractItemView::SingleSelection);
    
    layout->addWidget(stackTree);
    
    // Styling
    setStyleSheet(
        "QTreeWidget { "
        "background-color: #252526; "
        "color: #d4d4d4; "
        "gridline-color: #3c3c3c; "
        "} "
        "QHeaderView::section { "
        "background-color: #3c3c3c; "
        "color: #d4d4d4; "
        "padding: 5px; "
        "border: none; "
        "}"
    );
    
    // Connect signals
    connect(stackTree, &QTreeWidget::itemClicked, this, &CallStackPanel::onItemClicked);
    connect(stackTree, &QTreeWidget::itemDoubleClicked, this, &CallStackPanel::onItemDoubleClicked);
}

void CallStackPanel::setCallStack(const QVector<StackFrame> &frames)
{
    callStack = frames;
    refreshStack();
}

void CallStackPanel::clear()
{
    callStack.clear();
    stackTree->clear();
    selectedFrameLevel = -1;
}

void CallStackPanel::refreshStack()
{
    stackTree->clear();
    
    for (const StackFrame &frame : callStack) {
        QTreeWidgetItem *item = new QTreeWidgetItem();
        item->setText(0, QString::number(frame.level));
        item->setText(1, frame.function);
        item->setText(2, frame.file);
        item->setText(3, QString::number(frame.line));
        item->setData(0, Qt::UserRole, frame.level);
        stackTree->addTopLevelItem(item);
    }
    
    // Select first frame
    if (stackTree->topLevelItemCount() > 0) {
        stackTree->setCurrentItem(stackTree->topLevelItem(0));
        selectedFrameLevel = 0;
    }
}

void CallStackPanel::onItemClicked(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column);
    selectedFrameLevel = item->data(0, Qt::UserRole).toInt();
    emit frameSelected(selectedFrameLevel);
}

void CallStackPanel::onItemDoubleClicked(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column);
    int row = stackTree->indexOfTopLevelItem(item);
    if (row >= 0 && row < callStack.count()) {
        const StackFrame &frame = callStack[row];
        emit frameDoubleClicked(frame.file, frame.line);
    }
}
