#ifndef DATASTORE_H
#define DATASTORE_H

#include "models.h"

#include <QList>
#include <QString>
#include <QStringList>

struct AbilityScore {
    QString name;
    int score = 0;
};

struct RoadmapStep {
    QString stage;
    QString title;
    QString description;
    QString priority;
};

struct CareerTarget {
    QString name;
    QList<int> abilityWeights;
    QStringList keywords;
};

struct CareerPlan {
    QString targetName;
    int matchScore = 0;
    QStringList strengths;
    QStringList gaps;
    QList<RoadmapStep> steps;
};

class DataStore
{
public:
    bool load(const QString &filePath);
    bool save(const QString &filePath) const;
    void loadDemoData();
    void clear();

    double calculateGpa() const;
    double totalCredits() const;
    QString growthSuggestion() const;
    QList<AbilityScore> abilityScores() const;
    QString abilitySummary() const;
    QList<CareerTarget> careerTargets() const;
    CareerPlan analyzeCareerTarget(const QString &targetName) const;

    ProfileInfo profile;
    QList<CourseRecord> courses;
    QList<ExperienceRecord> experiences;
    QList<AchievementRecord> achievements;
};

#endif // DATASTORE_H
