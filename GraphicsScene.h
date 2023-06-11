#ifndef GRAPHICSSCENE_H
#define GRAPHICSSCENE_H

#include <QBrush>
#include <QFileDialog>
#include <QGraphicsPathItem>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsView>
#include <QKeyEvent>
#include <QMessageBox>
#include <QMimeData>
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QQueue>
#include <QWheelEvent>

class GraphicsScene : public QGraphicsScene {
 public:
  GraphicsScene(QObject* parent = nullptr) : QGraphicsScene(parent) {
    currentPen = QPen(Qt::black);
    currentBrush = QBrush(Qt::transparent);

    isDrawing = false;
    currentPathItem = nullptr;
  }

 protected:
  void mousePressEvent(QGraphicsSceneMouseEvent* event) override {
    if (event->button() == Qt::LeftButton) {
      // Start drawing freehand path
      currentPath = new QPainterPath();
      currentPath->moveTo(event->scenePos());
      isDrawing = true;

      // Add the path item to the scene
      currentPathItem = addPath(*currentPath, currentPen);
      currentPathItem->setBrush(Qt::NoBrush);  // Disable fill
    }

    QGraphicsScene::mousePressEvent(event);
  }

  void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override {
    if (isDrawing) {
      // Continue drawing freehand path or straight line
      if (event->modifiers() & Qt::ShiftModifier) {
        // Shift key is pressed, draw a straight line
        const QPointF& startPoint = currentPath->currentPosition();
        const QPointF& endPoint = event->scenePos();

        // Calculate the straight line
        QPainterPath path;
        path.moveTo(startPoint);
        path.lineTo(endPoint);
        currentPathItem->setPath(path);
      } else {
        // Shift key is not pressed, continue drawing freehand path
        currentPath->lineTo(event->scenePos());
        currentPathItem->setPath(*currentPath);
      }
      update();
    }

    QGraphicsScene::mouseMoveEvent(event);
  }

  void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override {
    if (isDrawing && event->button() == Qt::LeftButton) {
      // Finish drawing freehand path
      isDrawing = false;

      // Enqueue the path item to the undo list
      undoList.enqueue(currentPathItem);

      // Clear the current path
      delete currentPath;
      currentPath = nullptr;
    }

    QGraphicsScene::mouseReleaseEvent(event);
  }

  void dragEnterEvent(QGraphicsSceneDragDropEvent* event) override {
    if (event->mimeData()->hasUrls()) {
      event->acceptProposedAction();
    }
  }

  void dropEvent(QGraphicsSceneDragDropEvent* event) override {
    if (event->mimeData()->hasUrls()) {
      QList<QUrl> urls = event->mimeData()->urls();
      if (urls.length() > 0) {
        QString filePath = urls[0].toLocalFile();
        importFromFile(filePath);
      }
    }
  }

  void wheelEvent(QGraphicsSceneWheelEvent* event) override {
    if (event->modifiers() == Qt::ControlModifier) {
      // Zoom in or out
      int zoomFactor = event->delta() > 0 ? 2 : -2;

      // Get the cursor position in the scene coordinates
      QPointF cursorPos = event->scenePos();

      // Get the associated view
      QGraphicsView* view = views().at(0);
      if (view) {
        qreal scaleFactor = qPow(2, zoomFactor / 100.0);
        view->scale(scaleFactor, scaleFactor);

        // Calculate the new cursor position after scaling
        QPointF newCursorPos = cursorPos * scaleFactor;

        // Adjust the view's scroll position to keep the cursor at the same
        // position
        QPointF scrollOffset = newCursorPos - event->scenePos();
        setSceneRect(sceneRect().translated(-scrollOffset));
      }
    }

    // Call the base class implementation to handle other wheel events
    QGraphicsScene::wheelEvent(event);
  }

  void keyPressEvent(QKeyEvent* event) override {
    if (event->modifiers() == Qt::ControlModifier) {
      if (event->key() == Qt::Key_S) {
        QString filePath = QFileDialog::getSaveFileName(
            nullptr, "Save Image", QString(), "PNG Image (*.png)");
        if (!filePath.isEmpty()) {
          saveToFile(filePath);
        }
      } else if (event->key() == Qt::Key_O) {
        QString filePath = QFileDialog::getOpenFileName(
            nullptr, "Import Image", QString(), "PNG Image (*.png)");

        if (!filePath.isEmpty()) {
          importFromFile(filePath);
        }
      } else if (event->key() == Qt::Key_Z) {
        undoLastPath();
      } else if (event->key() == Qt::Key_Y) {
        redoLastPath();
      }
    }
    QGraphicsScene::keyPressEvent(event);
  }

