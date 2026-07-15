#include "mainwindow.h"
#include "codeeditor.h"
#include "filemanager.h"
#include "compiler.h"
#include "debugger.h"
#include "compileroutputpanel.h"
#include "compiler_output_feeder.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QAction>
#include <QActionGroup>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <QApplication>
#include <QScreen>
#include <QSplitter>
#include <QLabel>
#include <QProgressBar>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QDebug>
#include <QProcess>
#include <QFileInfo>
#include <QDir>

#include "projectconfig.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), isModified(false), isCompiling(false), isDebugging(false), runAfterCompile(false)
{
    setWindowTitle("CodeRunner - C/C++ IDE");
    setWindowIcon(QIcon(":/icons/app.png"));
    
    // Create components
    fileManager = std::make_unique<FileManager>();
    codeEditor = std::make_unique<CodeEditor>();
    compiler = std::make_unique<Compiler>();
    debugger = std::make_unique<Debugger>();
    runProcess = nullptr;
    compilerPanel = std::make_unique<CompilerOutputPanel>();
    
    // Setup UI
    createUI();
    createMenuBar();
    createToolBars();
    createStatusBar();
    setupTheme();
    setupConnections();
    loadSettings();
    
    // Accept drag and drop
    setAcceptDrops(true);
    
    // Set window size
    resize(1400, 900);
    
    // Center on screen
    QScreen *screen = QApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    int x = (screenGeometry.width() - width()) / 2;
    int y = (screenGeometry.height() - height()) / 2;
    move(x, y);
}

MainWindow::~MainWindow()
{
    saveSettings();
}

void MainWindow::createUI()
{
    centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    
    // Main horizontal splitter: file tree + editor + console
    mainSplitter = new QSplitter(Qt::Horizontal);
    
    // Left panel: File manager (25%)
    mainSplitter->addWidget(fileManager.get());
    
    // Right panel: vertical splitter for editor and console
    rightSplitter = new QSplitter(Qt::Vertical);
    
    // Code editor (70%)
    rightSplitter->addWidget(codeEditor.get());
    
    // Console widget
    QWidget *consoleWidget = new QWidget();
    QVBoxLayout *consoleLayout = new QVBoxLayout(consoleWidget);
    consoleLayout->setContentsMargins(0, 0, 0, 0);
    
    // Output console -> CompilerOutputPanel
    compilerPanel->setPlaceholderText("Build output will appear here...");
    compilerPanel->setMaximumHeight(200);
    consoleLayout->addWidget(compilerPanel.get());
    rightSplitter->addWidget(consoleWidget);
    
    // Set splitter sizes (70% editor, 30% console)
    rightSplitter->setSizes({700, 300});
    
    // Add right panel to main splitter
    mainSplitter->addWidget(rightSplitter);
    
    // Set splitter sizes (25% files, 75% editor+console)
    mainSplitter->setSizes({350, 1050});
    mainSplitter->setCollapsible(0, false);
    mainSplitter->setCollapsible(1, false);
    
    mainLayout->addWidget(mainSplitter);
    setCentralWidget(centralWidget);
}

