*** Begin Patch
*** Update File: src/mainwindow.cpp
@@
-#include <QDragEnterEvent>
-#include <QMimeData>
-#include <QDebug>
+#include <QDragEnterEvent>
+#include <QMimeData>
+#include <QDebug>
+#include <QProcess>
+#include <QFileInfo>
+#include <QDir>
@@
-MainWindow::MainWindow(QWidget *parent)
-    : QMainWindow(parent), isModified(false), isCompiling(false), isDebugging(false)
+MainWindow::MainWindow(QWidget *parent)
+    : QMainWindow(parent), isModified(false), isCompiling(false), isDebugging(false)
 {
@@
-    // Create components
+    // Create components
     fileManager = std::make_unique<FileManager>();
     codeEditor = std::make_unique<CodeEditor>();
     compiler = std::make_unique<Compiler>();
     debugger = std::make_unique<Debugger>();
+    runProcess = nullptr;
@@
 void MainWindow::setupConnections()
 {
-    // Connect compiler signals
-    connect(compiler.get(), QOverload<bool, const QString &>::of(&Compiler::compilationFinished),
-            this, &MainWindow::onCompileFinished);
+    // Connect compiler signals
+    connect(compiler.get(), QOverload<bool, const QString &>::of(&Compiler::compilationFinished),
+            this, &MainWindow::onCompileFinished);
+    connect(compiler.get(), &Compiler::compilationStarted, this, &MainWindow::onCompilationStarted);
+    connect(compiler.get(), &Compiler::outputReceived, this, &MainWindow::onCompilationOutput);
+    connect(compiler.get(), &Compiler::compilationWarning, this, &MainWindow::onCompilationWarning);
+    connect(compiler.get(), &Compiler::compilationError, this, &MainWindow::onCompilationError);
@@
-// Build menu slots
-void MainWindow::compile() { qDebug() << "Compile"; }
-void MainWindow::compileAndRun() { qDebug() << "Compile & Run"; }
-void MainWindow::stop() { qDebug() << "Stop"; }
+// Build menu slots
+void MainWindow::compile()
+{
+    // Clear console
+    outputConsole->clear();
+    runAfterCompile = false;
+
+    if (!currentProjectPath.isEmpty()) {
+        // compile project
+        // compute expected output and store
+        ProjectConfig cfg;
+        QString configFile;
+        QFileInfo pinfo(currentProjectPath);
+        if (pinfo.isDir()) {
+            QStringList candidates = {"project.json", "project.conf", "coderunner.json", "projectconfig.json"};
+            for (const QString &name : candidates) {
+                QString candidate = QDir(currentProjectPath).filePath(name);
+                if (QFileInfo::exists(candidate)) { configFile = candidate; break; }
+            }
+            if (!configFile.isEmpty()) cfg.load(configFile);
+            else { cfg.setProjectPath(currentProjectPath); cfg.setProjectName(pinfo.fileName()); }
+        } else {
+            // not a dir, maybe a file
+            cfg.setProjectPath(pinfo.absolutePath()); cfg.setProjectName(pinfo.baseName());
+        }
+        QString binPath = QDir(cfg.getProjectPath()).filePath(cfg.getBinDir());
+        QDir().mkpath(binPath);
+        QString out = QDir(binPath).filePath(cfg.getProjectName());
+#ifdef Q_OS_WIN
+        out += ".exe";
+#endif
+        lastBuildOutputPath = out;
+        compilationStatusLabel->setText(tr("Building project..."));
+        isCompiling = true;
+        compiler->compileProject(currentProjectPath);
+    } else if (!currentFilePath.isEmpty()) {
+        // compile single file
+        QFileInfo fi(currentFilePath);
+        QString outDir = fi.absolutePath();
+        QString out = QDir(outDir).filePath(fi.completeBaseName());
+#ifdef Q_OS_WIN
+        out += ".exe";
+#endif
+        lastBuildOutputPath = out;
+        compilationStatusLabel->setText(tr("Building file..."));
+        isCompiling = true;
+        compiler->compileFile(currentFilePath, lastBuildOutputPath);
+    } else {
+        QMessageBox::information(this, tr("Build"), tr("No project or file selected to build."));
+    }
+}
+
+void MainWindow::compileAndRun()
+{
+    runAfterCompile = true;
+    compile();
+}
+
+void MainWindow::stop()
+{
+    // Stop compilation or running process
+    if (isCompiling) {
+        compiler->stop();
+        isCompiling = false;
+        compilationStatusLabel->setText(tr("Stopped"));
+    }
+
+    if (runProcess && runProcess->state() == QProcess::Running) {
+        runProcess->terminate();
+        if (!runProcess->waitForFinished(2000)) {
+            runProcess->kill();
+            runProcess->waitForFinished();
+        }
+        compilationStatusLabel->setText(tr("Run stopped"));
+    }
+}
@@
-// Signal handlers
-void MainWindow::onCompileFinished(bool success, const QString &output)
-{
-    outputConsole->setPlainText(output);
-    if (success) {
-        statusLabel->setText(tr("Compilation successful"));
-        compilationStatusLabel->setText(tr("✓ Success"));
-    } else {
-        statusLabel->setText(tr("Compilation failed"));
-        compilationStatusLabel->setText(tr("✗ Failed"));
-    }
-}
+// New compilation/run signal handlers
+void MainWindow::onCompilationStarted()
+{
+    compilationProgress->setVisible(true);
+    compilationProgress->setRange(0, 0); // marquee
+    compilationStatusLabel->setText(tr("Compiling..."));
+}
+
+void MainWindow::onCompilationOutput(const QString &output)
+{
+    // Append incremental output
+    outputConsole->appendPlainText(output);
+}
+
+void MainWindow::onCompilationWarning(const QString &warning)
+{
+    outputConsole->appendPlainText(tr("[warning] %1").arg(warning));
+}
+
+void MainWindow::onCompilationError(const QString &error)
+{
+    outputConsole->appendPlainText(tr("[error] %1").arg(error));
+}
+
+void MainWindow::onRunProcessOutput()
+{
+    if (!runProcess) return;
+    QString out = QString::fromUtf8(runProcess->readAllStandardOutput());
+    outputConsole->appendPlainText(out);
+}
+
+void MainWindow::onRunProcessError()
+{
+    if (!runProcess) return;
+    QString err = QString::fromUtf8(runProcess->readAllStandardError());
+    outputConsole->appendPlainText(err);
+}
+
+void MainWindow::onRunProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
+{
+    Q_UNUSED(exitStatus);
+    QString msg = tr("Program exited with code %1").arg(exitCode);
+    outputConsole->appendPlainText(msg);
+    compilationStatusLabel->setText(tr("Run finished"));
+    runAfterCompile = false;
+}
+
+void MainWindow::onCompileFinished(bool success, const QString &output)
+{
+    outputConsole->setPlainText(output);
+    compilationProgress->setVisible(false);
+    if (success) {
+        statusLabel->setText(tr("Compilation successful"));
+        compilationStatusLabel->setText(tr("✓ Success"));
+    } else {
+        statusLabel->setText(tr("Compilation failed"));
+        compilationStatusLabel->setText(tr("✗ Failed"));
+    }
+
+    isCompiling = false;
+
+    // If requested, run the produced binary
+    if (success && runAfterCompile && !lastBuildOutputPath.isEmpty()) {
+        runProcess = std::make_unique<QProcess>(this);
+        connect(runProcess.get(), &QProcess::readyReadStandardOutput, this, &MainWindow::onRunProcessOutput);
+        connect(runProcess.get(), &QProcess::readyReadStandardError, this, &MainWindow::onRunProcessError);
+        connect(runProcess.get(), QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
+                this, &MainWindow::onRunProcessFinished);
+
+        compilationStatusLabel->setText(tr("Running..."));
+        runProcess->start(lastBuildOutputPath);
+        if (!runProcess->waitForStarted(1000)) {
+            outputConsole->appendPlainText(tr("Failed to start program: %1").arg(lastBuildOutputPath));
+            runAfterCompile = false;
+        }
+    }
+}
*** End Patch