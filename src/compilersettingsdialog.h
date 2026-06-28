#ifndef COMPILERSETTINGSDIALOG_H
#define COMPILERSETTINGSDIALOG_H

#include "projectconfig.h"
#include <QDialog>
#include <QComboBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>

class CompilerSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CompilerSettingsDialog(ProjectConfig &config, QWidget *parent = nullptr);
    
    ProjectConfig getConfig() const { return config; }
    
private slots:
    void onAccept();
    void onAddIncludePath();
    void onRemoveIncludePath();
    void onAddLibraryPath();
    void onRemoveLibraryPath();
    void onBrowseIncludePath();
    void onBrowseLibraryPath();

private:
    void createUI();
    void setupConnections();
    void loadConfig();
    void updateCompilerList();
    void updateStandardList();
    
    ProjectConfig &config;
    
    // UI Components
    QComboBox *compilerCombo;
    QComboBox *standardCombo;
    QComboBox *optimizationCombo;
    QCheckBox *debugCheckBox;
    QCheckBox *warningsCheckBox;
    QLineEdit *customFlagsEdit;
    
    QListWidget *includePathsList;
    QListWidget *libraryPathsList;
    QPushButton *addIncludeBtn;
    QPushButton *removeIncludeBtn;
    QPushButton *addLibraryBtn;
    QPushButton *removeLibraryBtn;
    
    QLineEdit *sourceDirEdit;
    QLineEdit *buildDirEdit;
    QLineEdit *binDirEdit;
};

#endif // COMPILERSETTINGSDIALOG_H