void MainWindow::createMenuBar()
{
    QMenuBar *menuBar = new QMenuBar(this);
    setMenuBar(menuBar);
    
    // File Menu
    QMenu *fileMenu = menuBar->addMenu(tr("&File"));
    
    QAction *newProjectAction = fileMenu->addAction(tr("&New Project"));
    newProjectAction->setShortcut(QKeySequence::New);
    connect(newProjectAction, &QAction::triggered, this, &MainWindow::newProject);
    
    QAction *newFileAction = fileMenu->addAction(tr("New &File"));
    newFileAction->setShortcut(Qt::CTRL | Qt::SHIFT | Qt::Key_N);
    connect(newFileAction, &QAction::triggered, this, &MainWindow::newFile);
    
    fileMenu->addSeparator();
    
    QAction *openProjectAction = fileMenu->addAction(tr("&Open Project"));
    openProjectAction->setShortcut(Qt::CTRL | Qt::SHIFT | Qt::Key_O);
    connect(openProjectAction, &QAction::triggered, this, &MainWindow::openProject);
    
    QAction *openFileAction = fileMenu->addAction(tr("Open &File"));
    openFileAction->setShortcut(QKeySequence::Open);
    connect(openFileAction, &QAction::triggered, this, &MainWindow::openFile);
    
    fileMenu->addSeparator();
    
    QAction *saveAction = fileMenu->addAction(tr("&Save"));
    saveAction->setShortcut(QKeySequence::Save);
    connect(saveAction, &QAction::triggered, this, &MainWindow::saveFile);
    
    QAction *saveAsAction = fileMenu->addAction(tr("Save &As"));
    saveAsAction->setShortcut(QKeySequence::SaveAs);
    connect(saveAsAction, &QAction::triggered, this, &MainWindow::saveFileAs);
    
    QAction *saveAllAction = fileMenu->addAction(tr("Save A&ll"));
    saveAllAction->setShortcut(Qt::CTRL | Qt::SHIFT | Qt::Key_S);
    connect(saveAllAction, &QAction::triggered, this, &MainWindow::saveAll);
    
    fileMenu->addSeparator();
    
    QAction *exitAction = fileMenu->addAction(tr("E&xit"));
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction, &QAction::triggered, this, &MainWindow::exit);
    
    // Edit Menu
    QMenu *editMenu = menuBar->addMenu(tr("&Edit"));
    
    QAction *undoAction = editMenu->addAction(tr("&Undo"));
    undoAction->setShortcut(QKeySequence::Undo);
    connect(undoAction, &QAction::triggered, this, &MainWindow::undo);
    
    QAction *redoAction = editMenu->addAction(tr("&Redo"));
    redoAction->setShortcut(QKeySequence::Redo);
    connect(redoAction, &QAction::triggered, this, &MainWindow::redo);
    
    editMenu->addSeparator();
    
    QAction *cutAction = editMenu->addAction(tr("Cu&t"));
    cutAction->setShortcut(QKeySequence::Cut);
    connect(cutAction, &QAction::triggered, this, &MainWindow::cut);
    
    QAction *copyAction = editMenu->addAction(tr("&Copy"));
    copyAction->setShortcut(QKeySequence::Copy);
    connect(copyAction, &QAction::triggered, this, &MainWindow::copy);
    
    QAction *pasteAction = editMenu->addAction(tr("&Paste"));
    pasteAction->setShortcut(QKeySequence::Paste);
    connect(pasteAction, &QAction::triggered, this, &MainWindow::paste);
    
    editMenu->addSeparator();
    
    QAction *selectAllAction = editMenu->addAction(tr("Select &All"));
    selectAllAction->setShortcut(QKeySequence::SelectAll);
    connect(selectAllAction, &QAction::triggered, this, &MainWindow::selectAll);
    
    editMenu->addSeparator();
    
    QAction *findAction = editMenu->addAction(tr("&Find"));
    findAction->setShortcut(QKeySequence::Find);
    connect(findAction, &QAction::triggered, this, &MainWindow::find);
    
    QAction *replaceAction = editMenu->addAction(tr("&Replace"));
    replaceAction->setShortcut(QKeySequence::Replace);
    connect(replaceAction, &QAction::triggered, this, &MainWindow::replace);
    
    // Build Menu
    QMenu *buildMenu = menuBar->addMenu(tr("&Build"));
    
    QAction *compileAction = buildMenu->addAction(tr("&Compile"));
    compileAction->setShortcut(Qt::CTRL | Qt::Key_F9);
    connect(compileAction, &QAction::triggered, this, &MainWindow::compile);
    
    QAction *compileRunAction = buildMenu->addAction(tr("Compile & &Run"));
    compileRunAction->setShortcut(Qt::CTRL | Qt::Key_F10);
    connect(compileRunAction, &QAction::triggered, this, &MainWindow::compileAndRun);
    
    buildMenu->addSeparator();
    
    QAction *stopAction = buildMenu->addAction(tr("&Stop"));
    stopAction->setShortcut(Qt::CTRL | Qt::SHIFT | Qt::Key_F5);
    connect(stopAction, &QAction::triggered, this, &MainWindow::stop);
    
    // Debug Menu
    QMenu *debugMenu = menuBar->addMenu(tr("&Debug"));
    
    QAction *startDebugAction = debugMenu->addAction(tr("&Start Debugging"));
    startDebugAction->setShortcut(Qt::Key_F5);
    connect(startDebugAction, &QAction::triggered, this, &MainWindow::startDebug);
    
    QAction *stepOverAction = debugMenu->addAction(tr("&Step Over"));
    stepOverAction->setShortcut(Qt::Key_F10);
    connect(stepOverAction, &QAction::triggered, this, &MainWindow::stepOver);
    
    QAction *stepIntoAction = debugMenu->addAction(tr("Step &Into"));
    stepIntoAction->setShortcut(Qt::Key_F11);
    connect(stepIntoAction, &QAction::triggered, this, &MainWindow::stepInto);
    
    QAction *stepOutAction = debugMenu->addAction(tr("Step &Out"));
    stepOutAction->setShortcut(Qt::SHIFT | Qt::Key_F11);
    connect(stepOutAction, &QAction::triggered, this, &MainWindow::stepOut);
    
    debugMenu->addSeparator();
    
    QAction *breakpointAction = debugMenu->addAction(tr("Toggle &Breakpoint"));
    breakpointAction->setShortcut(Qt::CTRL | Qt::Key_B);
    connect(breakpointAction, &QAction::triggered, this, &MainWindow::toggleBreakpoint);
    
    QAction *continueAction = debugMenu->addAction(tr("&Continue"));
    continueAction->setShortcut(Qt::Key_F6);
    connect(continueAction, &QAction::triggered, this, &MainWindow::continueExecution);
    
    // Tools Menu
    QMenu *toolsMenu = menuBar->addMenu(tr("&Tools"));
    
    QAction *preferencesAction = toolsMenu->addAction(tr("&Preferences"));
    preferencesAction->setShortcut(QKeySequence::Preferences);
    connect(preferencesAction, &QAction::triggered, this, &MainWindow::preferences);
    
    QAction *compilersAction = toolsMenu->addAction(tr("&Manage Compilers"));
    connect(compilersAction, &QAction::triggered, this, &MainWindow::manageCompilers);
    
    // Help Menu
    QMenu *helpMenu = menuBar->addMenu(tr("&Help"));
    
    QAction *documentationAction = helpMenu->addAction(tr("&Documentation"));
    connect(documentationAction, &QAction::triggered, this, &MainWindow::documentation);
    
    helpMenu->addSeparator();
    
    QAction *aboutAction = helpMenu->addAction(tr("&About"));
    connect(aboutAction, &QAction::triggered, this, &MainWindow::about);
    
    QAction *aboutQtAction = helpMenu->addAction(tr("About &Qt"));
    connect(aboutQtAction, &QAction::triggered, this, &MainWindow::aboutQt);
}

