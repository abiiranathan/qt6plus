#ifndef ENHANCEDTREEVIEW_H
#define ENHANCEDTREEVIEW_H

#include <QModelIndex>
#include <QStringList>
#include <QTreeView>

class EnhancedTreeView : public QTreeView {
    Q_OBJECT

   public:
    explicit EnhancedTreeView(QWidget* parent = nullptr);

    void expandAllItems();
    void collapseAllItems();
    void expandSelectedItems();
    void collapseSelectedItems();
    void toggleExpandCollapseSelectedItems();
    //Clears the contents of the EnhancedTreeView.
    void clear();

    void deleteSelectedItems();
    QModelIndex addItem(const QStringList& itemData, const QModelIndex& parent = QModelIndex());

    void setHeaders(const QStringList& headers);
    [[nodiscard]] QModelIndexList getCurrentSelection() const;
    void setItemIcon(const QModelIndex& index, const QIcon& icon);

   protected:
    void keyPressEvent(QKeyEvent* event) override;

   private:
    void expandCollapseItems(const QModelIndex& index, bool expand);
};

#endif  // ENHANCEDTREEVIEW_H
