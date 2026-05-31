#include "datastore.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QStringList>
#include <QTextStream>
#include <QtGlobal>

namespace {
    int boundedScore(double value)
    {
        return qBound(0, static_cast<int>(value + 0.5), 100);
    }

    bool containsAny(const QString& text, const QStringList& keywords)
    {
        for (const auto& keyword : keywords) {
            if (text.contains(keyword, Qt::CaseInsensitive)) {
                return true;
            }
        }
        return false;
    }

    int keywordHits(const QString& text, const QStringList& keywords)
    {
        int hits = 0;
        for (const auto& keyword : keywords) {
            if (text.contains(keyword, Qt::CaseInsensitive)) {
                ++hits;
            }
        }
        return hits;
    }
}

void DataStore::clear()
{
    profile = ProfileInfo{};
    courses.clear();
    experiences.clear();
    achievements.clear();
}

bool DataStore::load(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) return false;

    const auto doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isObject()) return false;

    clear();
    const auto root = doc.object();
    profile = ProfileInfo::fromJson(root.value("profile").toObject());

    for (const auto& v : root.value("courses").toArray()) {
        courses.append(CourseRecord::fromJson(v.toObject()));
    }
    for (const auto& v : root.value("experiences").toArray()) {
        experiences.append(ExperienceRecord::fromJson(v.toObject()));
    }
    for (const auto& v : root.value("achievements").toArray()) {
        achievements.append(AchievementRecord::fromJson(v.toObject()));
    }
    return true;
}

bool DataStore::save(const QString& filePath) const
{
    QJsonObject root;
    root.insert("profile", profile.toJson());

    QJsonArray courseArray;
    for (const auto& item : courses) courseArray.append(item.toJson());
    root.insert("courses", courseArray);

    QJsonArray expArray;
    for (const auto& item : experiences) expArray.append(item.toJson());
    root.insert("experiences", expArray);

    QJsonArray achievementArray;
    for (const auto& item : achievements) achievementArray.append(item.toJson());
    root.insert("achievements", achievementArray);

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) return false;
    file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    return true;
}

double DataStore::totalCredits() const
{
    double sum = 0.0;
    for (const auto& course : courses) sum += course.credit;
    return sum;
}

double DataStore::calculateGpa() const
{
    double weighted = 0.0;
    double credits = 0.0;
    for (const auto& course : courses) {
        weighted += course.gradePoint() * course.credit;
        credits += course.credit;
    }
    return credits <= 0.0001 ? 0.0 : weighted / credits;
}

QString DataStore::growthSuggestion() const
{
    const double gpa = calculateGpa();
    const int expCount = experiences.size();
    const int prizeCount = achievements.size();

    if (courses.isEmpty() && experiences.isEmpty() && achievements.isEmpty()) {
        return "还没有足够数据。建议先录入本学期课程、一次项目经历和一个短期目标。";
    }
    if (gpa >= 3.7 && expCount < 2) {
        return "学习基础比较稳，可以补充竞赛、项目或实习经历，让简历从“成绩好”变成“能做事”。";
    }
    if (gpa < 2.5 && !courses.isEmpty()) {
        return "课程成绩还有提升空间。建议先复盘低分课程，安排每周固定学习时间，再增加课外任务。";
    }
    if (expCount >= 3 && prizeCount == 0) {
        return "经历记录较丰富，可以尝试把项目成果沉淀为奖项、证书、作品集或公开展示材料。";
    }
    return "整体发展比较均衡。下一步可以把课程、项目和职业目标连接起来，形成更明确的四年规划线。";
}