 public slots:
  void setPen(const QPen& pen) { currentPen = pen; }
  void setPenColor(const QColor& color) { currentPen.setColor(color); }
  void setPenWidth(int width) {
    currentPen.setWidth(width);
    updateCurrentPathPen();
  }

  void setPenStyle(Qt::PenStyle style) {
    currentPen.setStyle(style);
    updateCurrentPathPen();
  }

  void setPenCapStyle(Qt::PenCapStyle capStyle) {
    currentPen.setCapStyle(capStyle);
    updateCurrentPathPen();
  }

  void setPenJoinStyle(Qt::PenJoinStyle joinStyle) {
    currentPen.setJoinStyle(joinStyle);
    updateCurrentPathPen();
  }

  void setBrushColor(const QColor& color) { currentBrush.setColor(color); }

  void setBrushStyle(Qt::BrushStyle style) { currentBrush.setStyle(style); }

  void undoLastPath() {
    if (!undoList.isEmpty()) {
      // Dequeue the path item from the undo list
      QGraphicsPathItem* item = undoList.takeLast();

      // Add the path item to the redo list
      redoList.append(item);

      // Remove the path item from the scene
      removeItem(item);
    }
  }

  void redoLastPath() {
    if (!redoList.isEmpty()) {
      // Take the path item from the redo list
      QGraphicsPathItem* item = redoList.takeLast();

      // Add the path item back to the scene
      addItem(item);

      // Enqueue the path item to the undo list
      undoList.enqueue(item);
    }
  }

  void importFromFile(const QString& filePath) {
    QImage image(filePath);
    if (image.isNull()) {
      QMessageBox::critical(nullptr, "Error", "Failed to load image.");
      return;
    }

    clear();
    QGraphicsPixmapItem* pixmapItem = addPixmap(QPixmap::fromImage(image));
    pixmapItem->setPos(
        sceneRect().topLeft());  // Set the position of the pixmap to the
                                 // top-left corner of the scene
  }

  void saveToFile(const QString& filePath) {
    QImage image(sceneRect().size().toSize(), QImage::Format_ARGB32);
    image.fill(Qt::transparent);
    QPainter painter(&image);
    render(&painter);
    image.save(filePath);
  }

  void drawTriangle(const QPointF& position, qreal size,
                    const QPen& pen = QPen(Qt::black),
                    const QBrush& brush = Qt::NoBrush) {
    // Create a QGraphicsPolygonItem and add it to the scene
    QGraphicsPolygonItem* triangleItem = new QGraphicsPolygonItem();
    triangleItem->setPos(position);

    // Define the triangle's points
    QPolygonF trianglePolygon;
    trianglePolygon << QPointF(0, 0) << QPointF(size, 0)
                    << QPointF(size / 2, size);

    // Set the triangle's polygon and properties
    triangleItem->setPolygon(trianglePolygon);
    triangleItem->setPen(pen);
    triangleItem->setBrush(brush);

    addItem(triangleItem);  // Add the triangle item to the scene
  }

  void drawTriangle(const QPointF& point1, const QPointF& point2,
                    const QPointF& point3, const QPen& pen = QPen(Qt::black),
                    const QBrush& brush = Qt::NoBrush) {
    // Create a QPainterPath and move to the first point
    QPainterPath path;
    path.moveTo(point1);

    // Draw lines to connect the three points
    path.lineTo(point2);
    path.lineTo(point3);
    path.lineTo(point1);

    // Create a QGraphicsPathItem and set the path
    QGraphicsPathItem* pathItem = new QGraphicsPathItem();
    pathItem->setPath(path);

    pathItem->setBrush(brush);
    pathItem->setPen(pen);

    // Add the path item to the scene
    addItem(pathItem);
  }

  void createSquare(const QPointF& position, qreal size,
                    const QPen& pen = QPen(Qt::black),
                    const QBrush& brush = Qt::NoBrush) {
    // Create a QGraphicsRectItem and add it to the scene
    QGraphicsRectItem* squareItem = new QGraphicsRectItem();
    squareItem->setPos(position);

    // Set the square's geometry and properties
    squareItem->setRect(0, 0, size, size);

    squareItem->setBrush(brush);
    squareItem->setPen(pen);

    addItem(squareItem);  // Add the square item to the scene
  }

