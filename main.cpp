#include <QApplication>
#include <QLabel>
#include <QMainWindow>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>
#include "BluetoothDevice.h"
#include "Delegates.h"
#include "GraphicsScene.h"
#include "Splitter.h"
#include "TableWidget.h"

void BTConnect() {
    BluetoothDevice bluetoothDevice;
    bluetoothDevice.discoverDevices();

    QObject::connect(&bluetoothDevice, &BluetoothDevice::deviceDiscovered,
                     [](const QBluetoothDeviceInfo& deviceInfo) {
                         // Handle discovered device
                     });

    QObject::connect(&bluetoothDevice, &BluetoothDevice::connected, [&]() {
        // Connected to the device
        bluetoothDevice.writeData("Hello, Bluetooth!");
    });

    QObject::connect(&bluetoothDevice, &BluetoothDevice::dataReceived,
                     [](const QByteArray& data) {
                         // Handle received data
                     });
}

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    // Create a main window
    QWidget mainWindow;
    QVBoxLayout layout(&mainWindow);

    // Create the custom splitter
    Splitter splitter;
    splitter.setOrientation(Qt::Vertical);

    // Create a table widget
    TableWidget* table = new TableWidget(nullptr, QList<int>(), QList<int>{0, 1});
    table->title = "RISTAL UNIVERSITY";
    table->logo = QUrl::fromLocalFile("/home/nabiizy/Downloads/logo-white.png");

    table->setHorizontalHeaders(QStringList{"ID", "Name", "DOB", "Sex", "CreatedAt", "Time"},
                                QStringList{"id", "name", "dob", "sex", "created_at", "time"});

    table->setItemDelegateForColumn(2, new DateDelegate());
    table->setItemDelegateForColumn(3, new ComboBoxDelegate(nullptr, QStringList{"Male", "Female"}));
    table->setItemDelegateForColumn(4, new DateTimeDelegate());
    table->setItemDelegateForColumn(5, new TimeDelegate());

    table->setDoubleClickHandler([](int row, int col, auto data) {
        qDebug() << data << "\n";
    });

    // Get the vertical header
    QHeaderView* verticalHeader = table->verticalHeader();

    // Hide the row numbers
    verticalHeader->setVisible(false);

    table->setData(
        QVector<QStringList>{{"1", "Abiira Nathan", "1989-05-18", "Male", "2023-06-07T06:30:13.075Z", "16:30:34"},
                             {"2", "Kwikiriza Dan", "2005-06-12", "Female", "null", "00:30:00"}});

    // Add widgets to the splitter
    splitter.addWidget(table, 1);

    // Set the stretch factors for the splitter widgets
    //  splitter.setWidgetSizes(200, 600, 200);

    // Add the splitter to the layout
    layout.addWidget(&splitter);

    // Set the layout for the main window
    mainWindow.setLayout(&layout);
    mainWindow.setWindowTitle("Splitter Example");

    // Test Graphics scene
    QGraphicsView view;
    GraphicsScene scene;
    scene.setSceneRect(0, 0, 400, 400);

    QPen pen(Qt::red, 2, Qt::SolidLine, Qt::FlatCap, Qt::BevelJoin);
    scene.setPen(pen);

    view.setScene(&scene);
    // Enable anti-aliasing for the QGraphicsView
    view.setRenderHint(QPainter::Antialiasing);

    view.setWindowTitle("GraphicsScene Test");

    // Set up the initial pen color and brush color
    scene.setBrushColor(Qt::transparent);
    scene.setBackgroundBrush(Qt::white);

    scene.createCircle(QPointF(10, 10), 100, pen, Qt::blue);

    scene.addLine(QLineF({5, 5}, {200, 100}));

    scene.drawBezierCurve(QPointF(100, 100), QPointF(300, 100), QPointF(150, 50),
                          QPointF(250, 150));

    scene.drawQuadraticCurve(QPointF(100, 100), QPointF(300, 100),
                             QPointF(200, 0));

    scene.createRectangle(QPointF(100, 200), 200, 60, QPen(Qt::cyan),
                          QBrush(Qt::DiagCrossPattern));

    scene.addText(QPoint(10, 200), "HELLO ABIIRA NATHAN",
                  QFont("Arial", 18, 600));

    scene.drawTriangle(QPointF(20, 50), QPointF(200, 75), QPointF(300, 300),
                       QPen(Qt::cyan), QBrush(Qt::blue));

    splitter.addWidget(&view);
    splitter.setWidgetStretchFactors(1, 1);

    BTConnect();

    mainWindow.show();

    return app.exec();
}