QList<AbilityScore> DataStore::abilityScores() const
{
    const bool hasAnyData = !courses.isEmpty() || !experiences.isEmpty() || !achievements.isEmpty();
    if (!hasAnyData) {
        return { {"学业基础", 0}, {"实践能力", 0}, {"竞赛科研", 0}, {"荣誉成果", 0}, {"领导协作", 0} };
    }

    const double gpaPart = calculateGpa() / 4.0 * 70.0;
    const double coursePart = qMin(courses.size() * 6.0, 30.0);
    const int academic = boundedScore(gpaPart + coursePart);

    int projectCount = 0;
    int internshipCount = 0;
    int activityCount = 0;
    int competitionCount = 0;
    int researchKeywordCount = 0;
    int leadershipKeywordCount = 0;
    const QStringList researchKeywords = { "研究", "论文", "科研", "实验", "分析", "算法", "数据" };
    const QStringList leadershipKeywords = { "负责人", "组长", "代表", "部长", "组织", "协调", "队长", "管理" };

    for (const auto& item : experiences) {
        const QString text = item.type + item.title + item.role + item.description;
        if (item.type.contains("项目")) ++projectCount;
        if (item.type.contains("实习")) ++internshipCount;
        if (item.type.contains("课外") || item.type.contains("活动")) ++activityCount;
        if (item.type.contains("竞赛")) ++competitionCount;
        if (containsAny(text, researchKeywords)) ++researchKeywordCount;
        if (containsAny(text, leadershipKeywords)) ++leadershipKeywordCount;
    }

    int honorCount = 0;
    int roleCount = 0;
    int volunteerCount = 0;
    int certificateCount = 0;
    for (const auto& item : achievements) {
        const QString text = item.category + item.title + item.level + item.note;
        if (item.category.contains("奖项") || item.category.contains("荣誉") || containsAny(text, { "奖", "优秀", "荣誉" })) ++honorCount;
        if (item.category.contains("角色") || containsAny(text, leadershipKeywords)) ++roleCount;
        if (item.category.contains("志愿") || text.contains("志愿")) ++volunteerCount;
        if (text.contains("证书") || text.contains("认证")) ++certificateCount;
    }

    const int practice = boundedScore(projectCount * 22 + internshipCount * 26 + activityCount * 14 + experiences.size() * 6);
    const int research = boundedScore(competitionCount * 28 + researchKeywordCount * 14 + projectCount * 8);
    const int honor = boundedScore(honorCount * 32 + certificateCount * 20 + achievements.size() * 6);
    const int leadership = boundedScore(roleCount * 28 + volunteerCount * 18 + leadershipKeywordCount * 16);

    return { {"学业基础", academic}, {"实践能力", practice}, {"竞赛科研", research},
            {"荣誉成果", honor}, {"领导协作", leadership} };
}

QString DataStore::abilitySummary() const
{
    const auto scores = abilityScores();
    if (courses.isEmpty() && experiences.isEmpty() && achievements.isEmpty()) {
        return "能力雷达图：暂无足够数据。\n建议先录入课程、一次项目/竞赛/实习经历，以及一个奖项或角色记录，系统会自动生成能力画像。";
    }

    AbilityScore strongest = scores.first();
    AbilityScore weakest = scores.first();
    for (const auto& score : scores) {
        if (score.score > strongest.score) strongest = score;
        if (score.score < weakest.score) weakest = score;
    }

    QString advice;
    if (weakest.name == "学业基础") {
        advice = "建议补充课程记录并优先提升低分课程，稳定 GPA 是后续规划的底盘。";
    }
    else if (weakest.name == "实践能力") {
        advice = "建议增加项目、实习或课外活动，把课堂知识转化为可展示作品。";
    }
    else if (weakest.name == "竞赛科研") {
        advice = "建议参加一次竞赛训练或小型研究项目，沉淀数据分析、算法或论文类成果。";
    }
    else if (weakest.name == "荣誉成果") {
        advice = "建议把已有项目成果包装为奖项、证书、作品集或公开展示材料。";
    }
    else {
        advice = "建议主动承担组长、负责人、志愿服务或班级角色，补强协作与组织能力。";
    }

    return QString("能力雷达图：当前最突出的是“%1”（%2分），最需要补强的是“%3”（%4分）。\n%5")
        .arg(strongest.name)
        .arg(strongest.score)
        .arg(weakest.name)
        .arg(weakest.score)
        .arg(advice);
}

