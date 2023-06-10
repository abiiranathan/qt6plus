#ifndef SPLITTER_H
#define SPLITTER_H

#include <QSplitter>

#include "splitter_global.h"

class SPLITTER_EXPORT Splitter : public QSplitter {
 public:
  Splitter(QWidget *parent = nullptr) : QSplitter(parent) {}

  void setWidgetSizes(int first, int second) {
    QList<int> sizes;
    sizes.append(first);
    sizes.append(second);
    setSizes(sizes);
  }

  void setWidgetSizes(int first, int second, int third) {
    QList<int> sizes;
    sizes.append(first);
    sizes.append(second);
    sizes.append(third);
    setSizes(sizes);
  }

  void setWidgetStretchFactors(int first, int second) {
    QList<int> stretchFactors;
    stretchFactors.append(first);
    stretchFactors.append(second);
    setStretchFactors(stretchFactors);
  }

  void setWidgetStretchFactors(int first, int second, int third) {
    QList<int> stretchFactors;
    stretchFactors.append(first);
    stretchFactors.append(second);
    stretchFactors.append(third);
    setStretchFactors(stretchFactors);
  }

  void addWidget(QWidget *widget, int stretch = 0) {
    if (widget) {
      QSplitter::addWidget(widget);
      setStretchFactor(indexOf(widget), stretch);
    }
  }

  void insertWidget(int index, QWidget *widget, int stretch = 0) {
    if (widget) {
      QSplitter::insertWidget(index, widget);
      setStretchFactor(index, stretch);
    }
  }

  void setStretchFactors(const QVector<int> &stretchFactors) {
    int ncount = qMin(stretchFactors.size(), count());
    for (int i = 0; i < ncount; ++i) {
      setStretchFactor(i, stretchFactors[i]);
    }
  }
};

#endif  // SPLITTER_H
