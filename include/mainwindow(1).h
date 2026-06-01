#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "datastore.h"
#include <QMainWindow>
#include <QTableWidgetItem>

class QComboBox;
class QLabel;
class QVBoxLayout;
class QProgressBar;
class QTableWidget;
class RadarChartWidget;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void addCourse();
    void removeCourse();
    void addExperience();
    void removeExperience();
    void addAchievement();
    void removeAchievement();
    void saveData();
    void loadData();
    void exportResume();
    void refreshDashboard();
    void loadDemoData();

private:
    void setupTables();
    void setupStyle();
    void setupDashboardScrollArea();
    void setupRadarSection();
    void setupCareerSection();
    void collectProfileFromUi();
    void renderAll();
    void renderCourses();
    void renderExperiences();
    void renderAchievements();
    void showMessage(const QString &text);
    bool validateCourseInput(QString *reason) const;

    Ui::MainWindow *ui;
    DataStore store;
    RadarChartWidget *radarChart = nullptr;
    QLabel *labelRadarInsight = nullptr;
    QComboBox *comboCareerTarget = nullptr;
    QProgressBar *progressCareerMatch = nullptr;
    QLabel *labelCareerInsight = nullptr;
    QTableWidget *careerRoadmapTable = nullptr;
    QVBoxLayout *dashboardContentLayout = nullptr;
};

#endif // MAINWINDOW_H
