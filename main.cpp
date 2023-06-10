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
  splitter.setOrientation(Qt::Horizontal);

  // Add widgets to the splitter
  splitter.addWidget(label1, 1);
  splitter.addWidget(button, 2);
  splitter.addWidget(label2, 3);

  // Set the stretch factors for the splitter widgets
  //  splitter.setWidgetStretchFactors(1, 2, 3);
  splitter.setWidgetSizes(200, 600, 200);

  // Add the splitter to the layout
  layout.addWidget(&splitter);

  // Set the layout for the main window
  mainWindow.setLayout(&layout);
  mainWindow.setWindowTitle("Splitter Example");

  // Test Graphics scene
  QGraphicsView view;
  GraphicsScene scene;

  view.setScene(&scene);
  view.setWindowTitle("GraphicsScene Test");

  // Set up the initial pen color and brush color
  scene.setBrushColor(Qt::transparent);
  scene.setBackgroundBrush(Qt::white);

  QPen pen(Qt::red, 2, Qt::SolidLine, Qt::FlatCap, Qt::BevelJoin);
  scene.setPen(pen);
  //  scene.setBrushStyle(Qt::DiagCrossPattern);  // Set brush style
  //  scene.saveToFile("image.png");  // Save the scene as an image file

  // Show the view
  view.show();

  //  mainWindow.show();

  return app.exec();
}
