#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "radarchartwidget.h"
#include "resumeexporter.h"

#include <QDir>
#include <QComboBox>
#include <QFileDialog>
#include <QFrame>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QProgressBar>
#include <QScrollArea>
#include <QStandardPaths>
#include <QTableWidget>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setupTables();
    setupStyle();
    setupDashboardScrollArea();
    setupRadarSection();
    setupCareerSection();

    connect(ui->btnDashboard, &QPushButton::clicked, this, [this]{ ui->pages->setCurrentWidget(ui->dashboardPage); refreshDashboard(); });
    connect(ui->btnCourses, &QPushButton::clicked, this, [this]{ ui->pages->setCurrentWidget(ui->coursePage); });
    connect(ui->btnExperience, &QPushButton::clicked, this, [this]{ ui->pages->setCurrentWidget(ui->experiencePage); });
    connect(ui->btnAchievement, &QPushButton::clicked, this, [this]{ ui->pages->setCurrentWidget(ui->achievementPage); });
    connect(ui->btnResume, &QPushButton::clicked, this, [this]{ ui->pages->setCurrentWidget(ui->resumePage); refreshDashboard(); });

    connect(ui->btnAddCourse, &QPushButton::clicked, this, &MainWindow::addCourse);
    connect(ui->btnRemoveCourse, &QPushButton::clicked, this, &MainWindow::removeCourse);
    connect(ui->btnAddExperience, &QPushButton::clicked, this, &MainWindow::addExperience);
    connect(ui->btnRemoveExperience, &QPushButton::clicked, this, &MainWindow::removeExperience);
    connect(ui->btnAddAchievement, &QPushButton::clicked, this, &MainWindow::addAchievement);
    connect(ui->btnRemoveAchievement, &QPushButton::clicked, this, &MainWindow::removeAchievement);

    connect(ui->actionSave, &QAction::triggered, this, &MainWindow::saveData);
    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::loadData);
    connect(ui->actionExportResume, &QAction::triggered, this, &MainWindow::exportResume);
    connect(ui->btnSaveData, &QPushButton::clicked, this, &MainWindow::saveData);
    connect(ui->btnLoadData, &QPushButton::clicked, this, &MainWindow::loadData);
    connect(ui->btnExportResume, &QPushButton::clicked, this, &MainWindow::exportResume);
    connect(ui->btnDemo, &QPushButton::clicked, this, &MainWindow::loadDemoData);

    ui->dateExpStart->setDate(QDate::currentDate().addMonths(-1));
    ui->dateExpEnd->setDate(QDate::currentDate());
    ui->dateAchievement->setDate(QDate::currentDate());
    renderAll();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupStyle()
{
    setWindowTitle("Curriculum Vitae - 个人发展规划系统");
    resize(1180, 760);

    const QString css = R"(
        QMainWindow { background: #f5f7fb; }
        QScrollArea { border: none; background: #f5f7fb; }
        #leftPanel { background: #172033; border: none; }
        #brandTitle { color: white; font-size: 20px; font-weight: 700; }
        #brandSubTitle { color: #aeb8cc; }
        QPushButton { border: 1px solid #d6dce8; border-radius: 8px; padding: 8px 14px; background: white; }
        QPushButton:hover { color: #172033; background: #eef3ff; border-color: #9fb5e8; }
        QPushButton:pressed { color: #172033; background: #dfe9ff; border-color: #7f9ddd; }
        QPushButton[nav="true"] { color: #e7eefc; background: transparent; border: 1px solid transparent; padding: 12px 16px; text-align: left; border-radius: 10px; }
        QPushButton[nav="true"]:hover { color: #ffffff; background: #2f4161; border-color: #516989; }
        QPushButton[nav="true"]:pressed { color: #ffffff; background: #3867d6; border-color: #6f95ef; }
        QPushButton[primary="true"] { background: #3867d6; color: white; border: none; border-radius: 8px; padding: 8px 16px; }
        QPushButton[primary="true"]:hover { color: white; background: #315ccc; border: none; }
        QPushButton[primary="true"]:pressed { color: white; background: #244fb3; border: none; }
        QLineEdit, QDoubleSpinBox, QComboBox, QDateEdit, QTextEdit { color: #172033; border: 1px solid #d6dce8; border-radius: 8px; padding: 7px; background: white; selection-background-color: #3867d6; selection-color: white; }
        QComboBox:hover, QDateEdit:hover, QDoubleSpinBox:hover { color: #172033; background: #ffffff; border-color: #8fa8df; }
        QComboBox QAbstractItemView { color: #172033; background: #ffffff; border: 1px solid #cbd5e1; border-radius: 6px; outline: none; selection-background-color: #3867d6; selection-color: #ffffff; padding: 4px; }
        QComboBox QAbstractItemView::item { min-height: 28px; padding: 6px 10px; color: #172033; background: #ffffff; }
        QComboBox QAbstractItemView::item:hover { color: #ffffff; background: #3867d6; }
        QComboBox QAbstractItemView::item:selected { color: #ffffff; background: #3867d6; }
        QTableWidget { background: white; border: 1px solid #dde3ef; border-radius: 10px; gridline-color: #edf0f6; }
        QHeaderView::section { background: #f0f3f9; padding: 8px; border: none; font-weight: 600; }
        QProgressBar { color: #172033; background: #eef2f8; border: 1px solid #d6dce8; border-radius: 8px; text-align: center; padding: 2px; }
        QProgressBar::chunk { background: #3867d6; border-radius: 6px; }
        QLabel[card="true"] { background: white; border: 1px solid #dde3ef; border-radius: 14px; padding: 18px; }
        QLabel[pageTitle="true"] { font-size: 22px; font-weight: 700; color: #172033; }
        QFrame[radarCard="true"] { background: white; border: 1px solid #dde3ef; border-radius: 14px; }
        QLabel[sectionTitle="true"] { font-size: 17px; font-weight: 700; color: #172033; }
        QLabel[muted="true"] { color: #657187; line-height: 1.4; }
    )";
    setStyleSheet(css);
}

void MainWindow::setupDashboardScrollArea()
{
    auto *scrollArea = new QScrollArea(ui->dashboardPage);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);

    auto *content = new QWidget(scrollArea);
    dashboardContentLayout = new QVBoxLayout(content);
    dashboardContentLayout->setContentsMargins(28, 24, 28, 24);
    dashboardContentLayout->setSpacing(12);

    ui->dashboardLayout->setContentsMargins(0, 0, 0, 0);
    ui->dashboardLayout->setSpacing(0);
    while (QLayoutItem *item = ui->dashboardLayout->takeAt(0)) {
        dashboardContentLayout->addItem(item);
    }

    scrollArea->setWidget(content);
    ui->dashboardLayout->addWidget(scrollArea);
}

void MainWindow::setupRadarSection()
{
    auto *radarCard = new QFrame(ui->dashboardPage);
    radarCard->setProperty("radarCard", "true");
    auto *cardLayout = new QHBoxLayout(radarCard);
    cardLayout->setContentsMargins(18, 18, 18, 18);
    cardLayout->setSpacing(20);

    radarChart = new RadarChartWidget(radarCard);
    cardLayout->addWidget(radarChart, 3);

    auto *textPanel = new QWidget(radarCard);
    auto *textLayout = new QVBoxLayout(textPanel);
    textLayout->setContentsMargins(0, 0, 0, 0);
    textLayout->setSpacing(10);

    auto *title = new QLabel("个人能力雷达图", textPanel);
    title->setProperty("sectionTitle", "true");
    textLayout->addWidget(title);

    auto *description = new QLabel("系统会根据课程、经历、成就自动生成五维能力画像，可作为本项目的创新功能进行现场演示。", textPanel);
    description->setWordWrap(true);
    description->setProperty("muted", "true");
    textLayout->addWidget(description);

    labelRadarInsight = new QLabel(textPanel);
    labelRadarInsight->setWordWrap(true);
    labelRadarInsight->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    labelRadarInsight->setProperty("muted", "true");
    textLayout->addWidget(labelRadarInsight, 1);

    cardLayout->addWidget(textPanel, 2);
    dashboardContentLayout->insertWidget(3, radarCard);
}

void MainWindow::setupCareerSection()
{
    auto *careerCard = new QFrame(ui->dashboardPage);
    careerCard->setProperty("radarCard", "true");
    auto *outerLayout = new QVBoxLayout(careerCard);
    outerLayout->setContentsMargins(18, 18, 18, 18);
    outerLayout->setSpacing(12);

    auto *topLayout = new QHBoxLayout();
    auto *title = new QLabel("职业目标差距分析与四年路线图", careerCard);
    title->setProperty("sectionTitle", "true");
    topLayout->addWidget(title, 1);

    comboCareerTarget = new QComboBox(careerCard);
    for (const auto &target : store.careerTargets()) {
        comboCareerTarget->addItem(target.name);
    }
    comboCareerTarget->setMinimumWidth(190);
    topLayout->addWidget(comboCareerTarget);
    outerLayout->addLayout(topLayout);

    auto *middleLayout = new QHBoxLayout();
    progressCareerMatch = new QProgressBar(careerCard);
    progressCareerMatch->setRange(0, 100);
    progressCareerMatch->setFormat("目标匹配度 %p%");
    progressCareerMatch->setMinimumHeight(30);
    middleLayout->addWidget(progressCareerMatch, 1);

    labelCareerInsight = new QLabel(careerCard);
    labelCareerInsight->setWordWrap(true);
    labelCareerInsight->setProperty("muted", "true");
    middleLayout->addWidget(labelCareerInsight, 2);
    outerLayout->addLayout(middleLayout);

    careerRoadmapTable = new QTableWidget(careerCard);
    careerRoadmapTable->setColumnCount(4);
    careerRoadmapTable->setHorizontalHeaderLabels({"阶段", "行动任务", "说明", "优先级"});
    careerRoadmapTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    careerRoadmapTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    careerRoadmapTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    careerRoadmapTable->setMinimumHeight(190);
    outerLayout->addWidget(careerRoadmapTable);

    connect(comboCareerTarget, &QComboBox::currentTextChanged, this, [this] {
        refreshDashboard();
    });

    dashboardContentLayout->insertWidget(4, careerCard);
}

void MainWindow::setupTables()
{
    ui->courseTable->setColumnCount(5);
    ui->courseTable->setHorizontalHeaderLabels({"学期", "课程", "学分", "成绩", "绩点"});
    ui->courseTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->courseTable->setSelectionBehavior(QAbstractItemView::SelectRows);

    ui->experienceTable->setColumnCount(5);
    ui->experienceTable->setHorizontalHeaderLabels({"类型", "名称", "角色", "时间", "说明"});
    ui->experienceTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->experienceTable->setSelectionBehavior(QAbstractItemView::SelectRows);

    ui->achievementTable->setColumnCount(5);
    ui->achievementTable->setHorizontalHeaderLabels({"类别", "名称", "日期", "级别", "备注"});
    ui->achievementTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->achievementTable->setSelectionBehavior(QAbstractItemView::SelectRows);
}

void MainWindow::collectProfileFromUi()
{
    store.profile.name = ui->editName->text().trimmed();
    store.profile.university = ui->editSchool->text().trimmed();
    store.profile.major = ui->editMajor->text().trimmed();
    store.profile.goal = ui->editGoal->text().trimmed();
    store.profile.email = ui->editEmail->text().trimmed();
}

bool MainWindow::validateCourseInput(QString *reason) const
{
    if (ui->editCourseName->text().trimmed().isEmpty()) {
        *reason = "课程名称不能为空。";
        return false;
    }
    if (ui->spinCredit->value() <= 0.0) {
        *reason = "学分必须大于 0。";
        return false;
    }
    return true;
}

void MainWindow::addCourse()
{
    QString reason;
    if (!validateCourseInput(&reason)) {
        QMessageBox::warning(this, "输入不完整", reason);
        return;
    }
    store.courses.append({ui->editCourseName->text().trimmed(), ui->editSemester->text().trimmed(),
                          ui->spinCredit->value(), ui->spinScore->value()});
    ui->editCourseName->clear();
    renderAll();
}

void MainWindow::removeCourse()
{
    const int row = ui->courseTable->currentRow();
    if (row >= 0 && row < store.courses.size()) {
        store.courses.removeAt(row);
        renderAll();
    }
}

void MainWindow::addExperience()
{
    if (ui->editExpTitle->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "输入不完整", "经历名称不能为空。");
        return;
    }
    store.experiences.append({ui->comboExpType->currentText(), ui->editExpTitle->text().trimmed(),
                              ui->editExpRole->text().trimmed(), ui->dateExpStart->date(),
                              ui->dateExpEnd->date(), ui->textExpDesc->toPlainText().trimmed()});
    ui->editExpTitle->clear();
    ui->textExpDesc->clear();
    renderAll();
}

void MainWindow::removeExperience()
{
    const int row = ui->experienceTable->currentRow();
    if (row >= 0 && row < store.experiences.size()) {
        store.experiences.removeAt(row);
        renderAll();
    }
}

void MainWindow::addAchievement()
{
    if (ui->editAchievementTitle->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "输入不完整", "成就名称不能为空。");
        return;
    }
    store.achievements.append({ui->comboAchievementType->currentText(), ui->editAchievementTitle->text().trimmed(),
                               ui->dateAchievement->date(), ui->editAchievementLevel->text().trimmed(),
                               ui->editAchievementNote->text().trimmed()});
    ui->editAchievementTitle->clear();
    renderAll();
}

void MainWindow::removeAchievement()
{
    const int row = ui->achievementTable->currentRow();
    if (row >= 0 && row < store.achievements.size()) {
        store.achievements.removeAt(row);
        renderAll();
    }
}

void MainWindow::renderAll()
{
    renderCourses();
    renderExperiences();
    renderAchievements();
    refreshDashboard();
}

void MainWindow::renderCourses()
{
    ui->courseTable->setRowCount(store.courses.size());
    for (int i = 0; i < store.courses.size(); ++i) {
        const auto &c = store.courses.at(i);
        ui->courseTable->setItem(i, 0, new QTableWidgetItem(c.semester));
        ui->courseTable->setItem(i, 1, new QTableWidgetItem(c.name));
        ui->courseTable->setItem(i, 2, new QTableWidgetItem(QString::number(c.credit, 'f', 1)));
        ui->courseTable->setItem(i, 3, new QTableWidgetItem(QString::number(c.score, 'f', 1)));
        ui->courseTable->setItem(i, 4, new QTableWidgetItem(QString::number(c.gradePoint(), 'f', 1)));
    }
}

void MainWindow::renderExperiences()
{
    ui->experienceTable->setRowCount(store.experiences.size());
    for (int i = 0; i < store.experiences.size(); ++i) {
        const auto &e = store.experiences.at(i);
        ui->experienceTable->setItem(i, 0, new QTableWidgetItem(e.type));
        ui->experienceTable->setItem(i, 1, new QTableWidgetItem(e.title));
        ui->experienceTable->setItem(i, 2, new QTableWidgetItem(e.role));
        ui->experienceTable->setItem(i, 3, new QTableWidgetItem(e.startDate.toString("yyyy.MM") + " - " + e.endDate.toString("yyyy.MM")));
        ui->experienceTable->setItem(i, 4, new QTableWidgetItem(e.description));
    }
}

void MainWindow::renderAchievements()
{
    ui->achievementTable->setRowCount(store.achievements.size());
    for (int i = 0; i < store.achievements.size(); ++i) {
        const auto &a = store.achievements.at(i);
        ui->achievementTable->setItem(i, 0, new QTableWidgetItem(a.category));
        ui->achievementTable->setItem(i, 1, new QTableWidgetItem(a.title));
        ui->achievementTable->setItem(i, 2, new QTableWidgetItem(a.date.toString("yyyy-MM-dd")));
        ui->achievementTable->setItem(i, 3, new QTableWidgetItem(a.level));
        ui->achievementTable->setItem(i, 4, new QTableWidgetItem(a.note));
    }
}

void MainWindow::refreshDashboard()
{
    ui->editName->setText(store.profile.name);
    ui->editSchool->setText(store.profile.university);
    ui->editMajor->setText(store.profile.major);
    ui->editGoal->setText(store.profile.goal);
    ui->editEmail->setText(store.profile.email);

    ui->labelGpa->setText(QString("GPA\n%1").arg(QString::number(store.calculateGpa(), 'f', 2)));
    ui->labelCredits->setText(QString("累计学分\n%1").arg(QString::number(store.totalCredits(), 'f', 1)));
    ui->labelExperienceCount->setText(QString("经历数量\n%1").arg(store.experiences.size()));
    ui->labelAchievementCount->setText(QString("成就记录\n%1").arg(store.achievements.size()));
    ui->labelAdvice->setText("成长画像分析：\n" + store.growthSuggestion());
    if (radarChart) {
        radarChart->setScores(store.abilityScores());
    }
    if (labelRadarInsight) {
        labelRadarInsight->setText(store.abilitySummary());
    }
    CareerPlan careerPlan;
    if (comboCareerTarget) {
        careerPlan = store.analyzeCareerTarget(comboCareerTarget->currentText());
        if (progressCareerMatch) {
            progressCareerMatch->setValue(careerPlan.matchScore);
        }
        if (labelCareerInsight) {
            labelCareerInsight->setText(QString("优势：%1\n短板：%2")
                                        .arg(careerPlan.strengths.join("；"))
                                        .arg(careerPlan.gaps.join("；")));
        }
        if (careerRoadmapTable) {
            careerRoadmapTable->setRowCount(careerPlan.steps.size());
            for (int i = 0; i < careerPlan.steps.size(); ++i) {
                const auto &step = careerPlan.steps.at(i);
                careerRoadmapTable->setItem(i, 0, new QTableWidgetItem(step.stage));
                careerRoadmapTable->setItem(i, 1, new QTableWidgetItem(step.title));
                careerRoadmapTable->setItem(i, 2, new QTableWidgetItem(step.description));
                careerRoadmapTable->setItem(i, 3, new QTableWidgetItem(step.priority));
            }
        }
    }

    QString preview;
    preview += "# " + store.profile.name + "\n";
    preview += store.profile.university + "｜" + store.profile.major + "\n";
    preview += "发展方向：" + store.profile.goal + "\n\n";
    preview += "GPA：" + QString::number(store.calculateGpa(), 'f', 2) + "\n";
    preview += "课程：" + QString::number(store.courses.size()) + " 门，经历：" + QString::number(store.experiences.size())
            + " 项，成就：" + QString::number(store.achievements.size()) + " 条\n";
    preview += "\n能力雷达画像：\n";
    for (const auto &score : store.abilityScores()) {
        preview += "- " + score.name + "：" + QString::number(score.score) + "/100\n";
    }
    if (!careerPlan.targetName.isEmpty()) {
        preview += "\n职业目标规划：" + careerPlan.targetName + "\n";
        preview += "匹配度：" + QString::number(careerPlan.matchScore) + "%\n";
        preview += "短板：" + careerPlan.gaps.join("；") + "\n";
    }
    ui->resumePreview->setPlainText(preview);
}

void MainWindow::saveData()
{
    collectProfileFromUi();
    const QString path = QFileDialog::getSaveFileName(this, "保存成长档案", QDir::homePath() + "/pdp-data.json", "JSON 文件 (*.json)");
    if (path.isEmpty()) return;
    if (store.save(path)) showMessage("数据已经保存。\n" + path);
    else QMessageBox::critical(this, "保存失败", "无法写入文件，请检查路径权限。意外退出不会影响已保存文件。 ");
}

void MainWindow::loadData()
{
    const QString path = QFileDialog::getOpenFileName(this, "导入成长档案", QDir::homePath(), "JSON 文件 (*.json)");
    if (path.isEmpty()) return;
    if (store.load(path)) {
        renderAll();
        showMessage("数据已经导入。\n" + path);
    } else {
        QMessageBox::critical(this, "导入失败", "文件格式不正确，或该文件不是本系统导出的 JSON。 ");
    }
}

void MainWindow::exportResume()
{
    collectProfileFromUi();
    const QString path = QFileDialog::getSaveFileName(this, "导出 Markdown 简历", QDir::homePath() + "/个人简历.md", "Markdown 文件 (*.md)");
    if (path.isEmpty()) return;
    if (ResumeExporter::exportMarkdown(store, path)) showMessage("简历已经导出。\n" + path);
    else QMessageBox::critical(this, "导出失败", "无法写入简历文件。 ");
}

void MainWindow::loadDemoData()
{
    store.loadDemoData();
    renderAll();
    showMessage("已载入演示数据。可以直接用于课堂演示，也可以继续修改。 ");
}

void MainWindow::showMessage(const QString &text)
{
    statusBar()->showMessage(text, 5000);
    QMessageBox::information(this, "提示", text);
}