QList<CareerTarget> DataStore::careerTargets() const
{
    return {
        {"数据分析师", {25, 25, 25, 10, 15}, {"数据", "分析", "Python", "数据库", "算法", "可视化", "建模"}},
        {"软件开发工程师", {25, 35, 15, 10, 15}, {"软件", "开发", "Qt", "C++", "算法", "数据结构", "数据库", "系统"}},
        {"金融科技/金融分析", {25, 25, 20, 15, 15}, {"金融", "数据", "分析", "建模", "量化", "经济", "风险"}},
        {"保研/科研方向", {30, 15, 35, 15, 5}, {"科研", "研究", "论文", "实验", "竞赛", "算法", "建模"}},
        {"产品/项目管理方向", {15, 20, 10, 20, 35}, {"产品", "项目", "调研", "组织", "协调", "负责人", "组长"}}
    };
}

CareerPlan DataStore::analyzeCareerTarget(const QString& targetName) const
{
    const auto targets = careerTargets();
    CareerTarget target = targets.first();
    for (const auto& item : targets) {
        if (item.name == targetName) {
            target = item;
            break;
        }
    }

    CareerPlan plan;
    plan.targetName = target.name;

    const bool hasAnyData = !courses.isEmpty() || !experiences.isEmpty() || !achievements.isEmpty();
    if (!hasAnyData) {
        plan.matchScore = 0;
        plan.gaps << "暂无课程、经历或成就数据，系统还无法判断目标匹配度。";
        plan.steps = {
            {"大一", "建立基础档案", "先录入核心课程、一次课堂项目或社团经历，让系统形成初始画像。", "高"},
            {"大二", "补充方向经历", "围绕目标方向选择一门关键课程，并完成一个可展示的小项目。", "中"},
            {"大三", "形成代表成果", "参加竞赛、科研、实习或项目实践，沉淀作品集与证明材料。", "中"},
            {"大四", "整理申请材料", "根据目标整理简历、作品集、成绩单和面试讲述材料。", "中"}
        };
        return plan;
    }

    const auto scores = abilityScores();
    int weighted = 0;
    int weightSum = 0;
    for (int i = 0; i < scores.size() && i < target.abilityWeights.size(); ++i) {
        weighted += scores.at(i).score * target.abilityWeights.at(i);
        weightSum += target.abilityWeights.at(i);
    }

    QString corpus = profile.major + profile.goal;
    for (const auto& course : courses) corpus += course.name + course.semester;
    for (const auto& exp : experiences) corpus += exp.type + exp.title + exp.role + exp.description;
    for (const auto& ach : achievements) corpus += ach.category + ach.title + ach.level + ach.note;
    const int keywordBonus = qMin(keywordHits(corpus, target.keywords) * 3, 12);
    plan.matchScore = boundedScore((weightSum == 0 ? 0 : weighted / static_cast<double>(weightSum)) + keywordBonus);

    for (const auto& score : scores) {
        if (score.score >= 75) {
            plan.strengths << QString("%1较突出（%2分）").arg(score.name).arg(score.score);
        }
        else if (score.score < 55) {
            plan.gaps << QString("%1仍需补强（%2分）").arg(score.name).arg(score.score);
        }
    }
    if (keywordBonus < 6) {
        plan.gaps << "目标方向关键词积累不足，建议补充更贴近目标的课程、项目或成果描述。";
    }
    if (plan.strengths.isEmpty()) {
        plan.strengths << "已有记录能够形成初步画像，但还没有特别突出的优势维度。";
    }
    if (plan.gaps.isEmpty()) {
        plan.gaps << "整体匹配较均衡，下一步应把已有经历整理成作品集和简历亮点。";
    }

    if (target.name == "数据分析师") {
        plan.steps = {
            {"大一", "夯实数学与编程", "重点维护高数、统计、Python、数据库等课程记录，保证 GPA 稳定。", "高"},
            {"大二", "完成数据分析作品", "选择一个真实数据集，完成清洗、建模、可视化和报告展示。", "高"},
            {"大三", "争取竞赛或实习", "参加数据分析竞赛或寻找数据分析实习，把项目成果变成可讲述案例。", "高"},
            {"大四", "沉淀作品集与简历", "整理 2-3 个数据项目，准备面试中的业务理解和模型解释。", "中"}
        };
    }
    else if (target.name == "软件开发工程师") {
        plan.steps = {
            {"大一", "建立编程基础", "强化 C++、数据结构、算法和 Git 使用习惯，补全课程记录。", "高"},
            {"大二", "做一个完整应用", "完成带界面、数据存储和导出能力的项目，并记录架构设计。", "高"},
            {"大三", "参与团队项目或实习", "积累协作开发、代码评审、测试和部署经验。", "高"},
            {"大四", "准备工程化作品集", "整理项目 README、演示视频、源码链接和常见面试题。", "中"}
        };
    }
    else if (target.name == "金融科技/金融分析") {
        plan.steps = {
            {"大一", "补齐金融与数据基础", "保持数学、经济、金融、编程课程记录，形成交叉背景。", "高"},
            {"大二", "做金融数据分析项目", "围绕股票、基金、宏观数据或风控主题完成一次分析报告。", "高"},
            {"大三", "寻找金融科技实践", "争取实验室、券商、银行、金融科技公司的项目或实习经历。", "高"},
            {"大四", "形成行业化表达", "把数据能力翻译成金融业务价值，整理简历和案例讲述。", "中"}
        };
    }
    else if (target.name == "保研/科研方向") {
        plan.steps = {
            {"大一", "稳定 GPA 和基础课", "优先保证核心课程成绩，建立高质量课程档案。", "高"},
            {"大二", "进入科研训练", "联系老师或实验室，参与小型研究、复现论文或竞赛建模。", "高"},
            {"大三", "产出科研成果", "争取论文、竞赛奖项、项目报告或推荐信素材。", "高"},
            {"大四", "准备申请材料", "整理成绩排名、科研经历、个人陈述和面试问答。", "中"}
        };
    }
    else {
        plan.steps = {
            {"大一", "培养表达与协作", "主动参与社团、班级或课程小组，记录角色与贡献。", "中"},
            {"大二", "完成用户调研项目", "围绕一个真实问题做调研、需求分析、原型和汇报。", "高"},
            {"大三", "承担负责人角色", "在项目、竞赛或活动中担任组长/负责人，积累组织协调案例。", "高"},
            {"大四", "整理项目管理案例", "用 STAR 方法整理冲突处理、进度推进和成果交付案例。", "中"}
        };
    }

    return plan;
}

