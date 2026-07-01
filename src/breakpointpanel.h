#ifndef BREAKPOINTPANEL_H
#define BREAKPOINTPANEL_H

#include <QWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QVector>
#include "debugger_structs.h"

class BreakpointPanel : public QWidget
{
    Q_OBJECT

public:
    explicit BreakpointPanel(QWidget *parent = nullptr);
    
    // Breakpoint management
    void addBreakpoint(const Breakpoint &bp);
    void removeBreakpoint(int id);
    void updateBreakpoint(const Breakpoint &bp);
    void clearAll();
    
    QVector<Breakpoint> getBreakpoints() const { return breakpoints; }
    Breakpoint getBreakpoint(int row) const;
    
signals:
    void breakpointToggled(int id, bool enabled);
    void breakpointDeleted(int id);
    void breakpointDoubleClicked(const QString &file, int line);
    void breakpointEdited(const Breakpoint &bp);

private slots:
    void onItemDoubleClicked(int row, int column);
    void onCellChanged(int row, int column);
    void onDeleteClicked();
    void onClearAllClicked();
    void onItemSelectionChanged();

private:
    void setupUI();
    void refreshTable();
    void updateRowFromBreakpoint(int row, const Breakpoint &bp);
    
    QTableWidget *breakpointTable;
    QPushButton *deleteBtn;
    QPushButton *clearAllBtn;
    QVector<Breakpoint> breakpoints;
};

#endif // BREAKPOINTPANEL_H
