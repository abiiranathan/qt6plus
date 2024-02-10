#include "TableWidget.h"

// =============== HtmlPreviewWidget oveerides paintEvent =========
HtmlPreviewWidget::HtmlPreviewWidget(QString html)
    : htmlContent(html) {
    updatePreview();
}

void HtmlPreviewWidget::paintEvent(QPaintEvent* event) {
    QPrintPreviewWidget::paintEvent(event);

    QPainter painter(this);
    QTextDocument doc;

    doc.setHtml(htmlContent);
    doc.setDefaultTextOption(QTextOption(Qt::AlignLeft | Qt::AlignTop));
    doc.setPageSize(size());

    painter.save();
    painter.translate(rect().topLeft());

    // Draw the document's contents
    doc.drawContents(&painter);
    painter.restore();
}

// =================== CustomTableModel overrides flags ===========================
CustomTableModel::CustomTableModel(const QList<int>& editableColumns, const QList<int>& disabledColumns,
                                   QObject* parent)
    : QStandardItemModel(parent),
      editableColumns(editableColumns),
      disabledColumns(disabledColumns) {}

Qt::ItemFlags CustomTableModel::flags(const QModelIndex& index) const {
    if (!index.isValid())
        return Qt::NoItemFlags;

    if (editableColumns.contains(index.column()))
        return Qt::ItemFlags(Qt::ItemIsSelectable | Qt::ItemIsEditable |
                             Qt::ItemIsEnabled);  // Enable editing for the specified columns

    // Disable editing for the specified columns
    if (disabledColumns.contains(index.column()))
        return Qt::ItemFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    // Default behavior for other columns
    return QStandardItemModel::flags(index);
}

// ============== TableWidget implementation ========================

/**
     * Constructor for the TableWidget.
     */
TableWidget::TableWidget(QWidget* parent, QList<int> editableColumns,
                         QList<int> disabledColumns)
    : QTableView(parent) {
    tableModel = new CustomTableModel(editableColumns, disabledColumns, this);

    proxyModel = new QSortFilterProxyModel(this);
    proxyModel->setSourceModel(tableModel);
    proxyModel->setFilterKeyColumn(-1);
    setModel(proxyModel);

    // Set default properties
    setSelectionMode(QAbstractItemView::SingleSelection);
    setSelectionBehavior(QAbstractItemView::SelectRows);

    // Connect item selection signal
    connect(selectionModel(), &QItemSelectionModel::selectionChanged, this,
            &TableWidget::handleSelectionChanged);

    connect(model(), &QAbstractItemModel::dataChanged, this, &TableWidget::handleDataChanged,
            Qt::QueuedConnection);

    contextMenuEnabled = true;
    fit();
}

// Destructor
TableWidget::~TableWidget() {
    tableModel->deleteLater();
    proxyModel->deleteLater();
}

// Returns the number of rows
int TableWidget::rowCount() const {
    return model()->rowCount();
}

// Returns the number of columns
int TableWidget::columnCount() const {
    return model()->columnCount();
}

// Resize headers to fit content
void TableWidget::fit() {
    // Set horizontal header resize mode to stretch for each column
    horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
}