void MainWindow::createToolBars()
{
    QToolBar *buildToolBar = addToolBar(tr("Build"));
    buildToolBar->setObjectName("BuildToolBar");
    buildToolBar->setIconSize(QSize(24, 24));
    
    buildToolBar->addAction(tr("Compile"), this, &MainWindow::compile);
    buildToolBar->addAction(tr("Compile & Run"), this, &MainWindow::compileAndRun);
    buildToolBar->addSeparator();
    buildToolBar->addAction(tr("Stop"), this, &MainWindow::stop);
    
    QToolBar *debugToolBar = addToolBar(tr("Debug"));
    debugToolBar->setObjectName("DebugToolBar");
    debugToolBar->setIconSize(QSize(24, 24));
    
    debugToolBar->addAction(tr("Debug"), this, &MainWindow::startDebug);
    debugToolBar->addAction(tr("Step Over"), this, &MainWindow::stepOver);
    debugToolBar->addAction(tr("Step Into"), this, &MainWindow::stepInto);
    debugToolBar->addAction(tr("Step Out"), this, &MainWindow::stepOut);
    debugToolBar->addAction(tr("Continue"), this, &MainWindow::continueExecution);
}

void MainWindow::createStatusBar()
{
    statusLabel = new QLabel(tr("Ready"));
    statusBar()->addWidget(statusLabel, 1);
    
    cursorPosLabel = new QLabel(tr("Line: 1, Col: 1"));
    statusBar()->addPermanentWidget(cursorPosLabel);
    
    compilationStatusLabel = new QLabel(tr("Idle"));
    statusBar()->addPermanentWidget(compilationStatusLabel);
    
    compilationProgress = new QProgressBar();
    compilationProgress->setMaximumWidth(150);
    compilationProgress->setVisible(false);
    statusBar()->addPermanentWidget(compilationProgress);
}