void DataStore::loadDemoData()
{
    clear();
    profile.name = "Tiffany";
    profile.university = "对外经济贸易大学";
    profile.major = "数据科学与大数据技术";
    profile.goal = "数据科学 + 金融分析";
    profile.email = "tiffany@example.com";

    courses.append({ "高等数学（二）", "大一春季", 4, 88 });
    courses.append({ "数据结构与算法分析", "大一春季", 3, 91 });
    courses.append({ "软件体系结构与设计模式", "大一春季", 2, 86 });
    courses.append({ "Python 数据分析", "大二秋季", 3, 93 });
    courses.append({ "数据库系统", "大二秋季", 3, 89 });

    experiences.append({ "项目", "个人发展规划系统原型", "后端与数据管理负责人", QDate(2026, 5, 1), QDate(2026, 6, 10), "使用 Qt/C++ 完成课程、经历、成就、能力雷达图与简历导出模块。" });
    experiences.append({ "竞赛", "校内数据分析训练", "成员", QDate(2026, 3, 10), QDate(2026, 4, 20), "负责数据整理、算法建模和结果解释，训练基本的数据分析表达能力。" });
    experiences.append({ "实习", "金融科技实验室助理", "数据分析助理", QDate(2026, 7, 1), QDate(2026, 8, 20), "参与研究数据清洗和可视化分析，协助完成阶段性报告。" });
    experiences.append({ "课外活动", "学院职业发展工作坊", "小组组长", QDate(2026, 4, 5), QDate(2026, 4, 30), "组织小组分工与展示，协调成员完成职业路径调研。" });

    achievements.append({ "荣誉", "课程项目优秀展示", QDate(2026, 6, 1), "院级", "完成可运行原型，展示界面、架构和扩展性。" });
    achievements.append({ "奖项", "数据分析训练营优秀成员", QDate(2026, 4, 25), "校级", "竞赛训练成果优秀。" });
    achievements.append({ "角色", "学习小组组长", QDate(2026, 3, 1), "班级", "负责组织每周复盘与资料整理。" });
}