// Resize headers, stretching them to fill parent
void TableWidget::stretch() {
    // Set horizontal header resize mode to stretch for each column
    horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

// Set Interactive resizable headers
void TableWidget::interactive() {
    horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
}

// Sets the column to filter on. Default -1 (all columns)
void TableWidget::setFilterKeyColumn(int column) {
    proxyModel->setFilterKeyColumn(column);
}

void TableWidget::setContextMenuEnabled(bool enabled) {
    contextMenuEnabled = enabled;
}

// Set table horizontal headers.
// fieldNames should be equal in length to horizontalHeaders(otherwise won't be used)
// fieldNames are used in generating JSON and CSV data.
void TableWidget::setHorizontalHeaders(const QStringList& horizontalHeaders,
                                       const QStringList& fields) {
    // set headers
    headers = horizontalHeaders;

    // Set the horizontal Headers
    tableModel->setHorizontalHeaderLabels(horizontalHeaders);
    // Adjust header sizes to fit the contents
    horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // Set the field names
    fieldNames = fields;
}

// fieldNames should be equal in length to horizontalHeaders(otherwise won't be used)
// fieldNames are used in generating JSON and CSV data.
void TableWidget::setFieldNames(const QStringList& fieldNames_) {
    fieldNames = fieldNames_;
}

// Sets vertical headers for the table.
void TableWidget::setVerticalHeaders(const QStringList& headers) {
    verticalHeaders = headers;
    if (!verticalHeaders.isEmpty())
        tableModel->setVerticalHeaderLabels(verticalHeaders);

    // Adjust header sizes to fit the contents
    if (!verticalHeaders.isEmpty())
        verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
}

void TableWidget::resetHeaders() {
    tableModel->setHorizontalHeaderLabels(headers);
    auto sectionResizeMode = horizontalHeader()->sectionResizeMode(0);
    horizontalHeader()->setSectionResizeMode(sectionResizeMode);

    if (!verticalHeaders.isEmpty()) {
        tableModel->setVerticalHeaderLabels(verticalHeaders);
        verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    }
}

/**
     * Populates the table with data.
     */
void TableWidget::setData(const QVector<QStringList>& data) {
    tableModel->clear();
    tableModel->setRowCount(data.size());
    tableModel->setColumnCount(0);

    // Update the column count
    if (!data.isEmpty())
        tableModel->setColumnCount(data[0].size());

    // Update the headers because the table was cleared
    resetHeaders();

    for (int row = 0; row < data.size(); ++row) {
        const QStringList& rowDataList = data[row];
        for (int column = 0; column < rowDataList.size(); ++column) {
            QString text = rowDataList[column];

            if (text == "null" || text == "undefined") {
                text = "";
            }
            auto item = new QStandardItem(text);
            tableModel->setItem(row, column, item);
        }
    }
}

// Sets the signals and slots for double click on table. Calls handler with data for
// the double-clicked row.
void TableWidget::setDoubleClickHandler(std::function<void(int row, int col, const QStringList& data)> handler) {
    doubleClickHandler = std::move(handler);
}

// Generates an html table and writes it to a QString that is returned.
QString TableWidget::generateHtmlTable() {
    QString html;

    int rowCount = model()->rowCount();
    int columnCount = model()->columnCount();

    // Start the HTML table with CSS styles
    html += "<table style='border-collapse: collapse; width: 100%;'>";

    // Generate table headers with CSS styles
    html += "<thead><tr>";
    for (int col = 0; col < columnCount; ++col) {
        html += "<th style='border: 1px solid #ddd; padding: 8px; background-color: #f2f2f2;'>";
        html += model()->headerData(col, Qt::Horizontal).toString();
        html += "</th>";
    }
    html += "</tr></thead>";

    // Generate table rows with CSS styles
    html += "<tbody>";
    for (int row = 0; row < rowCount; ++row) {
        html += "<tr>";
        for (int col = 0; col < columnCount; ++col) {
            html += "<td style='border: 1px solid #ddd; padding: 8px;'>";
            html += model()->data(model()->index(row, col)).toString();
            html += "</td>";
        }
        html += "</tr>";
    }
    html += "</tbody>";

    // End the HTML table
    html += "</table>";

    return html;
}

// Generates and returns QString containing CSV for the table data.
QString TableWidget::generateCsvData() {
    QString csv;

    int rowCount = model()->rowCount();
    int columnCount = model()->columnCount();

    if (useFields()) {
        // Generate CSV headers
        for (int col = 0; col < columnCount; ++col) {
            if (col > 0) {
                csv += ",";
            }
            csv += "\"" + fieldNames[col] + "\"";
        }
        csv += "\n";
    }

    // Generate CSV data rows
    for (int row = 0; row < rowCount; ++row) {
        for (int col = 0; col < columnCount; ++col) {
            if (col > 0) {
                csv += ",";
            }
            QString value = model()->data(model()->index(row, col)).toString();
            // Quote the value if it contains a comma
            if (value.contains(',')) {
                value = "\"" + value + "\"";
            }
            csv += value;
        }
        csv += "\n";
    }

    return csv;
}

// Generates and returns QString containing JSON for the table data.
// The valueConverter is required if you want to convert cell data to other types from QString.
QString TableWidget::generateJsonData(QVariant (*valueConverter)(int col, const QString& cellData)) {
    QJsonArray rowsArray;

    int rowCount = model()->rowCount();
    int columnCount = model()->columnCount();

    auto useCustomFields = useFields();

    // Generate JSON data rows
    for (int row = 0; row < rowCount; ++row) {
        QJsonObject rowObject;
        for (int col = 0; col < columnCount; ++col) {
            QString columnName = model()->headerData(col, Qt::Horizontal).toString();

            if (useCustomFields) {
                columnName = fieldNames[col];
            }

            QVariant cellValue = model()->data(model()->index(row, col));
            if (valueConverter) {
                // Convert value using the user-provided callback function
                cellValue = valueConverter(col, cellValue.toString());
            }
            rowObject[columnName] = QJsonValue::fromVariant(cellValue);
        }
        rowsArray.append(rowObject);
    }

    QJsonDocument jsonDoc(rowsArray);
    return jsonDoc.toJson();
}

void TableWidget::showPrintPreview() {
    // Generate the HTML table
    QString htmlTable = generateHtmlTable();

    QString html = "<div style=\"text-align: center; margin-bottom:16px;\">";

    if (!title.isEmpty()) {
        html += "<h1 style=\"font-size: 18px; margin-bottom: 4px;\">" + title + "</h1>";
    }

    if (!logo.isEmpty()) {
        html += QString("<div style=\"display: inline-block;\"><img src=\"%1\" width=\"64\" height=\"64\" /></div>").arg(logo.toString());
    }

    html += "<br/> </div>";
    html += htmlTable;

    // Create a QTextDocument to set the HTML content
    QTextDocument document;
    document.setHtml(html);

    // Create a custom HTML preview widget
    HtmlPreviewWidget previewWidget(html);

    // Create a QPrintPreviewDialog and set the custom preview widget
    QPrinter printer(QPrinter::HighResolution);
    QPrintPreviewDialog previewDialog(&printer);
    previewDialog.setMinimumSize(800, 600);
    previewDialog.setWindowTitle("Print Preview");

    // Set the custom preview widget as the central widget of the print preview dialog
    previewDialog.setWindowFlags(previewDialog.windowFlags() &
                                 ~Qt::WindowContextHelpButtonHint);

    QVBoxLayout* layout = new QVBoxLayout();
    layout->addWidget(&previewWidget);

    // Connect the paintRequested signal to handle the printing
    connect(&previewDialog, &QPrintPreviewDialog::paintRequested, this,
            [this, &document](QPrinter* printer) { document.print(printer); });

    // Show the print preview dialog
    previewDialog.exec();
}

void TableWidget::printTable(QPrinter* printer) {
    // Create a QTextBrowser to display the HTML content
    QTextBrowser textBrowser;

    // Generate the HTML table from the QTableView
    QString htmlTable = generateHtmlTable();

    QString html = "<div style=\"text-align: center; margin-bottom:16px;\">";

    if (!title.isEmpty()) {
        html += "<h1 style=\"font-size: 18px; margin-bottom: 4px;\">" + title + "</h1>";
    }

    if (!logo.isEmpty()) {
        html += QString("<div style=\"display: inline-block;\"><img src=\"%1\" width=\"64\" height=\"64\" /></div>").arg(logo.toString());
    }

    html += "<br/> </div>";
    html += htmlTable;

    // Set the HTML content to the QTextBrowser
    textBrowser.setHtml(html);

    if (printer == nullptr) {
        printer = new QPrinter(QPrinter::HighResolution);
    }

    // Print the QTextBrowser content
    QPrintDialog printDialog(printer);
    if (printDialog.exec() == QDialog::Accepted) {
        textBrowser.print(printer);
    }
}

void TableWidget::appendRow(const QStringList& rowData) {
    int row = tableModel->rowCount();
    tableModel->setRowCount(row + 1);
    setRowData(row, rowData);
}

void TableWidget::deleteRow(int row) {
    if (row >= 0 && row < tableModel->rowCount()) {
        tableModel->removeRow(row);
    }
}

void TableWidget::clearTable() {
    tableModel->clear();
}

void TableWidget::appendRows(const QVector<QStringList>& rowsData) {
    int currentRowCount = tableModel->rowCount();
    int rowsToAdd = rowsData.size();
    int newRowCount = currentRowCount + rowsToAdd;

    tableModel->setRowCount(newRowCount);

    for (int row = 0; row < rowsToAdd; ++row) {
        const QStringList& rowData = rowsData[row];
        setRowData(currentRowCount + row, rowData);
    }
}

auto TableWidget::getAllTableData() const {
    QList<QList<QVariant>> tableData;

    if (tableModel) {
        for (int row = 0; row < tableModel->rowCount(); ++row) {
            QList<QVariant> rowData;

            for (int col = 0; col < tableModel->columnCount(); ++col) {
                QModelIndex index = tableModel->index(row, col);
                QVariant data = tableModel->data(index);
                rowData.append(data);
            }
            tableData.append(rowData);
        }
    }
    return tableData;
}

QList<QList<QString>> TableWidget::getSelectedRows() const {
    QList<QList<QString>> selectedRowsData;

    QModelIndexList selectedIndexes = selectionModel()->selectedRows();
    for (const QModelIndex& index : selectedIndexes) {
        QList<QString> rowData;

        for (int col = 0; col < tableModel->columnCount(); ++col) {
            QModelIndex dataIndex = tableModel->index(index.row(), col);
            QString data = tableModel->data(dataIndex).toString();
            rowData.append(data);
        }
        selectedRowsData.append(rowData);
    }

    return selectedRowsData;
}

std::optional<QStringList> TableWidget::getCurrentRow() const {
    std::optional<QStringList> rowData;

    QModelIndex index = currentIndex();
    if (index.isValid()) {
        rowData.emplace();

        for (int col = 0; col < tableModel->columnCount(); ++col) {
            QModelIndex dataIndex = tableModel->index(index.row(), col);
            QVariant data = tableModel->data(dataIndex);
            rowData->append(data.toString());
        }
    }
    return rowData;
}

void TableWidget::selectRowRange(int startRow, int endRow) {
    // Create the selection model and get the model index for the desired row
    QItemSelectionModel* selModel = selectionModel();
    // Get the model indices for the start and end rows
    QModelIndex startModelIndex = model()->index(startRow, 0);
    QModelIndex endModelIndex = model()->index(endRow, 0);

    // Create a selection range for the row range
    QItemSelection selection(startModelIndex, endModelIndex);

    // Clear the existing selection and select the desired row range
    selModel->clearSelection();
    selModel->select(selection, QItemSelectionModel::Select);
}

void TableWidget::keyPressEvent(QKeyEvent* event) {
    // Check if Ctrl+Shift+P is pressed
    if (event->modifiers() == (Qt::ControlModifier | Qt::ShiftModifier) &&
        event->key() == Qt::Key_P) {
        // Show print preview
        showPrintPreview();
        return;
    } else if (event->modifiers() == Qt::ControlModifier && event->key() == Qt::Key_P) {
        printTable();
        return;
    } else if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) {
        auto selected = getCurrentRow();
        if (doubleClickHandler && selected) {
            QModelIndex index = currentIndex();
            doubleClickHandler(index.row(), index.column(), selected.value());
        }
    }
    QTableView::keyPressEvent(event);
}

