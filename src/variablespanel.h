#ifndef VARIABLESPANEL_H
#define VARIABLESPANEL_H

#include <QWidget>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QPushButton>
#include <QLineEdit>
#include <QVector>
#include "debugger_structs.h"

class VariablesPanel : public QWidget
{
    Q_OBJECT

public:
    explicit VariablesPanel(QWidget *parent = nullptr);
    
    // Update variables
    void setLocalVariables(const QVector<Variable> &vars);
    void setGlobalVariables(const QVector<Variable> &vars);
    void addWatchExpression(const QString &expr);
    void removeWatchExpression(const QString &expr);
    void updateWatchValues(const QMap<QString, QString> &values);
    
    QVector<Variable> getLocalVariables() const { return localVariables; }
    QVector<Variable> getGlobalVariables() const { return globalVariables; }
    QStringList getWatchExpressions() const { return watchExpressions; }
    
signals:
    void watchExpressionAdded(const QString &expr);
    void watchExpressionRemoved(const QString &expr);
    void variableModified(const QString &name, const QString &value);

private slots:
    void onAddWatchClicked();
    void onRemoveWatchClicked();
    void onWatchExpressionEdited(QTreeWidgetItem *item, int column);

private:
    void setupUI();
    void refreshVariables();
    void addVariableToTree(QTreeWidgetItem *parent, const Variable &var);
    
    QTreeWidget *variablesTree;
    QLineEdit *watchInputEdit;
    QPushButton *addWatchBtn;
    QPushButton *removeWatchBtn;
    
    QTreeWidgetItem *localVarsItem;
    QTreeWidgetItem *globalVarsItem;
    QTreeWidgetItem *watchesItem;
    
    QVector<Variable> localVariables;
    QVector<Variable> globalVariables;
    QStringList watchExpressions;
};

#endif // VARIABLESPANEL_H
