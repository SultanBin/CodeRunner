#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSplitter>
#include <QPlainTextEdit>
#include <QTreeWidget>
#include <QLabel>
#include <QProgressBar>
#include <memory>

class CodeEditor;
class FileManager;
class Compiler;
class Debugger;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private slots:
    // File menu
    void newProject();
    void newFile();
    void openProject();
    void openFile();
    void saveFile();
    void saveFileAs();
    void saveAll();
    void closeFile();
    void exit();
    
    // Edit menu
    void undo();
    void redo();
    void cut();
    void copy();
    void paste();
    void selectAll();
    void find();
    void replace();
    
    // Build menu
    void compile();
    void compileAndRun();
    void stop();
    
    // Debug menu
    void startDebug();
    void stepOver();
    void stepInto();
    void stepOut();
    void toggleBreakpoint();
    void continueExecution();
    
    // Tools menu
    void preferences();
    void manageCompilers();
    
    // Help menu
    void about();
    void aboutQt();
    void documentation();
    
    // Signals from components
    void onCompileFinished(bool success, const QString &output);
    void onFileSelected(const QString &filepath);
    void onFileModified(bool modified);

private:
    void createUI();
    void createMenuBar();
    void createToolBars();
    void createStatusBar();
    void connectSignals();
    void loadSettings();
    void saveSettings();
    void setupTheme();
    void setupConnections();
    
    // UI Components
    QWidget *centralWidget;
    QSplitter *mainSplitter;      // Horizontal: left panel + right panel
    QSplitter *rightSplitter;     // Vertical: editor + console
    
    std::unique_ptr<FileManager> fileManager;
    std::unique_ptr<CodeEditor> codeEditor;
    std::unique_ptr<Compiler> compiler;
    std::unique_ptr<Debugger> debugger;
    
    // Output console
    QPlainTextEdit *outputConsole;
    QPlainTextEdit *debugConsole;
    
    // Status bar
    QLabel *statusLabel;
    QLabel *cursorPosLabel;
    QLabel *compilationStatusLabel;
    QProgressBar *compilationProgress;
    
    // State
    QString currentProjectPath;
    QString currentFilePath;
    bool isModified;
    bool isCompiling;
    bool isDebugging;
};

#endif // MAINWINDOW_H