void TableWidget::mouseDoubleClickEvent(QMouseEvent* event) {
    if (doubleClickHandler) {
        QModelIndex index = indexAt(event->pos());
        if (index.isValid()) {
            int row = index.row();
            int column = index.column();

            QStringList rowData;
            for (int c = 0; c < tableModel->columnCount(); ++c) {
                auto cellItem = tableModel->item(row, c);
                if (cellItem)
                    rowData.append(cellItem->text());
            }
            doubleClickHandler(row, column, rowData);
        }
    }
    // Call base class implementation
    QTableView::mouseDoubleClickEvent(event);
}

void TableWidget::contextMenuEvent(QContextMenuEvent* event) {
    if (!contextMenuEnabled)
        return;

    QMenu contextMenu(this);

    // Add actions to the context menu
    QAction* copyAction = contextMenu.addAction("Copy");
    QAction* pasteAction = contextMenu.addAction("Paste");
    QAction* deleteAction = contextMenu.addAction("Remove");

    // Add more actions as needed

    // Show the context menu at the mouse position
    QPoint pos = event->globalPos();
    QAction* selectedItem = contextMenu.exec(mapToGlobal(pos));

    // Handle the selected action
    if (selectedItem == copyAction) {
        // Perform copy action
        std::optional<QStringList> rowData = getCurrentRow();
        if (rowData) {
            QApplication::clipboard()->setText(rowData->join("\t"));
        }
    } else if (selectedItem == pasteAction) {
        QString clipboardText = QApplication::clipboard()->text();
        if (clipboardText.isEmpty())
            return;

        QStringList items = clipboardText.split("\t");
        if (items.size() == model()->columnCount()) {
            appendRow(items);
        }
    } else if (selectedItem == deleteAction) {
        QModelIndex index = currentIndex();
        if (index.isValid()) {
            deleteRow(index.row());
        }
    }

    QTableView::contextMenuEvent(event);
}

