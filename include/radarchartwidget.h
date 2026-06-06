#ifndef RADARCHARTWIDGET_H
#define RADARCHARTWIDGET_H

#include "datastore.h"

#include <QWidget>

class RadarChartWidget : public QWidget
{
    Q_OBJECT

public:
    explicit RadarChartWidget(QWidget *parent = nullptr);

    void setScores(const QList<AbilityScore> &scores);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QList<AbilityScore> m_scores;
};

#endif // RADARCHARTWIDGET_H
