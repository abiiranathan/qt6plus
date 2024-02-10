#ifndef TABLE_WIDGET_H
#define TABLE_WIDGET_H

#include <QHeaderView>
#include <QList>
#include <QPainter>
#include <QPrintDialog>
#include <QPrintPreviewDialog>
#include <QPrintPreviewWidget>
#include <QPrinter>
#include <QRegularExpression>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QtWidgets>
#include <optional>
#include <tuple>

// Remove the '-mno-direct-extern-access' flag
// #ifndef TABLE_WIDGET_H

class HtmlPreviewWidget : public QPrintPreviewWidget {
   public:
    HtmlPreviewWidget(QString html);

   protected:
    void paintEvent(QPaintEvent* event) override;

   private:
    QString htmlContent;
};

class CustomTableModel : public QStandardItemModel {
    Q_OBJECT

   public:
    explicit CustomTableModel(const QList<int>& editableColumns, const QList<int>& disabledColumns,
                              QObject* parent = nullptr);

    Qt::ItemFlags flags(const QModelIndex& index) const override;

   private:
    QList<int> editableColumns;
    QList<int> disabledColumns;
};

class TableWidget : public QTableView {
    Q_OBJECT

   public:
    QString title;
    QUrl logo;

    /**
     * Constructor for the TableWidget.
     */
    explicit TableWidget(QWidget* parent = nullptr, QList<int> editableColumns = QList<int>{},
                         QList<int> disabledColumns = QList<int>{});

    // Destructor
    ~TableWidget();

    // Returns the number of rows
    int rowCount() const;

    // Returns the number of columns
    int columnCount() const;

    // Resize headers to fit content
    void fit();

    // Resize headers, stretching them to fill parent
    void stretch();

    // Set Interactive resizable headers
    void interactive();

    // Sets the column to filter on. Default -1 (all columns)
    void setFilterKeyColumn(int column);

    void setContextMenuEnabled(bool enabled);

    // Set table horizontal headers.
    // fieldNames should be equal in length to horizontalHeaders(otherwise won't be used)
    // fieldNames are used in generating JSON and CSV data.
    void setHorizontalHeaders(const QStringList& horizontalHeaders,
                              const QStringList& fields = QStringList());

    // fieldNames should be equal in length to horizontalHeaders(otherwise won't be used)
    // fieldNames are used in generating JSON and CSV data.
    void setFieldNames(const QStringList& fieldNames_);

    /**
     * Sets vertical headers for the table.
     */
    void setVerticalHeaders(const QStringList& headers);

    void resetHeaders();

    /**
     * Populates the table with data.
     */
    void setData(const QVector<QStringList>& data);

    // Sets the signals and slots for double click on table. Calls handler with data for
    // the double-clicked row.
    void setDoubleClickHandler(std::function<void(int row, int col, const QStringList& data)> handler);

    // Generates an html table and writes it to a QString that is returned.
    QString generateHtmlTable();

    // Generates and returns QString containing CSV for the table data.
    QString generateCsvData();

    // Generates and returns QString containing JSON for the table data.
    // The valueConverter is required if you want to convert cell data to other types from QString.
    QString generateJsonData(QVariant (*valueConverter)(int col, const QString& cellData) = nullptr);

    void showPrintPreview();

    void printTable(QPrinter* printer = nullptr);

    void appendRow(const QStringList& rowData);

    void deleteRow(int row);
    void clearTable();

    void appendRows(const QVector<QStringList>& rowsData);

    auto getAllTableData() const;

    QList<QList<QString>> getSelectedRows() const;
    std::optional<QStringList> getCurrentRow() const;

    void selectRowRange(int startRow, int endRow);

   protected:
    void keyPressEvent(QKeyEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void contextMenuEvent(QContextMenuEvent* event) override;

   signals:
    void tableSelectionChanged(int row, int column, const QStringList& rowData);
    void rowUpdated(int row, int column, const QStringList& rowData);

   public slots:
    void filterTable(const QString& query,
                     const QRegularExpression::PatternOption caseSensitivity = QRegularExpression::CaseInsensitiveOption,
                     int column = -1);

   private slots:
    void handleSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

    void handleDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight,
                           const QVector<int>& roles = QVector<int>());

   private:
    std::function<void(int, int, const QStringList&)> doubleClickHandler;
    void setRowData(int row, const QStringList& rowData);

    bool contextMenuEnabled;

    // Initialize the table model
    CustomTableModel* tableModel;

    // Initialize QSortFilterProxy table model to filter the table.
    QSortFilterProxyModel* proxyModel;

    // Table Headers
    // e.g ["ID", "First Name", "Created At"]
    QStringList headers;

    // e.g ["id", "first_name", "created_at"] used in generating json & csv data
    QStringList fieldNames;

    // Vertical Headers
    QStringList verticalHeaders;

    // use fieldNames in generating csv and json
    bool useFields() const;

    // Set background to a cell
    void setCellBackground(int row, int column, const QColor& color);

    // Set row background
    void setRowBackground(int row, const QColor& color);

    // Set column background
    void setColumnBackground(int column, const QColor& color);
};

#endif  // TABLE_WIDGET_H