void TableWidget::filterTable(const QString& query,
                              const QRegularExpression::PatternOption caseSensitivity,
                              int column) {

    if (query.isEmpty()) {
        proxyModel->invalidate();
        return;
    }

    // -1 is all columns
    if (column >= -1 && column < model()->columnCount()) {
        proxyModel->setFilterKeyColumn(column);
    }
    QRegularExpression regex(query, caseSensitivity);
    proxyModel->setFilterRegularExpression(regex);
}

void TableWidget::handleSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected) {
    Q_UNUSED(deselected);
    if (selected.isEmpty())
        return;

    int selectedRow = selected.indexes().first().row();
    int selectedCol = selected.indexes().first().column();

    QStringList rowData;
    for (int column = 0; column < tableModel->columnCount(); ++column) {
        auto item = tableModel->item(selectedRow, column);
        if (item)
            rowData.append(item->text());
    }
    emit tableSelectionChanged(selectedRow, selectedCol, rowData);
}

void TableWidget::handleDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight,
                                    const QVector<int>& roles) {
    Q_UNUSED(roles);

    // If no row selected, we are just setting the data
    if (selectionModel()->selectedIndexes().isEmpty())
        return;

    if (topLeft.row() != bottomRight.row()) {
        // Only handle single-row changes
        return;
    }

    QStringList rowData;
    const int row = topLeft.row();

    for (int column = 0; column < tableModel->columnCount(); ++column) {
        QModelIndex index = tableModel->index(row, column);
        QVariant data = tableModel->data(index);
        QString cellData = data.toString();
        rowData.append(cellData);
    }
    emit rowUpdated(row, topLeft.column(), rowData);
    return QTableView::dataChanged(topLeft, bottomRight, roles);
}