void MainWindow::setupTheme()
{
    // Dark theme stylesheet
    QString darkStylesheet = R"(
        QMainWindow {
            background-color: #2b2b2b;
            color: #ffffff;
        }
        QMenuBar {
            background-color: #3c3c3c;
            color: #ffffff;
            border-bottom: 1px solid #1e1e1e;
        }
        QMenuBar::item:selected {
            background-color: #0078d4;
        }
        QMenu {
            background-color: #3c3c3c;
            color: #ffffff;
            border: 1px solid #1e1e1e;
        }
        QMenu::item:selected {
            background-color: #0078d4;
        }
        QToolBar {
            background-color: #3c3c3c;
            color: #ffffff;
            border-bottom: 1px solid #1e1e1e;
        }
        QStatusBar {
            background-color: #3c3c3c;
            color: #ffffff;
            border-top: 1px solid #1e1e1e;
        }
        QPlainTextEdit {
            background-color: #1e1e1e;
            color: #ffffff;
            border: 1px solid #3c3c3c;
        }
        QTreeWidget {
            background-color: #252526;
            color: #ffffff;
            border: 1px solid #3c3c3c;
        }
        QTreeWidget::item:selected {
            background-color: #0078d4;
        }
        QSplitter::handle {
            background-color: #3c3c3c;
        }
        QScrollBar:vertical {
            background-color: #2b2b2b;
            width: 12px;
        }
        QScrollBar::handle:vertical {
            background-color: #555555;
            border-radius: 6px;
        }
        QScrollBar::handle:vertical:hover {
            background-color: #707070;
        }
    )";
    
    qApp->setStyle("Fusion");
    qApp->setStyleSheet(darkStylesheet);
}

