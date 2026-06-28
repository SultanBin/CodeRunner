#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <QWidget>
#include <QTreeWidget>
#include <QString>
#include <QFileSystemWatcher>
#include <memory>

class FileManager : public QWidget
{
    Q_OBJECT

public:
    FileManager(QWidget *parent = nullptr);
    ~FileManager();
    
    // Project operations
    void openProject(const QString &projectPath);
    void closeProject();
    void createNewProject(const QString &projectName, const QString &location);
    void createNewFile(const QString &filename, const QString &parentPath);
    void createNewFolder(const QString &foldername, const QString &parentPath);
    
    // File operations
    void renameItem(QTreeWidgetItem *item, const QString &newName);
    void deleteItem(QTreeWidgetItem *item);
    
    // Query
    QString getCurrentProjectPath() const { return currentProjectPath; }
    QString getSelectedFilePath() const;
    
signals:
    void fileSelected(const QString &filepath);
    void fileDoubleClicked(const QString &filepath);
    void projectOpened(const QString &projectPath);
    void projectClosed();
    void fileCreated(const QString &filepath);
    void fileDeleted(const QString &filepath);
    void fileRenamed(const QString &oldPath, const QString &newPath);

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;

private slots:
    void onItemSelected(QTreeWidgetItem *item, int column);
    void onItemDoubleClicked(QTreeWidgetItem *item, int column);
    void onFileSystemChanged(const QString &path);
    void refreshProjectTree();
    void showContextMenu(const QPoint &pos);

private:
    void populateTreeFromPath(const QString &path, QTreeWidgetItem *parentItem);
    QTreeWidgetItem *findItemByPath(const QString &path);
    void setupUI();
    
    QTreeWidget *projectTree;
    QString currentProjectPath;
    std::unique_ptr<QFileSystemWatcher> fileWatcher;
    QTreeWidgetItem *rootItem;
};

#endif // FILEMANAGER_H
