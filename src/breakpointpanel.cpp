#include "breakpointpanel.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QDebug>

BreakpointPanel::BreakpointPanel(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

void BreakpointPanel::setupUI()
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    
    // Breakpoint table
    breakpointTable = new QTableWidget();
    breakpointTable->setColumnCount(5);
    breakpointTable->setHorizontalHeaderLabels({tr("ID"), tr("File"), tr("Line"), tr("Condition"), tr("Hits")});
    breakpointTable->horizontalHeader()->setStretchLastSection(false);
    breakpointTable->setColumnWidth(0, 30);
    breakpointTable->setColumnWidth(1, 150);
    breakpointTable->setColumnWidth(2, 50);
    breakpointTable->setColumnWidth(3, 150);
    breakpointTable->setColumnWidth(4, 50);
    breakpointTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    breakpointTable->setSelectionMode(QAbstractItemView::SingleSelection);
    breakpointTable->setAlternatingRowColors(true);
    
    layout->addWidget(breakpointTable);
    
    // Buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    deleteBtn = new QPushButton(tr("Delete"));
    clearAllBtn = new QPushButton(tr("Clear All"));
    buttonLayout->addStretch();
    buttonLayout->addWidget(deleteBtn);
    buttonLayout->addWidget(clearAllBtn);
    layout->addLayout(buttonLayout);
    
    // Styling
    setStyleSheet(
        "QTableWidget { "
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
    connect(breakpointTable, &QTableWidget::itemDoubleClicked, this, [this](QTableWidgetItem *item) {
        int row = breakpointTable->row(item);
        if (row >= 0 && row < breakpoints.count()) {
            const Breakpoint &bp = breakpoints[row];
            emit breakpointDoubleClicked(bp.file, bp.line);
        }
    });
    
    connect(deleteBtn, &QPushButton::clicked, this, &BreakpointPanel::onDeleteClicked);
    connect(clearAllBtn, &QPushButton::clicked, this, &BreakpointPanel::onClearAllClicked);
    connect(breakpointTable, &QTableWidget::itemSelectionChanged, this, &BreakpointPanel::onItemSelectionChanged);
}

void BreakpointPanel::addBreakpoint(const Breakpoint &bp)
{
    if (!bp.isValid()) return;
    
    // Check for duplicates
    for (const Breakpoint &existing : breakpoints) {
        if (existing.file == bp.file && existing.line == bp.line) {
            return; // Already exists
        }
    }
    
    breakpoints.append(bp);
    refreshTable();
}

void BreakpointPanel::removeBreakpoint(int id)
{
    for (int i = 0; i < breakpoints.count(); ++i) {
        if (breakpoints[i].id == id) {
            breakpoints.removeAt(i);
            refreshTable();
            break;
        }
    }
}

void BreakpointPanel::updateBreakpoint(const Breakpoint &bp)
{
    for (int i = 0; i < breakpoints.count(); ++i) {
        if (breakpoints[i].id == bp.id) {
            breakpoints[i] = bp;
            updateRowFromBreakpoint(i, bp);
            break;
        }
    }
}

void BreakpointPanel::clearAll()
{
    breakpoints.clear();
    breakpointTable->setRowCount(0);
}

Breakpoint BreakpointPanel::getBreakpoint(int row) const
{
    if (row >= 0 && row < breakpoints.count()) {
        return breakpoints[row];
    }
    return Breakpoint();
}

void BreakpointPanel::refreshTable()
{
    breakpointTable->setRowCount(breakpoints.count());
    
    for (int i = 0; i < breakpoints.count(); ++i) {
        updateRowFromBreakpoint(i, breakpoints[i]);
    }
}

void BreakpointPanel::updateRowFromBreakpoint(int row, const Breakpoint &bp)
{
    QTableWidgetItem *idItem = new QTableWidgetItem(QString::number(bp.id));
    idItem->setFlags(idItem->flags() & ~Qt::ItemIsEditable);
    breakpointTable->setItem(row, 0, idItem);
    
    QTableWidgetItem *fileItem = new QTableWidgetItem(bp.file);
    fileItem->setFlags(fileItem->flags() & ~Qt::ItemIsEditable);
    breakpointTable->setItem(row, 1, fileItem);
    
    QTableWidgetItem *lineItem = new QTableWidgetItem(QString::number(bp.line));
    lineItem->setFlags(lineItem->flags() & ~Qt::ItemIsEditable);
    breakpointTable->setItem(row, 2, lineItem);
    
    QTableWidgetItem *condItem = new QTableWidgetItem(bp.condition);
    breakpointTable->setItem(row, 3, condItem);
    
    QTableWidgetItem *hitsItem = new QTableWidgetItem(QString::number(bp.hitCount));
    hitsItem->setFlags(hitsItem->flags() & ~Qt::ItemIsEditable);
    breakpointTable->setItem(row, 4, hitsItem);
}

void BreakpointPanel::onDeleteClicked()
{
    int row = breakpointTable->currentRow();
    if (row >= 0 && row < breakpoints.count()) {
        int id = breakpoints[row].id;
        removeBreakpoint(id);
        emit breakpointDeleted(id);
    }
}

void BreakpointPanel::onClearAllClicked()
{
    clearAll();
}

void BreakpointPanel::onItemSelectionChanged()
{
    int row = breakpointTable->currentRow();
    if (row >= 0) {
        deleteBtn->setEnabled(true);
    } else {
        deleteBtn->setEnabled(false);
    }
}
