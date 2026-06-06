#include "radarchartwidget.h"

#include <QPainter>
#include <QPainterPath>
#include <QtMath>

RadarChartWidget::RadarChartWidget(QWidget *parent)
    : QWidget(parent)
{
    setMinimumSize(360, 300);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void RadarChartWidget::setScores(const QList<AbilityScore> &scores)
{
    m_scores = scores;
    update();
}

void RadarChartWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.fillRect(rect(), QColor("#ffffff"));

    if (m_scores.isEmpty()) {
        painter.setPen(QColor("#6b7280"));
        painter.drawText(rect(), Qt::AlignCenter, "暂无能力数据");
        return;
    }

    const QPointF center(width() * 0.5, height() * 0.52);
    const double radius = qMin(width(), height()) * 0.32;
    const int axisCount = m_scores.size();
    const double startAngle = -M_PI / 2.0;

    auto pointFor = [&](int index, double ratio) {
        const double angle = startAngle + 2.0 * M_PI * index / axisCount;
        return QPointF(center.x() + qCos(angle) * radius * ratio,
                       center.y() + qSin(angle) * radius * ratio);
    };

    painter.setPen(QPen(QColor("#dbe3f0"), 1));
    for (int level = 1; level <= 5; ++level) {
        QPolygonF grid;
        const double ratio = level / 5.0;
        for (int i = 0; i < axisCount; ++i) {
            grid << pointFor(i, ratio);
        }
        painter.drawPolygon(grid);
    }

    painter.setPen(QPen(QColor("#c8d2e4"), 1));
    for (int i = 0; i < axisCount; ++i) {
        painter.drawLine(center, pointFor(i, 1.0));
    }

    QPolygonF area;
    for (int i = 0; i < axisCount; ++i) {
        area << pointFor(i, qBound(0, m_scores.at(i).score, 100) / 100.0);
    }

    QPainterPath fillPath;
    fillPath.addPolygon(area);
    fillPath.closeSubpath();
    painter.fillPath(fillPath, QColor(56, 103, 214, 70));
    painter.setPen(QPen(QColor("#3867d6"), 2.4));
    painter.drawPolygon(area);

    painter.setBrush(QColor("#3867d6"));
    painter.setPen(Qt::NoPen);
    for (const auto &point : area) {
        painter.drawEllipse(point, 4.2, 4.2);
    }

    painter.setPen(QColor("#172033"));
    QFont labelFont = painter.font();
    labelFont.setPointSize(10);
    labelFont.setBold(true);
    painter.setFont(labelFont);

    for (int i = 0; i < axisCount; ++i) {
        const QPointF labelPoint = pointFor(i, 1.24);
        const QString text = QString("%1\n%2").arg(m_scores.at(i).name).arg(m_scores.at(i).score);
        QRectF labelRect(labelPoint.x() - 48, labelPoint.y() - 20, 96, 42);
        painter.drawText(labelRect, Qt::AlignCenter, text);
    }

    painter.setPen(QColor("#8a97ad"));
    QFont smallFont = painter.font();
    smallFont.setPointSize(8);
    smallFont.setBold(false);
    painter.setFont(smallFont);
    painter.drawText(QRectF(center.x() - 28, center.y() - radius - 8, 56, 18), Qt::AlignCenter, "100");
    painter.drawText(QRectF(center.x() - 24, center.y() - 9, 48, 18), Qt::AlignCenter, "0");
}
