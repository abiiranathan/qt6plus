#include <QApplication>
#include <QLabel>
#include <QMainWindow>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

#include "GraphicsScene.h"
#include "splitter.h"

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);

  // Create a main window
  QWidget mainWindow;
  QVBoxLayout layout(&mainWindow);

  // Create widgets
  QLabel *label1 = new QLabel("Widget 1");
  QLabel *label2 = new QLabel("Widget 2");
  QPushButton *button = new QPushButton("Widget 3");

  // Create the custom splitter
  Splitter splitter;
  splitter.setOrientation(Qt::Vertical);

  // Add widgets to the splitter
  splitter.addWidget(label1, 1);
  splitter.addWidget(button, 2);
  splitter.addWidget(label2, 3);

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

  //  scene.setBrushStyle(Qt::DiagCrossPattern);  // Set brush style
  //  scene.saveToFile("image.png");  // Save the scene as an image file

  // Show the view
  //  view.show();

  splitter.addWidget(&view);
  splitter.setWidgetStretchFactors(1, 2, 3);
  mainWindow.show();

  return app.exec();
}