void TableWidget::setRowData(int row, const QStringList& rowData) {
    for (int column = 0; column < tableModel->columnCount(); ++column) {
        QStandardItem* item = new QStandardItem();
        QString text = rowData.value(column);

        // I hate nulls in a table
        if (text == "null" || text == "undefined") {
            text = "";
        }

        // Set the cell text
        item->setText(text);
        tableModel->setItem(row, column, item);
    }
}

// use fieldNames in generating csv and json
bool TableWidget::useFields() const {
    return (headers.size() == fieldNames.size()) && (fieldNames.size() == model()->columnCount());
}

void TableWidget::setCellBackground(int row, int column, const QColor& color) {
    QModelIndex index = model()->index(row, column);
    if (index.isValid()) {
        model()->setData(index, color, Qt::BackgroundRole);
    }
}

void TableWidget::setRowBackground(int row, const QColor& color) {
    for (int col = 0; col < columnCount(); ++col) {
        QModelIndex index = model()->index(row, col);
        if (index.isValid()) {
            model()->setData(index, color, Qt::BackgroundRole);
        }
    }
}

void TableWidget::setColumnBackground(int column, const QColor& color) {
    for (int row = 0; row < rowCount(); ++row) {
        QModelIndex index = model()->index(row, column);
        if (index.isValid()) {
            model()->setData(index, color, Qt::BackgroundRole);
        }
    }
}