void MainWindow::setupConnections()
{
    // Connect compiler signals
    connect(compiler.get(), QOverload<bool, const QString &>::of(&Compiler::compilationFinished),
            this, &MainWindow::onCompileFinished);
    connect(compiler.get(), &Compiler::compilationStarted, this, &MainWindow::onCompilationStarted);
    connect(compiler.get(), &Compiler::outputReceived, this, &MainWindow::onCompilationOutput);
    connect(compiler.get(), &Compiler::compilationWarning, this, &MainWindow::onCompilationWarning);
    connect(compiler.get(), &Compiler::compilationError, this, &MainWindow::onCompilationError);
    
    // Connect file manager signals
    connect(fileManager.get(), &FileManager::fileSelected,
            this, &MainWindow::onFileSelected);
    connect(fileManager.get(), &FileManager::fileDoubleClicked,
            this, &MainWindow::openFileAt);
    
    // Connect compiler panel error click
    connect(compilerPanel.get(), &CompilerOutputPanel::errorClicked,
            this, &MainWindow::openFileAt);
    
    // Connect editor signals
    connect(codeEditor.get(), &CodeEditor::modificationChanged,
            this, &MainWindow::onFileModified);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (isModified) {
        int ret = QMessageBox::warning(this, tr("Unsaved Changes"),
            tr("You have unsaved changes. Save before exiting?"),
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        
        if (ret == QMessageBox::Save) {
            saveFile();
            event->accept();
        } else if (ret == QMessageBox::Discard) {
            event->accept();
        } else {
            event->ignore();
        }
    } else {
        event->accept();
    }
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void MainWindow::dropEvent(QDropEvent *event)
{
    const QMimeData *mimeData = event->mimeData();
    if (mimeData->hasUrls()) {
        QStringList paths;
        foreach (const QUrl &url, mimeData->urls()) {
            paths << url.toLocalFile();
        }
        // Handle dropped files
        qDebug() << "Dropped files:" << paths;
    }
}

// File menu slots
void MainWindow::newProject() { qDebug() << "New Project"; }
void MainWindow::newFile() { qDebug() << "New File"; }
void MainWindow::openProject()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Project"));
    if (!dir.isEmpty()) {
        currentProjectPath = dir;
        fileManager->openProject(dir);
        statusLabel->setText(tr("Project: %1").arg(QFileInfo(dir).fileName()));
    }
}
void MainWindow::openFile()
{
    QString file = QFileDialog::getOpenFileName(this, tr("Open File"));
    if (!file.isEmpty()) {
        currentFilePath = file;
        // Ask file manager to open/select
        fileManager->openProject(QFileInfo(file).absolutePath());
        codeEditor->openFile(file);
    }
}
void MainWindow::saveFile()
{
    if (currentFilePath.isEmpty()) {
        saveFileAs();
        return;
    }
    codeEditor->saveFile(currentFilePath);
}

void MainWindow::saveFileAs()
{
    QString file = QFileDialog::getSaveFileName(this, tr("Save File As"),
        currentFilePath.isEmpty() ? currentProjectPath : currentFilePath,
        tr("C++ Files (*.cpp *.cxx *.cc);;C Files (*.c);;Header Files (*.h *.hpp);;All Files (*)"));
    if (!file.isEmpty()) {
        currentFilePath = file;
        codeEditor->saveFile(currentFilePath);
    }
}

void MainWindow::saveAll()
{
    saveFile();
}

void MainWindow::closeFile()
{
    if (codeEditor->isModified()) {
        auto result = QMessageBox::question(this, tr("Unsaved Changes"),
            tr("Save changes before closing?"),
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        if (result == QMessageBox::Save)
            saveFile();
        else if (result == QMessageBox::Cancel)
            return;
    }
    codeEditor->clear();
    currentFilePath.clear();
}
void MainWindow::exit() { close(); }

// Edit menu slots
void MainWindow::undo() { codeEditor->undo(); }
void MainWindow::redo() { codeEditor->redo(); }
void MainWindow::cut() { codeEditor->cut(); }
void MainWindow::copy() { codeEditor->copy(); }
void MainWindow::paste() { codeEditor->paste(); }
void MainWindow::selectAll() { codeEditor->selectAll(); }
void MainWindow::find() { qDebug() << "Find — not yet implemented"; }
void MainWindow::replace() { qDebug() << "Replace — not yet implemented"; }

// Build menu slots
void MainWindow::compile()
{
    // Clear console
    compilerPanel->clear();
    runAfterCompile = false;

    if (!currentProjectPath.isEmpty()) {
        // compile project
        // compute expected output and store
        ProjectConfig cfg;
        QString configFile;
        QFileInfo pinfo(currentProjectPath);
        if (pinfo.isDir()) {
            QStringList candidates = {"project.json", "project.conf", "coderunner.json", "projectconfig.json"};
            for (const QString &name : candidates) {
                QString candidate = QDir(currentProjectPath).filePath(name);
                if (QFileInfo::exists(candidate)) { configFile = candidate; break; }
            }
            if (!configFile.isEmpty()) cfg.load(configFile);
            else { cfg.setProjectPath(currentProjectPath); cfg.setProjectName(pinfo.fileName()); }
        } else {
            // not a dir, maybe a file
            cfg.setProjectPath(pinfo.absolutePath()); cfg.setProjectName(pinfo.baseName());
        }
        QString binPath = QDir(cfg.getProjectPath()).filePath(cfg.getBinDir());
        QDir().mkpath(binPath);
        QString out = QDir(binPath).filePath(cfg.getProjectName());
#ifdef Q_OS_WIN
        out += ".exe";
#endif
        lastBuildOutputPath = out;
        compilationStatusLabel->setText(tr("Building project..."));
        isCompiling = true;
        compiler->compileProject(currentProjectPath);
    } else if (!currentFilePath.isEmpty()) {
        // compile single file
        QFileInfo fi(currentFilePath);
        QString outDir = fi.absolutePath();
        QString out = QDir(outDir).filePath(fi.completeBaseName());
#ifdef Q_OS_WIN
        out += ".exe";
#endif
        lastBuildOutputPath = out;
        compilationStatusLabel->setText(tr("Building file..."));
        isCompiling = true;
        compiler->compileFile(currentFilePath, lastBuildOutputPath);
    } else {
        QMessageBox::information(this, tr("Build"), tr("No project or file selected to build."));
    }
}

void MainWindow::compileAndRun()
{
    runAfterCompile = true;
    compile();
}

void MainWindow::stop()
{
    // Stop compilation or running process
    if (isCompiling) {
        compiler->stop();
        isCompiling = false;
        compilationStatusLabel->setText(tr("Stopped"));
    }

    if (runProcess && runProcess->state() == QProcess::Running) {
        runProcess->terminate();
        if (!runProcess->waitForFinished(2000)) {
            runProcess->kill();
            runProcess->waitForFinished();
        }
        compilationStatusLabel->setText(tr("Run stopped"));
    }
}

// Debug menu slots
void MainWindow::startDebug() { qDebug() << "Start Debug"; }
void MainWindow::stepOver() { qDebug() << "Step Over"; }
void MainWindow::stepInto() { qDebug() << "Step Into"; }
void MainWindow::stepOut() { qDebug() << "Step Out"; }
void MainWindow::toggleBreakpoint() { qDebug() << "Toggle Breakpoint"; }
void MainWindow::continueExecution() { qDebug() << "Continue"; }

// Tools menu slots
void MainWindow::preferences() { qDebug() << "Preferences"; }
void MainWindow::manageCompilers() { qDebug() << "Manage Compilers"; }

// Help menu slots
void MainWindow::about()
{
    QMessageBox::about(this, tr("About CodeRunner"),
        tr("CodeRunner 1.0.0\n\n"
           "A lightweight, cross-platform C/C++ IDE.\n\n"
           "Built with Qt 5/6\n"
           "License: Mozilla Public License 2.0"));
}

void MainWindow::aboutQt()
{
    QMessageBox::aboutQt(this, tr("About Qt"));
}

void MainWindow::documentation() { qDebug() << "Documentation"; }

// New compilation/run signal handlers
void MainWindow::onCompilationStarted()
{
    compilationProgress->setVisible(true);
    compilationProgress->setRange(0, 0); // marquee
    compilationStatusLabel->setText(tr("Compiling..."));
}

void MainWindow::onCompilationOutput(const QString &output)
{
    // Feed incremental output into the parser/panel
    feedOutputToPanel(compilerPanel.get(), output);
}

void MainWindow::onCompilationWarning(const QString &warning)
{
    feedOutputToPanel(compilerPanel.get(), warning);
}

void MainWindow::onCompilationError(const QString &error)
{
    feedOutputToPanel(compilerPanel.get(), error);
}

void MainWindow::onRunProcessOutput()
{
    if (!runProcess) return;
    QString out = QString::fromUtf8(runProcess->readAllStandardOutput());
    compilerPanel->addOutput(out);
}

void MainWindow::onRunProcessError()
{
    if (!runProcess) return;
    QString err = QString::fromUtf8(runProcess->readAllStandardError());
    compilerPanel->addOutput(err);
}

void MainWindow::onRunProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitStatus);
    QString msg = tr("Program exited with code %1").arg(exitCode);
    compilerPanel->addOutput(msg);
    compilationStatusLabel->setText(tr("Run finished"));
    runAfterCompile = false;
}

