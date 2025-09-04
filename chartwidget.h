#ifndef CHARTWIDGET_H
#define CHARTWIDGET_H

#include <QObject>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QGroupBox>
#include <QFrame>
#include <QDateEdit>
#include <QSpinBox>
#include <QCheckBox>
#include <QTimer>
#include <QDateTime>
#include <QList>
#include <QMap>
#include <QPainter>
#include <QPaintEvent>
#include <QEvent>
#include <QMouseEvent>
#include <QToolTip>
#include <QFont>
#include <QColor>
#include <QPen>
#include <QBrush>
#include <QLinearGradient>
#include <QRadialGradient>
#include <QConicalGradient>
#include "database.h"
#include "user.h"

class ChartWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ChartWidget(Database* database, const User& currentUser, QWidget *parent = nullptr);
    ~ChartWidget();

public slots:
    void refreshCharts();
    void exportCharts();

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    bool eventFilter(QObject* obj, QEvent* event) override;

private slots:
    void onChartTypeChanged(const QString& chartType);
    void onDateRangeChanged();
    void onRefreshClicked();
    void onExportClicked();
    void onAutoRefreshChanged(bool checked);
    void onAnimationTimer();

signals:
    void chartDataChanged();

private:
    Database* database_;
    User currentUser_;
    
    // UI组件
    QVBoxLayout* mainLayout_;
    QHBoxLayout* controlLayout_;
    QGroupBox* controlGroup_;
    QLabel* chartTypeLabel_;
    QComboBox* chartTypeCombo_;
    QLabel* dateRangeLabel_;
    QDateEdit* startDateEdit_;
    QDateEdit* endDateEdit_;
    QPushButton* refreshButton_;
    QPushButton* exportButton_;
    QCheckBox* autoRefreshCheck_;
    QSpinBox* refreshIntervalSpin_;
    QLabel* refreshIntervalLabel_;
    
    // 图表区域
    QFrame* chartFrame_;
    QVBoxLayout* chartLayout_;
    QLabel* chartTitleLabel_;
    QLabel* chartSubtitleLabel_;
    QWidget* chartDrawArea_;
    
    // 数据
    QMap<QString, int> userStats_;
    QMap<QString, int> questionStats_;
    QMap<QString, int> documentStats_;
    QMap<QString, QList<QPair<QString, int>>> timeSeriesData_;
    QList<QPair<QString, double>> pieChartData_;
    QList<QPair<QString, int>> barChartData_;
    QList<QPair<QString, int>> lineChartData_;
    
    // 图表状态
    QString currentChartType_;
    QRect chartArea_;
    QTimer* animationTimer_;
    QTimer* autoRefreshTimer_;
    int animationStep_;
    bool isAnimating_;
    
    // 交互状态
    QPoint lastMousePos_;
    bool isDragging_;
    double zoomLevel_;
    QPoint panOffset_;
    int hoveredSlice_;
    int hoveredBar_;
    int hoveredPoint_;

    // 私有方法
    void setupUI();
    void setupConnections();
    void applyStyles();
    void loadData();
    void updateChartArea();
    void drawChart(QPainter& painter);
    void drawPieChart(QPainter& painter);
    void drawBarChart(QPainter& painter);
    void drawLineChart(QPainter& painter);
    void drawDoughnutChart(QPainter& painter);
    void drawAreaChart(QPainter& painter);
    void drawLegend(QPainter& painter);
    void drawTitle(QPainter& painter);
    void drawGrid(QPainter& painter);
    void drawAxis(QPainter& painter);
    void drawDataLabels(QPainter& painter);
    
    // 数据处理方法
    void processUserData();
    void processQuestionData();
    void processDocumentData();
    void processTimeSeriesData();
    void calculateChartData();
    void updateStatistics();
    void loadSampleData();
    
    // 交互方法
    void handleMouseClick(const QPoint& pos);
    void handleMouseHover(const QPoint& pos);
    void updateTooltip(const QPoint& pos);
    bool isPointInChartArea(const QPoint& pos);
    int getSliceAtPoint(const QPoint& pos);
    int getBarAtPoint(const QPoint& pos);
    int getPointAtPoint(const QPoint& pos);
    
    // 动画方法
    void startAnimation();
    void stopAnimation();
    void updateAnimation();
    void animatePieChart(QPainter& painter);
    void animateBarChart(QPainter& painter);
    void animateLineChart(QPainter& painter);
    
    // 样式方法
    void setupColors();
    void setupFonts();
    void setupGradients();
    QColor getChartColor(int index);
    QLinearGradient createBarGradient(const QRect& rect, const QColor& color);
    QRadialGradient createPieGradient(const QRect& rect, const QColor& color);
    
    // 辅助方法
    bool exportToImage(const QString& filePath);
    bool exportToPDF(const QString& filePath);
    void saveChartData();
    QString formatNumber(int number);
    QString formatPercentage(double percentage);
    QRect calculateChartRect();
    double calculateAngle(const QPoint& center, const QPoint& point);
    QPoint calculatePointOnCircle(const QPoint& center, double radius, double angle);
};

#endif // CHARTWIDGET_H
