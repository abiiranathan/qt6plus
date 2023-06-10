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
    setSceneRect(QRectF(-500, -500, 1000, 1000));
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
    // Zoom in or out
    int zoomFactor = event->delta() > 0 ? 2 : -2;
    QList<QGraphicsView*> sceneViews = views();
    if (!sceneViews.isEmpty()) {
      QGraphicsView* view = sceneViews.at(0);
      view->scale(zoomFactor, zoomFactor);
    }
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
