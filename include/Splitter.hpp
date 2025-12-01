#ifndef SPLITTER_H
#define SPLITTER_H

#include <QSplitter>

class Splitter : public QSplitter {
   public:
    Splitter(QWidget* parent = nullptr);

    void setWidgetSizes(int first, int second);
    void setWidgetSizes(int first, int second, int third);
    void setWidgetStretchFactors(int first, int second);
    void setWidgetStretchFactors(int first, int second, int third);
    void addWidget(QWidget* widget, int stretch = 0);
    void insertWidget(int index, QWidget* widget, int stretch = 0);
    void setStretchFactors(const QVector<int>& stretchFactors);
};

#endif  // SPLITTER_H
