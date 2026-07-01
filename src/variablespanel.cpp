#include "variablespanel.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDebug>

VariablesPanel::VariablesPanel(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

void VariablesPanel::setupUI()
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    
    // Variables tree
    variablesTree = new QTreeWidget();
    variablesTree->setHeaderLabel(tr("Variables"));
    variablesTree->setColumnCount(2);
    variablesTree->setHeaderLabels({tr("Variable"), tr("Value")});
    variablesTree->setAlternatingRowColors(true);
    variablesTree->setAnimated(true);
    
    // Create root items
    localVarsItem = new QTreeWidgetItem();
    localVarsItem->setText(0, tr("Local Variables"));
    variablesTree->addTopLevelItem(localVarsItem);
    
    globalVarsItem = new QTreeWidgetItem();
    globalVarsItem->setText(0, tr("Global Variables"));
    variablesTree->addTopLevelItem(globalVarsItem);
    
    watchesItem = new QTreeWidgetItem();
    watchesItem->setText(0, tr("Watches"));
    variablesTree->addTopLevelItem(watchesItem);
    
    layout->addWidget(variablesTree);
    
    // Watch expression input
    QHBoxLayout *watchLayout = new QHBoxLayout();
    watchInputEdit = new QLineEdit();
    watchInputEdit->setPlaceholderText(tr("Enter expression to watch..."));
    addWatchBtn = new QPushButton(tr("Add"));
    removeWatchBtn = new QPushButton(tr("Remove"));
    removeWatchBtn->setEnabled(false);
    
    watchLayout->addWidget(watchInputEdit);
    watchLayout->addWidget(addWatchBtn);
    watchLayout->addWidget(removeWatchBtn);
    layout->addLayout(watchLayout);
    
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
        "} "
        "QLineEdit { "
        "background-color: #3c3c3c; "
        "color: #d4d4d4; "
        "border: 1px solid #555555; "
        "padding: 5px; "
        "}"
    );
    
    // Connect signals
    connect(addWatchBtn, &QPushButton::clicked, this, &VariablesPanel::onAddWatchClicked);
    connect(removeWatchBtn, &QPushButton::clicked, this, &VariablesPanel::onRemoveWatchClicked);
}

void VariablesPanel::setLocalVariables(const QVector<Variable> &vars)
{
    localVariables = vars;
    localVarsItem->takeChildren();
    
    for (const Variable &var : vars) {
        addVariableToTree(localVarsItem, var);
    }
    
    localVarsItem->setExpanded(true);
}

void VariablesPanel::setGlobalVariables(const QVector<Variable> &vars)
{
    globalVariables = vars;
    globalVarsItem->takeChildren();
    
    for (const Variable &var : vars) {
        addVariableToTree(globalVarsItem, var);
    }
}

void VariablesPanel::addWatchExpression(const QString &expr)
{
    if (expr.isEmpty() || watchExpressions.contains(expr)) return;
    
    watchExpressions.append(expr);
    
    QTreeWidgetItem *item = new QTreeWidgetItem();
    item->setText(0, expr);
    item->setText(1, tr("<pending>"));
    watchesItem->addChild(item);
    
    watchesItem->setExpanded(true);
    emit watchExpressionAdded(expr);
}

void VariablesPanel::removeWatchExpression(const QString &expr)
{
    watchExpressions.removeAll(expr);
    
    for (int i = 0; i < watchesItem->childCount(); ++i) {
        if (watchesItem->child(i)->text(0) == expr) {
            delete watchesItem->takeChild(i);
            break;
        }
    }
    
    emit watchExpressionRemoved(expr);
}

void VariablesPanel::updateWatchValues(const QMap<QString, QString> &values)
{
    for (int i = 0; i < watchesItem->childCount(); ++i) {
        QTreeWidgetItem *item = watchesItem->child(i);
        QString expr = item->text(0);
        if (values.contains(expr)) {
            item->setText(1, values[expr]);
        }
    }
}

void VariablesPanel::onAddWatchClicked()
{
    QString expr = watchInputEdit->text().trimmed();
    if (!expr.isEmpty()) {
        addWatchExpression(expr);
        watchInputEdit->clear();
    }
}

void VariablesPanel::onRemoveWatchClicked()
{
    QTreeWidgetItem *item = variablesTree->currentItem();
    if (item && item->parent() == watchesItem) {
        removeWatchExpression(item->text(0));
    }
}

void VariablesPanel::addVariableToTree(QTreeWidgetItem *parent, const Variable &var)
{
    QTreeWidgetItem *item = new QTreeWidgetItem();
    item->setText(0, QString("%1 (%2)").arg(var.name, var.type));
    item->setText(1, var.value);
    parent->addChild(item);
    
    // Add children for expandable variables
    if (var.isExpandable) {
        for (const Variable &member : var.members) {
            addVariableToTree(item, member);
        }
    }
}
