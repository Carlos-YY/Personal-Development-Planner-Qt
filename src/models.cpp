#include "models.h"

double CourseRecord::gradePoint() const
{
    if (score >= 90) return 4.0;
    if (score >= 85) return 3.7;
    if (score >= 82) return 3.3;
    if (score >= 78) return 3.0;
    if (score >= 75) return 2.7;
    if (score >= 72) return 2.3;
    if (score >= 68) return 2.0;
    if (score >= 64) return 1.5;
    if (score >= 60) return 1.0;
    return 0.0;
}

QJsonObject CourseRecord::toJson() const
{
    return { {"name", name}, {"semester", semester}, {"credit", credit}, {"score", score} };
}

CourseRecord CourseRecord::fromJson(const QJsonObject& obj)
{
    CourseRecord item;
    item.name = obj.value("name").toString();
    item.semester = obj.value("semester").toString();
    item.credit = obj.value("credit").toDouble();
    item.score = obj.value("score").toDouble();
    return item;
}

QJsonObject ExperienceRecord::toJson() const
{
    return { {"type", type}, {"title", title}, {"role", role},
            {"startDate", startDate.toString(Qt::ISODate)},
            {"endDate", endDate.toString(Qt::ISODate)},
            {"description", description} };
}

ExperienceRecord ExperienceRecord::fromJson(const QJsonObject& obj)
{
    ExperienceRecord item;
    item.type = obj.value("type").toString();
    item.title = obj.value("title").toString();
    item.role = obj.value("role").toString();
    item.startDate = QDate::fromString(obj.value("startDate").toString(), Qt::ISODate);
    item.endDate = QDate::fromString(obj.value("endDate").toString(), Qt::ISODate);
    item.description = obj.value("description").toString();
    return item;
}

QJsonObject AchievementRecord::toJson() const
{
    return { {"category", category}, {"title", title},
            {"date", date.toString(Qt::ISODate)}, {"level", level}, {"note", note} };
}

AchievementRecord AchievementRecord::fromJson(const QJsonObject& obj)
{
    AchievementRecord item;
    item.category = obj.value("category").toString();
    item.title = obj.value("title").toString();
    item.date = QDate::fromString(obj.value("date").toString(), Qt::ISODate);
    item.level = obj.value("level").toString();
    item.note = obj.value("note").toString();
    return item;
}

QJsonObject ProfileInfo::toJson() const
{
    return { {"name", name}, {"major", major}, {"university", university},
            {"goal", goal}, {"email", email} };
}

ProfileInfo ProfileInfo::fromJson(const QJsonObject& obj)
{
    ProfileInfo profile;
    profile.name = obj.value("name").toString(profile.name);
    profile.major = obj.value("major").toString(profile.major);
    profile.university = obj.value("university").toString(profile.university);
    profile.goal = obj.value("goal").toString(profile.goal);
    profile.email = obj.value("email").toString();
    return profile;
}
