#include "Splitter.h"

Splitter::Splitter(QWidget* parent)
    : QSplitter(parent) {}

void Splitter::setWidgetSizes(int first, int second) {
    QList<int> sizes;
    sizes.append(first);
    sizes.append(second);
    setSizes(sizes);
}

void Splitter::setWidgetSizes(int first, int second, int third) {
    QList<int> sizes;
    sizes.append(first);
    sizes.append(second);
    sizes.append(third);
    setSizes(sizes);
}

void Splitter::setWidgetStretchFactors(int first, int second) {
    QList<int> stretchFactors;
    stretchFactors.append(first);
    stretchFactors.append(second);
    setStretchFactors(stretchFactors);
}

void Splitter::setWidgetStretchFactors(int first, int second, int third) {
    QList<int> stretchFactors;
    stretchFactors.append(first);
    stretchFactors.append(second);
    stretchFactors.append(third);
    setStretchFactors(stretchFactors);
}

void Splitter::addWidget(QWidget* widget, int stretch) {
    if (widget) {
        QSplitter::addWidget(widget);
        setStretchFactor(indexOf(widget), stretch);
    }
}

void Splitter::insertWidget(int index, QWidget* widget, int stretch) {
    if (widget) {
        QSplitter::insertWidget(index, widget);
        setStretchFactor(index, stretch);
    }
}

void Splitter::setStretchFactors(const QVector<int>& stretchFactors) {
    int ncount = qMin(stretchFactors.size(), count());
    for (int i = 0; i < ncount; ++i) {
        setStretchFactor(i, stretchFactors[i]);
    }
}
