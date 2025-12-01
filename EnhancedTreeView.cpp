#include "EnhancedTreeView.hpp"
#include <QKeyEvent>
#include <QStandardItem>
#include <QStandardItemModel>

EnhancedTreeView::EnhancedTreeView(QWidget* parent) : QTreeView(parent) {}

void EnhancedTreeView::expandAllItems() {
    expandAll();
}

void EnhancedTreeView::collapseAllItems() {
    collapseAll();
}

void EnhancedTreeView::expandSelectedItems() {
    QModelIndexList selected = selectedIndexes();
    for (const QModelIndex& index : selected) {
        expand(index);
    }
}

void EnhancedTreeView::collapseSelectedItems() {
    QModelIndexList selected = selectedIndexes();
    for (const QModelIndex& index : selected) {
        collapse(index);
    }
}

void EnhancedTreeView::toggleExpandCollapseSelectedItems() {
    QModelIndexList selected = selectedIndexes();
    for (const QModelIndex& index : selected) {
        if (isExpanded(index)) {
            collapse(index);
        } else {
            expand(index);
        }
    }
}

void EnhancedTreeView::deleteSelectedItems() {
    QModelIndexList selected = selectedIndexes();
    auto* model = qobject_cast<QStandardItemModel*>(this->model());

    for (const QModelIndex& index : selected) {
        if (model) {
            model->removeRow(index.row(), index.parent());
        }
    }
}

QModelIndex EnhancedTreeView::addItem(const QStringList& itemData, const QModelIndex& parent) {
    auto* model = qobject_cast<QStandardItemModel*>(this->model());

    if (model) {
        QList<QStandardItem*> items;
        for (const QString& data : itemData) {
            items.append(new QStandardItem(data));
        }

        model->invisibleRootItem()->appendRow(items);

        QModelIndex newIndex = model->index(model->rowCount(parent) - 1, 0, parent);
        setCurrentIndex(newIndex);

        return newIndex;
    }

    return {};
}

void EnhancedTreeView::setHeaders(const QStringList& headers) {
    auto* model = qobject_cast<QStandardItemModel*>(this->model());

    if (model) {
        model->setHorizontalHeaderLabels(headers);
    }
}

QModelIndexList EnhancedTreeView::getCurrentSelection() const {
    return selectedIndexes();
}

void EnhancedTreeView::setItemIcon(const QModelIndex& index, const QIcon& icon) {
    auto* model = qobject_cast<QStandardItemModel*>(this->model());

    if (model) {
        QStandardItem* item = model->itemFromIndex(index);
        if (item) {
            item->setIcon(icon);
        }
    }
}

void EnhancedTreeView::expandCollapseItems(const QModelIndex& index, bool expandNode) {  // NOLINT
    if (!index.isValid()) {
        return;
    }

    if (expandNode) {
        expand(index);
    } else {
        collapse(index);
    }

    int rowCount = model()->rowCount(index);
    for (int i = 0; i < rowCount; ++i) {
        QModelIndex childIndex = model()->index(i, 0, index);
        expandCollapseItems(childIndex, expandNode);
    }
}

void EnhancedTreeView::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Space) {
        toggleExpandCollapseSelectedItems();
        event->accept();
    } else {
        QTreeView::keyPressEvent(event);
    }
}

void EnhancedTreeView::clear() {
    auto* model = qobject_cast<QStandardItemModel*>(this->model());

    if (model) {
        model->clear();
    }
}
