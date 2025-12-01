#include <QAction>
#include <QApplication>
#include <QFileSystemModel>
#include <QInputDialog>
#include <QMenu>
#include "EnhancedTreeView.hpp"

int main(int argc, char* argv[]) {
    QApplication a(argc, argv);

    // Create the file system model and set the root directory
    QFileSystemModel model;
    model.setRootPath(QDir::rootPath());

    // Create the EnhancedTreeView and set the model
    EnhancedTreeView treeView;
    treeView.setModel(&model);
    treeView.setHeaders(QStringList() << "Files");

    // Create the context menu actions
    auto* createDirAction = new QAction("Create Directory", &treeView);
    auto* deleteAction = new QAction("Delete", &treeView);

    // Connect the actions to their respective slots
    QObject::connect(createDirAction, &QAction::triggered, [&]() {
        QModelIndex currentIndex = treeView.currentIndex();
        QString currentPath = model.filePath(currentIndex);
        QString dirName =
            QInputDialog::getText(&treeView, "Create Directory", "Enter directory name:");
        if (!dirName.isEmpty()) {
            QString newDirPath = currentPath + QDir::separator() + dirName;
            model.mkdir(currentIndex, dirName);
            treeView.setCurrentIndex(model.index(newDirPath));
        }
    });

    QObject::connect(deleteAction, &QAction::triggered, [&]() {
        QModelIndex currentIndex = treeView.currentIndex();
        if (currentIndex.isValid()) {
            model.remove(currentIndex);
        }
    });

    // Set the context menu policy to CustomContextMenu
    treeView.setContextMenuPolicy(Qt::CustomContextMenu);

    // Connect the customContextMenuRequested signal to show the context menu
    QObject::connect(&treeView, &QWidget::customContextMenuRequested, [&](const QPoint& pos) {
        QMenu contextMenu(&treeView);
        contextMenu.addAction(createDirAction);
        contextMenu.addAction(deleteAction);
        contextMenu.exec(treeView.mapToGlobal(pos));
    });

    treeView.show();

    return a.exec();
}
