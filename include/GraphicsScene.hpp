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

#include "qt6plus_export.hpp"

class QT6PLUS_EXPORT GraphicsScene : public QGraphicsScene {
   public:
    GraphicsScene(QObject* parent = nullptr);

   protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
    void dragEnterEvent(QGraphicsSceneDragDropEvent* event) override;
    void dropEvent(QGraphicsSceneDragDropEvent* event) override;
    void wheelEvent(QGraphicsSceneWheelEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

   public slots:
    void setPen(const QPen& pen);
    void setPenColor(const QColor& color);
    void setPenWidth(int width);
    void setPenStyle(Qt::PenStyle style);
    void setPenCapStyle(Qt::PenCapStyle capStyle);
    void setPenJoinStyle(Qt::PenJoinStyle joinStyle);
    void setBrushColor(const QColor& color);
    void setBrushStyle(Qt::BrushStyle style);

    void undoLastPath();
    void redoLastPath();

    void importFromFile(const QString& filePath);
    void saveToFile(const QString& filePath);

    void drawTriangle(const QPointF& position, qreal size, const QPen& pen = QPen(Qt::black),
                      const QBrush& brush = Qt::NoBrush);

    void drawTriangle(const QPointF& point1, const QPointF& point2, const QPointF& point3,
                      const QPen& pen = QPen(Qt::black), const QBrush& brush = Qt::NoBrush);

    void createSquare(const QPointF& position, qreal size, const QPen& pen = QPen(Qt::black),
                      const QBrush& brush = Qt::NoBrush);

    void createRectangle(const QPointF& position, qreal width, qreal height,
                         const QPen& pen = QPen(Qt::black), const QBrush& brush = Qt::NoBrush);

    void createCircle(const QPointF& position, qreal size, const QPen& pen = QPen(Qt::black),
                      const QBrush& brush = Qt::NoBrush);

    void createSphere(const QPointF& position, qreal size, const QPen& pen = QPen(Qt::black),
                      const QBrush& brush = Qt::NoBrush);

    void drawQuadraticCurve(const QPointF& startPoint, const QPointF& controlPoint,
                            const QPointF& endPoint, const QPen& pen = QPen(Qt::black),
                            const QBrush& brush = Qt::NoBrush);

    void drawBezierCurve(const QPointF& startPoint, const QPointF& endPoint,
                         const QPointF& controlPoint1, const QPointF& controlPoint2,
                         const QPen& pen = QPen(Qt::black), const QBrush& brush = Qt::NoBrush);

    void addText(const QPointF& startPoint, const QString& text, const QFont& font = QFont());

    void addHTMLText(const QPointF& startPoint, const QString& html, const QFont& font = QFont());

   private:
    QPen currentPen;
    QBrush currentBrush;
    QPainterPath* currentPath;
    bool isDrawing;
    QGraphicsPathItem* currentPathItem;

    QQueue<QGraphicsPathItem*> undoList;
    QList<QGraphicsPathItem*> redoList;

    void updateCurrentPathPen();
};

#endif  // GRAPHICSSCENE_H
