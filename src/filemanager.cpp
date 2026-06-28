#include "filemanager.h"
#include <QVBoxLayout>
#include <QHeaderView>
#include <QDir>
#include <QFileInfo>
#include <QContextMenuEvent>
#include <QMenu>
#include <QDebug>
#include <QFileSystemWatcher>
#include <QMimeData>
#include <QDragEnterEvent>
#include <QDropEvent>

FileManager::FileManager(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
    fileWatcher = std::make_unique<QFileSystemWatcher>();
    connect(fileWatcher.get(), &QFileSystemWatcher::directoryChanged,
            this, &FileManager::onFileSystemChanged);
}

FileManager::~FileManager()
{
}

void FileManager::setupUI()
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    
    projectTree = new QTreeWidget();
    projectTree->setHeaderLabel(tr("Project"));
    projectTree->setContextMenuPolicy(Qt::CustomContextMenu);
    projectTree->setIndentation(15);
    projectTree->setAnimated(true);
    projectTree->setUniformRowHeights(true);
    
    layout->addWidget(projectTree);
    
    connect(projectTree, &QTreeWidget::itemClicked,
            this, &FileManager::onItemSelected);
    connect(projectTree, &QTreeWidget::itemDoubleClicked,
            this, &FileManager::onItemDoubleClicked);
    connect(projectTree, &QTreeWidget::customContextMenuRequested,
            this, &FileManager::showContextMenu);
}

void FileManager::openProject(const QString &projectPath)
{
    QDir dir(projectPath);
    if (!dir.exists()) {
        qWarning() << "Project path does not exist:" << projectPath;
        return;
    }
    
    currentProjectPath = projectPath;
    projectTree->clear();
    
    // Create root item
    rootItem = new QTreeWidgetItem();
    rootItem->setText(0, dir.dirName());
    projectTree->addTopLevelItem(rootItem);
    
    // Populate tree
    populateTreeFromPath(projectPath, rootItem);
    projectTree->expandItem(rootItem);
    
    // Watch for changes
    fileWatcher->addPath(projectPath);
    
    emit projectOpened(projectPath);
}

void FileManager::closeProject()
{
    if (!currentProjectPath.isEmpty()) {
        fileWatcher->removePath(currentProjectPath);
    }
    projectTree->clear();
    currentProjectPath.clear();
    emit projectClosed();
}

void FileManager::createNewProject(const QString &projectName, const QString &location)
{
    QDir dir(location);
    if (!dir.mkdir(projectName)) {
        qWarning() << "Cannot create project directory";
        return;
    }
    
    QString projectPath = location + "/" + projectName;
    
    // Create src folder
    dir.cd(projectName);
    dir.mkdir("src");
    dir.mkdir("bin");
    dir.mkdir("obj");
    
    openProject(projectPath);
}

void FileManager::createNewFile(const QString &filename, const QString &parentPath)
{
    QString filepath = parentPath + "/" + filename;
    QFile file(filepath);
    if (file.open(QFile::WriteOnly)) {
        file.close();
        emit fileCreated(filepath);
        refreshProjectTree();
    }
}

void FileManager::createNewFolder(const QString &foldername, const QString &parentPath)
{
    QDir dir(parentPath);
    if (dir.mkdir(foldername)) {
        emit fileCreated(parentPath + "/" + foldername);
        refreshProjectTree();
    }
}

void FileManager::renameItem(QTreeWidgetItem *item, const QString &newName)
{
    QString oldPath = getSelectedFilePath();
    item->setText(0, newName);
    // TODO: Implement actual rename on disk
}

void FileManager::deleteItem(QTreeWidgetItem *item)
{
    QString path = getSelectedFilePath();
    QFileInfo info(path);
    
    if (info.isFile()) {
        QFile::remove(path);
    } else if (info.isDir()) {
        QDir dir(path);
        dir.removeRecursively();
    }
    
    emit fileDeleted(path);
    refreshProjectTree();
}

QString FileManager::getSelectedFilePath() const
{
    QTreeWidgetItem *item = projectTree->currentItem();
    if (!item) return QString();
    
    // Build path from tree hierarchy
    QString path;
    QTreeWidgetItem *current = item;
    while (current != nullptr) {
        if (current == rootItem) {
            path.prepend(currentProjectPath);
            break;
        }
        path.prepend("/" + current->text(0));
        current = current->parent();
    }
    
    return path;
}

void FileManager::onItemSelected(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column);
    QString filepath = getSelectedFilePath();
    QFileInfo info(filepath);
    
    if (info.isFile()) {
        emit fileSelected(filepath);
    }
}

void FileManager::onItemDoubleClicked(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column);
    QString filepath = getSelectedFilePath();
    QFileInfo info(filepath);
    
    if (info.isFile()) {
        emit fileDoubleClicked(filepath);
    } else if (info.isDir()) {
        // Toggle expand/collapse
        if (projectTree->isItemExpanded(item)) {
            projectTree->collapseItem(item);
        } else {
            projectTree->expandItem(item);
        }
    }
}

void FileManager::onFileSystemChanged(const QString &path)
{
    qDebug() << "File system changed:" << path;
    refreshProjectTree();
}

void FileManager::refreshProjectTree()
{
    if (!currentProjectPath.isEmpty()) {
        projectTree->clear();
        QDir dir(currentProjectPath);
        rootItem = new QTreeWidgetItem();
        rootItem->setText(0, dir.dirName());
        projectTree->addTopLevelItem(rootItem);
        populateTreeFromPath(currentProjectPath, rootItem);
        projectTree->expandItem(rootItem);
    }
}

void FileManager::populateTreeFromPath(const QString &path, QTreeWidgetItem *parentItem)
{
    QDir dir(path);
    dir.setFilter(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);
    dir.setSorting(QDir::DirsFirst | QDir::Name);
    
    foreach (const QFileInfo &fileInfo, dir.entryInfoList()) {
        QTreeWidgetItem *item = new QTreeWidgetItem();
        item->setText(0, fileInfo.fileName());
        
        if (fileInfo.isDir()) {
            // Set folder icon
            item->setIcon(0, QIcon(":/icons/folder.png"));
            parentItem->addChild(item);
            // Recursively add subdirectories
            populateTreeFromPath(fileInfo.absoluteFilePath(), item);
        } else {
            // Set file icon based on extension
            if (fileInfo.suffix() == "cpp" || fileInfo.suffix() == "c" || 
                fileInfo.suffix() == "h" || fileInfo.suffix() == "hpp") {
                item->setIcon(0, QIcon(":/icons/file_cpp.png"));
            } else {
                item->setIcon(0, QIcon(":/icons/file.png"));
            }
            parentItem->addChild(item);
        }
    }
}

QTreeWidgetItem *FileManager::findItemByPath(const QString &path)
{
    // TODO: Implement search for item by path
    return nullptr;
}

void FileManager::showContextMenu(const QPoint &pos)
{
    QTreeWidgetItem *item = projectTree->itemAt(pos);
    if (!item) return;
    
    QMenu menu;
    menu.addAction(tr("New File"));
    menu.addAction(tr("New Folder"));
    menu.addSeparator();
    menu.addAction(tr("Rename"));
    menu.addAction(tr("Delete"));
    menu.addSeparator();
    menu.addAction(tr("Expand All"));
    menu.addAction(tr("Collapse All"));
    
    menu.exec(projectTree->mapToGlobal(pos));
}