void MainWindow::onCompileFinished(bool success, const QString &output)
{
    // Final output: ensure full capture
    feedOutputToPanel(compilerPanel.get(), output);
    compilationProgress->setVisible(false);
    if (success) {
        statusLabel->setText(tr("Compilation successful"));
        compilationStatusLabel->setText(tr("✓ Success"));
    } else {
        statusLabel->setText(tr("Compilation failed"));
        compilationStatusLabel->setText(tr("✗ Failed"));
    }

    isCompiling = false;

    // If requested, run the produced binary
    if (success && runAfterCompile && !lastBuildOutputPath.isEmpty()) {
        runProcess = std::make_unique<QProcess>(this);
        connect(runProcess.get(), &QProcess::readyReadStandardOutput, this, &MainWindow::onRunProcessOutput);
        connect(runProcess.get(), &QProcess::readyReadStandardError, this, &MainWindow::onRunProcessError);
        connect(runProcess.get(), QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                this, &MainWindow::onRunProcessFinished);

        compilationStatusLabel->setText(tr("Running..."));
        runProcess->start(lastBuildOutputPath);
        if (!runProcess->waitForStarted(1000)) {
            compilerPanel->addOutput(tr("Failed to start program: %1").arg(lastBuildOutputPath));
            runAfterCompile = false;
        }
    }
}

