#include "resumeexporter.h"

#include <QFile>
#include <QTextStream>

bool ResumeExporter::exportMarkdown(const DataStore &store, const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) return false;

    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    out << "# " << store.profile.name << " - 个人简历\n\n";
    out << "**学校**：" << store.profile.university << "  \n";
    out << "**专业**：" << store.profile.major << "  \n";
    out << "**发展方向**：" << store.profile.goal << "  \n";
    if (!store.profile.email.isEmpty()) out << "**邮箱**：" << store.profile.email << "  \n";
    out << "**当前 GPA**：" << QString::number(store.calculateGpa(), 'f', 2) << "\n\n";

    out << "## 课程记录\n";
    for (const auto &c : store.courses) {
        out << "- " << c.semester << "｜" << c.name << "｜学分 " << c.credit
            << "｜成绩 " << c.score << "｜绩点 " << QString::number(c.gradePoint(), 'f', 1) << "\n";
    }

    out << "\n## 项目 / 竞赛 / 实习经历\n";
    for (const auto &e : store.experiences) {
        out << "- **" << e.title << "**（" << e.type << "，" << e.role << "，"
            << e.startDate.toString("yyyy.MM") << "-" << e.endDate.toString("yyyy.MM") << "）\n"
            << "  - " << e.description << "\n";
    }

    out << "\n## 成就与角色\n";
    for (const auto &a : store.achievements) {
        out << "- " << a.date.toString("yyyy.MM.dd") << "｜" << a.category << "｜"
            << a.title << "｜" << a.level;
        if (!a.note.isEmpty()) out << "｜" << a.note;
        out << "\n";
    }

    out << "\n## 能力雷达画像\n";
    for (const auto &score : store.abilityScores()) {
        out << "- " << score.name << "：" << score.score << "/100\n";
    }
    out << "\n" << store.abilitySummary() << "\n";

    const auto targets = store.careerTargets();
    if (!targets.isEmpty()) {
        QString targetName = targets.first().name;
        for (const auto &target : targets) {
            if (store.profile.goal.contains(target.name) || target.name.contains(store.profile.goal)
                    || target.keywords.join("").contains(store.profile.goal.left(2))) {
                targetName = target.name;
                break;
            }
        }
        const auto plan = store.analyzeCareerTarget(targetName);
        out << "\n## 职业目标规划摘要\n";
        out << "**目标方向**：" << plan.targetName << "  \n";
        out << "**目标匹配度**：" << plan.matchScore << "%  \n";
        out << "**优势**：" << plan.strengths.join("；") << "  \n";
        out << "**待补强**：" << plan.gaps.join("；") << "\n\n";
        out << "### 四年行动路线图\n";
        for (const auto &step : plan.steps) {
            out << "- **" << step.stage << "｜" << step.title << "**（" << step.priority << "优先级）："
                << step.description << "\n";
        }
    }

    out << "\n## 成长画像建议\n" << store.growthSuggestion() << "\n";
    return true;
}
