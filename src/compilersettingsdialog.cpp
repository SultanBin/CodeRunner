#include "compilersettingsdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QFileDialog>
#include <QDebug>
#include <QDialogButtonBox>

CompilerSettingsDialog::CompilerSettingsDialog(ProjectConfig &config, QWidget *parent)
    : QDialog(parent), config(config)
{
    setWindowTitle(tr("Compiler Settings"));
    setMinimumWidth(600);
    setMinimumHeight(500);
    
    createUI();
    setupConnections();
    loadConfig();
    updateCompilerList();
    updateStandardList();
}

void CompilerSettingsDialog::createUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // Compiler Settings Group
    QGroupBox *compilerGroup = new QGroupBox(tr("Compiler Settings"));
    QVBoxLayout *compilerLayout = new QVBoxLayout();
    
    // Compiler selection
    QHBoxLayout *compilerRow = new QHBoxLayout();
    compilerRow->addWidget(new QLabel(tr("Compiler:")));
    compilerCombo = new QComboBox();
    compilerCombo->addItems({"GCC", "Clang", "MinGW", "MSVC"});
    compilerRow->addWidget(compilerCombo);
    compilerLayout->addLayout(compilerRow);
    
    // Standard selection
    QHBoxLayout *standardRow = new QHBoxLayout();
    standardRow->addWidget(new QLabel(tr("C++ Standard:")));
    standardCombo = new QComboBox();
    standardCombo->addItems({"C++11", "C++14", "C++17", "C++20"});
    standardRow->addWidget(standardCombo);
    compilerLayout->addLayout(standardRow);
    
    // Optimization level
    QHBoxLayout *optimRow = new QHBoxLayout();
    optimRow->addWidget(new QLabel(tr("Optimization:")));
    optimizationCombo = new QComboBox();
    optimizationCombo->addItems({"-O0 (None)", "-O1 (Basic)", "-O2 (Moderate)", "-O3 (Aggressive)", "-Os (Size)"});
    optimRow->addWidget(optimizationCombo);
    compilerLayout->addLayout(optimRow);
    
    // Debug and Warnings
    debugCheckBox = new QCheckBox(tr("Include Debug Information (-g)"));
    warningsCheckBox = new QCheckBox(tr("Enable All Warnings (-Wall -Wextra)"));
    compilerLayout->addWidget(debugCheckBox);
    compilerLayout->addWidget(warningsCheckBox);
    
    // Custom flags
    QHBoxLayout *flagsRow = new QHBoxLayout();
    flagsRow->addWidget(new QLabel(tr("Custom Flags:")));
    customFlagsEdit = new QLineEdit();
    flagsRow->addWidget(customFlagsEdit);
    compilerLayout->addLayout(flagsRow);
    
    compilerGroup->setLayout(compilerLayout);
    mainLayout->addWidget(compilerGroup);
    
    // Paths Group
    QGroupBox *pathsGroup = new QGroupBox(tr("Include & Library Paths"));
    QVBoxLayout *pathsLayout = new QVBoxLayout();
    
    // Include paths
    QLabel *includeLabel = new QLabel(tr("Include Paths:"));
    pathsLayout->addWidget(includeLabel);
    includePathsList = new QListWidget();
    pathsLayout->addWidget(includePathsList);
    
    QHBoxLayout *includeButtonsLayout = new QHBoxLayout();
    addIncludeBtn = new QPushButton(tr("Add"));
    removeIncludeBtn = new QPushButton(tr("Remove"));
    includeButtonsLayout->addWidget(addIncludeBtn);
    includeButtonsLayout->addWidget(removeIncludeBtn);
    includeButtonsLayout->addStretch();
    pathsLayout->addLayout(includeButtonsLayout);
    
    // Library paths
    QLabel *libLabel = new QLabel(tr("Library Paths:"));
    pathsLayout->addWidget(libLabel);
    libraryPathsList = new QListWidget();
    pathsLayout->addWidget(libraryPathsList);
    
    QHBoxLayout *libButtonsLayout = new QHBoxLayout();
    addLibraryBtn = new QPushButton(tr("Add"));
    removeLibraryBtn = new QPushButton(tr("Remove"));
    libButtonsLayout->addWidget(addLibraryBtn);
    libButtonsLayout->addWidget(removeLibraryBtn);
    libButtonsLayout->addStretch();
    pathsLayout->addLayout(libButtonsLayout);
    
    pathsGroup->setLayout(pathsLayout);
    mainLayout->addWidget(pathsGroup);
    
    // Directories Group
    QGroupBox *dirsGroup = new QGroupBox(tr("Project Directories"));
    QVBoxLayout *dirsLayout = new QVBoxLayout();
    
    QHBoxLayout *sourceDirRow = new QHBoxLayout();
    sourceDirRow->addWidget(new QLabel(tr("Source Directory:")));
    sourceDirEdit = new QLineEdit();
    sourceDirRow->addWidget(sourceDirEdit);
    dirsLayout->addLayout(sourceDirRow);
    
    QHBoxLayout *buildDirRow = new QHBoxLayout();
    buildDirRow->addWidget(new QLabel(tr("Build Directory:")));
    buildDirEdit = new QLineEdit();
    buildDirRow->addWidget(buildDirEdit);
    dirsLayout->addLayout(buildDirRow);
    
    QHBoxLayout *binDirRow = new QHBoxLayout();
    binDirRow->addWidget(new QLabel(tr("Binary Directory:")));
    binDirEdit = new QLineEdit();
    binDirRow->addWidget(binDirEdit);
    dirsLayout->addLayout(binDirRow);
    
    dirsGroup->setLayout(dirsLayout);
    mainLayout->addWidget(dirsGroup);
    
    // Buttons
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    mainLayout->addWidget(buttonBox);
    
    connect(buttonBox, &QDialogButtonBox::accepted, this, &CompilerSettingsDialog::onAccept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

void CompilerSettingsDialog::setupConnections()
{
    connect(addIncludeBtn, &QPushButton::clicked, this, &CompilerSettingsDialog::onAddIncludePath);
    connect(removeIncludeBtn, &QPushButton::clicked, this, &CompilerSettingsDialog::onRemoveIncludePath);
    connect(addLibraryBtn, &QPushButton::clicked, this, &CompilerSettingsDialog::onAddLibraryPath);
    connect(removeLibraryBtn, &QPushButton::clicked, this, &CompilerSettingsDialog::onRemoveLibraryPath);
}

void CompilerSettingsDialog::loadConfig()
{
    // Set compiler
    int compilerIndex = compilerCombo->findText(config.getCompiler(), Qt::MatchStartsWith);
    if (compilerIndex >= 0) compilerCombo->setCurrentIndex(compilerIndex);
    
    // Set standard
    int standardIndex = standardCombo->findText(config.getStandard().mid(1), Qt::MatchContains);
    if (standardIndex >= 0) standardCombo->setCurrentIndex(standardIndex);
    
    // Set optimization
    int optimIndex = optimizationCombo->findText(config.getOptimization(), Qt::MatchStartsWith);
    if (optimIndex >= 0) optimizationCombo->setCurrentIndex(optimIndex);
    
    debugCheckBox->setChecked(config.getDebugInfo());
    warningsCheckBox->setChecked(config.getWarnings());
    customFlagsEdit->setText(config.getCustomFlags());
    
    // Load paths
    for (const QString &path : config.getIncludePaths()) {
        includePathsList->addItem(path);
    }
    
    for (const QString &path : config.getLibraryPaths()) {
        libraryPathsList->addItem(path);
    }
    
    sourceDirEdit->setText(config.getSourceDir());
    buildDirEdit->setText(config.getBuildDir());
    binDirEdit->setText(config.getBinDir());
}

void CompilerSettingsDialog::updateCompilerList()
{
    // This would query the system for available compilers
    // For now, we just show all options
}

void CompilerSettingsDialog::updateStandardList()
{
    // This would vary based on selected compiler
}

void CompilerSettingsDialog::onAccept()
{
    // Update compiler
    QString compiler = compilerCombo->currentText().toLower();
    config.setCompiler(compiler);
    
    // Update standard
    QString standard = "c++" + standardCombo->currentText().mid(4);
    config.setStandard(standard);
    
    // Update optimization
    QString optim = optimizationCombo->currentText();
    optim = optim.left(optim.indexOf(' '));
    config.setOptimization(optim);
    
    config.setDebugInfo(debugCheckBox->isChecked());
    config.setWarnings(warningsCheckBox->isChecked());
    config.setCustomFlags(customFlagsEdit->text());
    
    config.setSourceDir(sourceDirEdit->text());
    config.setBuildDir(buildDirEdit->text());
    config.setBinDir(binDirEdit->text());
    
    accept();
}

void CompilerSettingsDialog::onAddIncludePath()
{
    QString path = QFileDialog::getExistingDirectory(this, tr("Select Include Directory"));
    if (!path.isEmpty()) {
        includePathsList->addItem(path);
        config.addIncludePath(path);
    }
}

void CompilerSettingsDialog::onRemoveIncludePath()
{
    QListWidgetItem *item = includePathsList->currentItem();
    if (item) {
        config.removeIncludePath(item->text());
        delete includePathsList->takeItem(includePathsList->row(item));
    }
}

void CompilerSettingsDialog::onAddLibraryPath()
{
    QString path = QFileDialog::getExistingDirectory(this, tr("Select Library Directory"));
    if (!path.isEmpty()) {
        libraryPathsList->addItem(path);
        config.addLibraryPath(path);
    }
}

void CompilerSettingsDialog::onRemoveLibraryPath()
{
    QListWidgetItem *item = libraryPathsList->currentItem();
    if (item) {
        config.removeLibraryPath(item->text());
        delete libraryPathsList->takeItem(libraryPathsList->row(item));
    }
}

void CompilerSettingsDialog::onBrowseIncludePath()
{
    // Browse for include path
}

void CompilerSettingsDialog::onBrowseLibraryPath()
{
    // Browse for library path
}
