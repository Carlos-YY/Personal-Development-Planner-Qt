#ifndef MODELS_H
#define MODELS_H

#include <QString>
#include <QDate>
#include <QJsonObject>

struct CourseRecord {
    QString name;
    QString semester;
    double credit = 0.0;
    double score = 0.0;

    double gradePoint() const;
    QJsonObject toJson() const;
    static CourseRecord fromJson(const QJsonObject &obj);
};

struct ExperienceRecord {
    QString type;
    QString title;
    QString role;
    QDate startDate;
    QDate endDate;
    QString description;

    QJsonObject toJson() const;
    static ExperienceRecord fromJson(const QJsonObject &obj);
};

struct AchievementRecord {
    QString category;
    QString title;
    QDate date;
    QString level;
    QString note;

    QJsonObject toJson() const;
    static AchievementRecord fromJson(const QJsonObject &obj);
};

struct ProfileInfo {
    QString name = "Tiffany";
    QString major = "数据科学与大数据技术";
    QString university = "对外经济贸易大学";
    QString goal = "数据科学与金融交叉方向";
    QString email = "tiffany@example.com";

    QJsonObject toJson() const;
    static ProfileInfo fromJson(const QJsonObject &obj);
};

#endif // MODELS_H