void MainWindow::openFileAt(const QString &file, int line, int column)
{
    QString path = file;
    if (path.isEmpty()) return;

    // If path is relative, resolve against project
    QFileInfo fi(path);
    if (!fi.isAbsolute() && !currentProjectPath.isEmpty()) {
        path = QDir(currentProjectPath).filePath(path);
        fi = QFileInfo(path);
    }

    if (!fi.exists()) {
        compilerPanel->addOutput(tr("Cannot open file: %1").arg(path));
        return;
    }

    // Ensure project tree is aware of the project
    fileManager->openProject(fi.absolutePath());
    codeEditor->openFile(path);
    codeEditor->gotoLine(line);
    codeEditor->setFocus();

    QString msg = tr("Opened %1 at line %2").arg(fi.fileName()).arg(line);
    statusLabel->setText(msg);
}

void MainWindow::onFileSelected(const QString &filepath)
{
    currentFilePath = filepath;
    qDebug() << "File selected:" << filepath;
}

void MainWindow::onFileModified(bool modified)
{
    isModified = modified;
    QString title = "CodeRunner";
    if (modified) {
        title += " *";
    }
    setWindowTitle(title);
}

void MainWindow::loadSettings()
{
    QSettings settings("CodeRunner", "CodeRunner");
    // Load window geometry
    restoreGeometry(settings.value("geometry", saveGeometry()).toByteArray());
    restoreState(settings.value("windowState", saveState()).toByteArray());
}

void MainWindow::saveSettings()
{
    QSettings settings("CodeRunner", "CodeRunner");
    settings.setValue("geometry", saveGeometry());
    settings.setValue("windowState", saveState());
}