  void createRectangle(const QPointF& position, qreal width, qreal height,
                       const QPen& pen = QPen(Qt::black),
                       const QBrush& brush = Qt::NoBrush) {
    // Create a QGraphicsRectItem and add it to the scene
    QGraphicsRectItem* rectangleItem = new QGraphicsRectItem();
    rectangleItem->setPos(position);

    // Set the rectangle's geometry and properties
    rectangleItem->setRect(0, 0, width, height);
    rectangleItem->setBrush(brush);
    rectangleItem->setPen(pen);

    addItem(rectangleItem);  // Add the rectangle item to the scene
  }

  void createCircle(const QPointF& position, qreal size,
                    const QPen& pen = QPen(Qt::black),
                    const QBrush& brush = Qt::NoBrush) {
    QGraphicsEllipseItem* circleItem = new QGraphicsEllipseItem();
    circleItem->setPos(position);

    // Set the circle's geometry and properties
    circleItem->setRect(0, 0, size, size);
    circleItem->setBrush(brush);
    circleItem->setPen(pen);

    addItem(circleItem);  // Add the circle item to the scene
  }

  void createSphere(const QPointF& position, const qreal size,
                    const QPen& pen = QPen(Qt::black),
                    const QBrush& brush = Qt::NoBrush) {
    // Create a QGraphicsEllipseItem and add it to the scene
    QGraphicsEllipseItem* sphereItem = new QGraphicsEllipseItem();
    sphereItem->setPos(position);

    // Set the sphere's geometry and properties
    sphereItem->setRect(0, 0, size, size);
    sphereItem->setBrush(brush);
    sphereItem->setPen(pen);
    addItem(sphereItem);
  }

  void drawQuadraticCurve(const QPointF& startPoint,
                          const QPointF& controlPoint, const QPointF& endPoint,
                          const QPen& pen = QPen(Qt::black),
                          const QBrush& brush = Qt::NoBrush) {
    // Create a QPainterPath and move to the start point
    QPainterPath path;
    path.moveTo(startPoint);

    // Draw the smooth curve
    path.quadTo(controlPoint, endPoint);

    // Create a QGraphicsPathItem and set the path
    QGraphicsPathItem* pathItem = new QGraphicsPathItem();
    pathItem->setPath(path);

    // Set the pen and brush properties for the curve
    pathItem->setBrush(brush);
    pathItem->setPen(pen);

    // Add the path item to the scene
    addItem(pathItem);
  }

  void drawBezierCurve(const QPointF& startPoint, const QPointF& endPoint,
                       const QPointF& controlPoint1,
                       const QPointF& controlPoint2,
                       const QPen& pen = QPen(Qt::black),
                       const QBrush& brush = Qt::NoBrush) {
    // Create a QPainterPath and move to the start point
    QPainterPath path;
    path.moveTo(startPoint);

    // Draw the Bézier curve
    path.cubicTo(controlPoint1, controlPoint2, endPoint);

    // Create a QGraphicsPathItem and set the path
    QGraphicsPathItem* pathItem = new QGraphicsPathItem();
    pathItem->setPath(path);
    pathItem->setBrush(brush);
    pathItem->setPen(pen);

    // Add the path item to the scene
    addItem(pathItem);
  }

  void addText(const QPointF& startPoint, const QString& text,
               const QFont& font = QFont()) {
    QGraphicsTextItem* textItem = new QGraphicsTextItem(text);
    textItem->setFont(font);
    textItem->setPos(startPoint);
    addItem(textItem);
  }

  void addHTMLText(const QPointF& startPoint, const QString& html,
                   const QFont& font = QFont()) {
    QGraphicsTextItem* textItem = new QGraphicsTextItem();
    textItem->setHtml(html);
    textItem->setFont(font);
    textItem->setPos(startPoint);
    addItem(textItem);
  }

 private:
  QPen currentPen;
  QBrush currentBrush;
  QPainterPath* currentPath;
  bool isDrawing;
  QGraphicsPathItem* currentPathItem;

  QQueue<QGraphicsPathItem*> undoList;
  QList<QGraphicsPathItem*> redoList;

  void updateCurrentPathPen() {
    if (currentPathItem) {
      currentPathItem->setPen(currentPen);
    }
  }
};

#endif  // GRAPHICSSCENE_H
