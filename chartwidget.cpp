#include "chartwidget.h"
#include <QApplication>
#include <QScreen>
#include <QMessageBox>
#include <QFileDialog>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QDateTime>
#include <QTime>
#include <QtMath>
#include <QSettings>
#include <QPdfWriter>
#include <QPageSize>

ChartWidget::ChartWidget(Database* database, const User& currentUser, QWidget *parent)
    : QWidget(parent)
    , database_(database)
    , currentUser_(currentUser)
    , mainLayout_(nullptr)
    , controlLayout_(nullptr)
    , controlGroup_(nullptr)
    , chartTypeLabel_(nullptr)
    , chartTypeCombo_(nullptr)
    , dateRangeLabel_(nullptr)
    , startDateEdit_(nullptr)
    , endDateEdit_(nullptr)
    , refreshButton_(nullptr)
    , exportButton_(nullptr)
    , autoRefreshCheck_(nullptr)
    , refreshIntervalSpin_(nullptr)
    , refreshIntervalLabel_(nullptr)
    , chartFrame_(nullptr)
    , chartLayout_(nullptr)
    , chartTitleLabel_(nullptr)
    , chartSubtitleLabel_(nullptr)
    , chartDrawArea_(nullptr)
    , currentChartType_("饼图")
    , chartArea_(0, 0, 400, 300)
    , animationTimer_(nullptr)
    , autoRefreshTimer_(nullptr)
    , animationStep_(0)
    , isAnimating_(false)
    , lastMousePos_(0, 0)
    , isDragging_(false)
    , zoomLevel_(1.0)
    , panOffset_(0, 0)
    , hoveredSlice_(-1)
    , hoveredBar_(-1)
    , hoveredPoint_(-1)
{
    setupUI();
    setupConnections();
    applyStyles();
    setupColors();
    setupFonts();
    loadData();
}

ChartWidget::~ChartWidget()
{
    if (animationTimer_) {
        animationTimer_->stop();
    }
    if (autoRefreshTimer_) {
        autoRefreshTimer_->stop();
    }
}

void ChartWidget::setupUI()
{
    mainLayout_ = new QVBoxLayout(this);
    mainLayout_->setContentsMargins(10, 10, 10, 10);
    mainLayout_->setSpacing(10);
    
    // 控制面板 - 留出足够空间
    controlGroup_ = new QGroupBox("图表控制");
    controlGroup_->setMinimumHeight(120);  // 增加最小高度
    controlGroup_->setMaximumHeight(140);  // 增加最大高度
    controlLayout_ = new QHBoxLayout(controlGroup_);
    controlLayout_->setSpacing(12);
    controlLayout_->setContentsMargins(16, 16, 16, 16);

    // 图表类型选择
    chartTypeLabel_ = new QLabel("图表类型:");
    chartTypeCombo_ = new QComboBox();
    chartTypeCombo_->setMinimumWidth(120);
    chartTypeCombo_->addItem("饼图");
    chartTypeCombo_->addItem("柱状图");
    chartTypeCombo_->addItem("折线图");
    chartTypeCombo_->addItem("环形图");
    chartTypeCombo_->addItem("面积图");
    
    // 设置下拉框样式，确保选中项文字可见
    chartTypeCombo_->setStyleSheet(
        "QComboBox {"
        "    padding: 8px;"
        "    border: 2px solid #e1e8ed;"
        "    border-radius: 6px;"
        "    background: white;"
        "    color: #2c3e50;"
        "    min-width: 100px;"
        "}"
        "QComboBox:focus {"
        "    border-color: #3498db;"
        "}"
        "QComboBox::drop-down {"
        "    border: none;"
        "    width: 20px;"
        "}"
        "QComboBox::down-arrow {"
        "    width: 12px;"
        "    height: 12px;"
        "}"
        "QComboBox QAbstractItemView {"
        "    background: white;"
        "    border: 2px solid #e1e8ed;"
        "    border-radius: 6px;"
        "    selection-background-color: #3498db;"
        "    selection-color: white;"
        "    color: #2c3e50;"
        "    padding: 4px;"
        "}"
        "QComboBox QAbstractItemView::item {"
        "    padding: 8px;"
        "    border: none;"
        "    border-radius: 4px;"
        "    color: #2c3e50;"
        "}"
        "QComboBox QAbstractItemView::item:selected {"
        "    background: #3498db;"
        "    color: white;"
        "}"
        "QComboBox QAbstractItemView::item:hover {"
        "    background: #e3f2fd;"
        "    color: #2c3e50;"
        "}"
    );
    
    // 日期范围
    dateRangeLabel_ = new QLabel("日期范围:");
    startDateEdit_ = new QDateEdit(QDate::currentDate().addDays(-30));
    startDateEdit_->setCalendarPopup(true);
    startDateEdit_->setMinimumWidth(130);
    endDateEdit_ = new QDateEdit(QDate::currentDate());
    endDateEdit_->setCalendarPopup(true);
    endDateEdit_->setMinimumWidth(130);
    
    // 操作按钮
    refreshButton_ = new QPushButton("刷新");
    refreshButton_->setMinimumWidth(90);
    exportButton_ = new QPushButton("导出");
    exportButton_->setMinimumWidth(90);
    
    // 自动刷新
    autoRefreshCheck_ = new QCheckBox("自动刷新");
    refreshIntervalLabel_ = new QLabel("间隔(秒):");
    refreshIntervalSpin_ = new QSpinBox();
    refreshIntervalSpin_->setRange(5, 300);
    refreshIntervalSpin_->setValue(30);
    refreshIntervalSpin_->setEnabled(false);
    refreshIntervalSpin_->setMinimumWidth(70);

    // 设置控件最小宽度，确保布局美观
    chartTypeCombo_->setMinimumWidth(130);
    chartTypeCombo_->setMaximumWidth(160);
    startDateEdit_->setMinimumWidth(130);
    startDateEdit_->setMaximumWidth(150);
    endDateEdit_->setMinimumWidth(130);
    endDateEdit_->setMaximumWidth(150);
    refreshButton_->setMinimumWidth(90);
    refreshButton_->setMaximumWidth(110);
    exportButton_->setMinimumWidth(90);
    exportButton_->setMaximumWidth(110);
    refreshIntervalSpin_->setMinimumWidth(70);
    refreshIntervalSpin_->setMaximumWidth(90);

    controlLayout_->addWidget(chartTypeLabel_);
    controlLayout_->addWidget(chartTypeCombo_);
    controlLayout_->addSpacing(20);
    controlLayout_->addWidget(dateRangeLabel_);
    controlLayout_->addWidget(startDateEdit_);
    controlLayout_->addWidget(new QLabel("至"));
    controlLayout_->addWidget(endDateEdit_);
    controlLayout_->addSpacing(20);
    controlLayout_->addWidget(refreshButton_);
    controlLayout_->addWidget(exportButton_);
    controlLayout_->addSpacing(20);
    controlLayout_->addWidget(autoRefreshCheck_);
    controlLayout_->addWidget(refreshIntervalLabel_);
    controlLayout_->addWidget(refreshIntervalSpin_);
    controlLayout_->addStretch();
    
    mainLayout_->addWidget(controlGroup_);
    
    // 图表区域
    chartFrame_ = new QFrame();
    chartFrame_->setObjectName("chartWidget");
    chartFrame_->setFrameStyle(QFrame::Box | QFrame::Raised);
    chartFrame_->setMinimumSize(700, 500);
    chartFrame_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    chartFrame_->setAttribute(Qt::WA_PaintOnScreen, false);
    chartFrame_->setAttribute(Qt::WA_OpaquePaintEvent, false);
    
    chartLayout_ = new QVBoxLayout(chartFrame_);
    chartLayout_->setContentsMargins(16, 16, 16, 16);
    
    // 图表标题
    chartTitleLabel_ = new QLabel("HR数据统计图表");
    chartTitleLabel_->setAlignment(Qt::AlignCenter);
    chartTitleLabel_->setStyleSheet("font-size: 18px; font-weight: bold; color: #2c3e50; margin-bottom: 6px;");
    
    chartSubtitleLabel_ = new QLabel("问答数据按类别分布");
    chartSubtitleLabel_->setAlignment(Qt::AlignCenter);
    chartSubtitleLabel_->setStyleSheet("font-size: 14px; color: #7f8c8d; margin-bottom: 8px;");
    
    chartLayout_->addWidget(chartTitleLabel_);
    chartLayout_->addWidget(chartSubtitleLabel_);
    
    // 创建图表绘制区域
    chartDrawArea_ = new QWidget();
    chartDrawArea_->installEventFilter(this); // 确保首次显示时能触发重绘
    chartDrawArea_->setMinimumSize(700, 420);
    chartDrawArea_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    chartDrawArea_->setStyleSheet("QWidget { background: white; border: 1px solid #e1e8ed; border-radius: 4px; }");
    chartLayout_->addWidget(chartDrawArea_);
    chartLayout_->setStretch(0, 0);   // 标题
    chartLayout_->setStretch(1, 0);   // 副标题
    chartLayout_->setStretch(2, 1);   // 绘制区域最大化
    
    mainLayout_->addWidget(chartFrame_);
    
    // 创建定时器
    animationTimer_ = new QTimer(this);
    animationTimer_->setInterval(50); // 20 FPS
    
    autoRefreshTimer_ = new QTimer(this);
}

void ChartWidget::setupConnections()
{
    connect(chartTypeCombo_, static_cast<void(QComboBox::*)(const QString&)>(&QComboBox::currentTextChanged),
            this, &ChartWidget::onChartTypeChanged);
    connect(startDateEdit_, &QDateEdit::dateChanged, this, &ChartWidget::onDateRangeChanged);
    connect(endDateEdit_, &QDateEdit::dateChanged, this, &ChartWidget::onDateRangeChanged);
    connect(refreshButton_, &QPushButton::clicked, this, &ChartWidget::onRefreshClicked);
    connect(exportButton_, &QPushButton::clicked, this, &ChartWidget::onExportClicked);
    connect(autoRefreshCheck_, &QCheckBox::toggled, this, &ChartWidget::onAutoRefreshChanged);
    connect(animationTimer_, &QTimer::timeout, this, &ChartWidget::onAnimationTimer);
    
    connect(autoRefreshTimer_, &QTimer::timeout, this, &ChartWidget::refreshCharts);
}

void ChartWidget::applyStyles()
{
    setStyleSheet(
        "QGroupBox {"
        "    font-weight: bold;"
        "    border: 2px solid #e1e8ed;"
        "    border-radius: 8px;"
        "    margin-top: 10px;"
        "    padding-top: 10px;"
        "}"
        "QGroupBox::title {"
        "    subcontrol-origin: margin;"
        "    left: 10px;"
        "    padding: 0 5px 0 5px;"
        "    color: #2c3e50;"
        "}"
        "QFrame#chartWidget {"
        "    background: white;"
        "    border: 2px solid #e1e8ed;"
        "    border-radius: 8px;"
        "}"
    );
}

void ChartWidget::setupColors()
{
    // 设置图表颜色主题 - 使用专业的蓝色系
    // 颜色将在绘制时动态生成
}

void ChartWidget::setupFonts()
{
    // 设置图表字体
}

void ChartWidget::setupGradients()
{
    // 设置渐变效果
}

void ChartWidget::loadData()
{
    processUserData();
    processQuestionData();
    processDocumentData();
    processTimeSeriesData();
    calculateChartData();
    updateChartArea();
    
    // 确保数据已加载
    if (pieChartData_.isEmpty() && barChartData_.isEmpty() && lineChartData_.isEmpty()) {
        loadSampleData();
    }
    
    // 强制重绘
    if (chartDrawArea_) {
        chartDrawArea_->update();
    }
    update();
}

void ChartWidget::processUserData()
{
    userStats_.clear();
    
    QList<User> users = database_->getAllUsers();
    int employees = 0, admins = 0, pending = 0, active = 0;
    
    for (const User& user : users) {
        if (user.isHRAdmin()) {
            admins++;
        } else {
            employees++;
        }
        
        if (user.getStatus() == UserStatus::Pending) {
            pending++;
        } else if (user.getStatus() == UserStatus::Active) {
            active++;
        }
    }
    
    userStats_["普通员工"] = employees;
    userStats_["HR管理员"] = admins;
    userStats_["待审核"] = pending;
    userStats_["已激活"] = active;
}

void ChartWidget::processQuestionData()
{
    // 基于所选时间范围统计各类别问题数量，确保与日期筛选联动
    questionStats_.clear();
    if (!startDateEdit_ || !endDateEdit_) {
        questionStats_ = database_->getQuestionCategoryStats();
        return;
    }
    QDateTime start(QDate(startDateEdit_->date()), QTime(0, 0, 0));
    QDateTime end(QDate(endDateEdit_->date()), QTime(23, 59, 59));
    QList<QuestionRecord> records = database_->getQuestionRecordsByDateRange(start, end);
    for (const QuestionRecord& r : records) {
        questionStats_[r.getCategory()] += 1;
    }
}

void ChartWidget::processDocumentData()
{
    documentStats_.clear();
    
    QList<Document> docs = database_->getAllDocuments();
    for (const Document& doc : docs) {
        documentStats_[doc.getCategory()]++;
    }
}

void ChartWidget::processTimeSeriesData()
{
    // 处理时间序列数据
    timeSeriesData_.clear();
    
    QDateTime start = QDateTime(startDateEdit_->date(), QTime(0, 0, 0));
    QDateTime end = QDateTime(endDateEdit_->date(), QTime(23, 59, 59));
    
    QList<QuestionRecord> records = database_->getQuestionRecordsByDateRange(start, end);
    
    QMap<QString, QList<QPair<QString, int>>> dailyStats;
    
    for (const QuestionRecord& record : records) {
        QString dateStr = record.getCreateTime().date().toString("MM-dd");
        QString category = record.getCategory();
        
        if (!dailyStats.contains(category)) {
            dailyStats[category] = QList<QPair<QString, int>>();
        }
        
        // 统计每日数据
        bool found = false;
        for (auto& pair : dailyStats[category]) {
            if (pair.first == dateStr) {
                pair.second++;
                found = true;
                break;
            }
        }
        if (!found) {
            dailyStats[category].append(qMakePair(dateStr, 1));
        }
    }
    
    timeSeriesData_ = dailyStats;
}

void ChartWidget::loadSampleData()
{
    QList<QPair<QString, double>> sampleData;
    sampleData.append(qMakePair("考勤", 5.0));
    sampleData.append(qMakePair("薪酬", 3.0));
    sampleData.append(qMakePair("绩效", 2.0));
    sampleData.append(qMakePair("入职离职", 4.0));
    sampleData.append(qMakePair("员工福利", 1.0));
    
    if (currentChartType_ == "饼图" || currentChartType_ == "环形图") {
        pieChartData_ = sampleData;
    } else if (currentChartType_ == "柱状图") {
        for (const auto& pair : sampleData) {
            barChartData_.append(qMakePair(pair.first, static_cast<int>(pair.second)));
        }
    } else if (currentChartType_ == "折线图" || currentChartType_ == "面积图") {
        for (const auto& pair : sampleData) {
            lineChartData_.append(qMakePair(pair.first, static_cast<int>(pair.second)));
        }
    }
}

void ChartWidget::calculateChartData()
{
    pieChartData_.clear();
    barChartData_.clear();
    lineChartData_.clear();
    
    // 如果没有问答数据，使用文档数据作为备选
    if (questionStats_.isEmpty()) {
        // 使用文档分类统计
        QMap<QString, int> docStats;
        QList<Document> docs = database_->getAllDocuments();
        for (const Document& doc : docs) {
            docStats[doc.getCategory()]++;
        }
        
        // 根据当前图表类型准备数据
        if (currentChartType_ == "饼图" || currentChartType_ == "环形图") {
            for (auto it = docStats.begin(); it != docStats.end(); ++it) {
                pieChartData_.append(qMakePair(it.key(), static_cast<double>(it.value())));
            }
        } else if (currentChartType_ == "柱状图") {
            for (auto it = docStats.begin(); it != docStats.end(); ++it) {
                barChartData_.append(qMakePair(it.key(), it.value()));
            }
        } else if (currentChartType_ == "折线图" || currentChartType_ == "面积图") {
            for (auto it = docStats.begin(); it != docStats.end(); ++it) {
                lineChartData_.append(qMakePair(it.key(), it.value()));
            }

        }
    } else {
        // 使用问答数据
        if (currentChartType_ == "饼图" || currentChartType_ == "环形图") {
            for (auto it = questionStats_.begin(); it != questionStats_.end(); ++it) {
                pieChartData_.append(qMakePair(it.key(), static_cast<double>(it.value())));
            }
        } else if (currentChartType_ == "柱状图") {
            for (auto it = questionStats_.begin(); it != questionStats_.end(); ++it) {
                barChartData_.append(qMakePair(it.key(), it.value()));
            }
        } else if (currentChartType_ == "折线图" || currentChartType_ == "面积图") {
            for (auto it = questionStats_.begin(); it != questionStats_.end(); ++it) {
                lineChartData_.append(qMakePair(it.key(), it.value()));
            }
        }
    }
    
    // 如果仍然没有数据，添加一些示例数据
    if (pieChartData_.isEmpty() && barChartData_.isEmpty() && lineChartData_.isEmpty()) {
        QList<QPair<QString, double>> sampleData;
        sampleData.append(qMakePair("考勤", 5.0));
        sampleData.append(qMakePair("薪酬", 3.0));
        sampleData.append(qMakePair("绩效", 2.0));
        sampleData.append(qMakePair("入职离职", 4.0));
        sampleData.append(qMakePair("员工福利", 1.0));
        
        if (currentChartType_ == "饼图" || currentChartType_ == "环形图") {
            pieChartData_ = sampleData;
        } else if (currentChartType_ == "柱状图") {
            for (const auto& pair : sampleData) {
                barChartData_.append(qMakePair(pair.first, static_cast<int>(pair.second)));
            }
        } else if (currentChartType_ == "折线图" || currentChartType_ == "面积图") {
            for (const auto& pair : sampleData) {
                lineChartData_.append(qMakePair(pair.first, static_cast<int>(pair.second)));
            }
        }
    }
}

void ChartWidget::updateChartArea()
{
    if (chartDrawArea_) {
        QRect drawRect = chartDrawArea_->rect();
        if (drawRect.width() > 0 && drawRect.height() > 0) {
            // 增加边距以容纳饼图标签
            chartArea_ = QRect(40, 40, drawRect.width() - 80, drawRect.height() - 80);
        } else {
            // 如果绘制区域还没有正确初始化，使用默认值
            chartArea_ = QRect(40, 40, 420, 220);
        }
    }
}

void ChartWidget::paintEvent(QPaintEvent* event)
{
    QWidget::paintEvent(event);
    
    // 确保有数据可显示
    if (pieChartData_.isEmpty() && barChartData_.isEmpty() && lineChartData_.isEmpty()) {
        loadSampleData();
    }
    
    // 在图表绘制区域绘制
    if (chartDrawArea_) {
        QPainter painter(chartDrawArea_);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setRenderHint(QPainter::TextAntialiasing, true);
        
        // 更新图表区域
        updateChartArea();
        
        // 绘制图表
        drawChart(painter);
        
        // 立即更新绘制区域
        // 不要在paintEvent里调用update()，避免循环重绘
    }
}

bool ChartWidget::eventFilter(QObject* obj, QEvent* event)
{
    if (obj == chartDrawArea_) {
        if (event->type() == QEvent::Show || event->type() == QEvent::Resize) {
            updateChartArea();
            update();
        } else if (event->type() == QEvent::Paint) {
            // 确保即使没有外部触发也能绘制
            QPainter painter(chartDrawArea_);
            painter.setRenderHint(QPainter::Antialiasing, true);
            painter.setRenderHint(QPainter::TextAntialiasing, true);
            updateChartArea();
            drawChart(painter);
            return true;
        }
    }
    return QWidget::eventFilter(obj, event);
}

void ChartWidget::drawChart(QPainter& painter)
{
    // 确保有数据可显示
    if (pieChartData_.isEmpty() && barChartData_.isEmpty() && lineChartData_.isEmpty()) {
        // 强制加载示例数据
        loadSampleData();
    }
    
    // 再次检查数据
    if (pieChartData_.isEmpty() && barChartData_.isEmpty() && lineChartData_.isEmpty()) {
        // 绘制占位图（灰色矩形），保证页面有可见结果
        painter.setBrush(QColor("#ecf0f1"));
        painter.setPen(QPen(QColor("#bdc3c7"), 2));
        painter.drawRect(chartArea_.adjusted(10, 10, -10, -10));
        painter.setPen(QColor("#7f8c8d"));
        painter.setFont(QFont("Microsoft YaHei", 16));
        painter.drawText(chartArea_, Qt::AlignCenter, "暂无可显示的数据\n(请尝试更换时间范围或点击刷新)");
        return;
    }
    
    // 根据图表类型绘制
    if (currentChartType_ == "饼图") {
        drawPieChart(painter);
    } else if (currentChartType_ == "柱状图") {
        drawBarChart(painter);
    } else if (currentChartType_ == "折线图") {
        drawLineChart(painter);
    } else if (currentChartType_ == "环形图") {
        drawDoughnutChart(painter);
    } else if (currentChartType_ == "面积图") {
        drawAreaChart(painter);
    }
    
    // 绘制图例
    drawLegend(painter);
    
    // 绘制标题
    drawTitle(painter);
}

void ChartWidget::drawPieChart(QPainter& painter)
{
    if (pieChartData_.isEmpty()) return;
    
    // 计算总值
    double total = 0;
    for (const auto& pair : pieChartData_) {
        total += pair.second;
    }
    
    if (total <= 0) return;
    
    // 计算饼图位置和大小
    QRect pieRect = chartArea_;
    int diameter = qMin(pieRect.width(), pieRect.height()) - 60; // 放大可用半径，给标签留边
    QRect drawRect(pieRect.center().x() - diameter/2, 
                   pieRect.center().y() - diameter/2,
                   diameter, diameter);
    
    // 绘制饼图扇形
    int startAngle = 0;
    
    for (int i = 0; i < pieChartData_.size(); ++i) {
        const auto& pair = pieChartData_[i];
        double percentage = pair.second / total;
        int spanAngle = static_cast<int>(percentage * 360 * 16); // Qt uses 1/16 degrees
        
        // 获取颜色
        QColor color = getChartColor(i);
        
        // 创建渐变
        QRadialGradient gradient = createPieGradient(drawRect, color);
        painter.setBrush(QBrush(gradient));
        painter.setPen(QPen(Qt::white, 2));
        
        // 绘制扇形
        painter.drawPie(drawRect, startAngle, spanAngle);
        
        // 绘制标签
        double labelAngle = (startAngle + spanAngle / 2) / 16.0 * M_PI / 180.0;
        
        // 计算扇形边缘点
        QPoint edgePoint = calculatePointOnCircle(drawRect.center(), diameter / 2, labelAngle);
        
        // 设置字体和测量文本
        painter.setFont(QFont("Microsoft YaHei", 9, QFont::Bold));
        QString labelText = QString("%1 %2%")
                           .arg(pair.first)
                           .arg(QString::number(percentage * 100, 'f', 1));
        
        QFontMetrics fm(painter.font());
        QRect textBounds = fm.boundingRect(labelText);
        
        // 计算标签的最佳位置 - 基于象限分布
        int labelRadius = diameter / 2 + 40; // 增加距离以避免重叠
        QPoint idealLabelPos(drawRect.center().x() + labelRadius * cos(labelAngle),
                            drawRect.center().y() + labelRadius * sin(labelAngle));
        
        // 根据角度调整标签位置，避免边缘裁切
        int textWidth = textBounds.width() + 8;
        int textHeight = textBounds.height() + 8;
        
        QPoint labelPos = idealLabelPos;
        
        // 右侧标签
        if (cos(labelAngle) > 0.5) {
            labelPos.setX(idealLabelPos.x());
        }
        // 左侧标签
        else if (cos(labelAngle) < -0.5) {
            labelPos.setX(idealLabelPos.x() - textWidth);
        }
        // 中间标签
        else {
            labelPos.setX(idealLabelPos.x() - textWidth / 2);
        }
        
        // 上下位置调整
        if (sin(labelAngle) < 0) {
            labelPos.setY(idealLabelPos.y() - textHeight / 2);
        } else {
            labelPos.setY(idealLabelPos.y() - textHeight / 2);
        }
        
        // 确保标签在绘制区域内
        QRect chartBounds = chartDrawArea_->rect().adjusted(5, 5, -5, -5);
        labelPos.setX(qMax(chartBounds.left(), 
                          qMin(labelPos.x(), chartBounds.right() - textWidth)));
        labelPos.setY(qMax(chartBounds.top(),
                          qMin(labelPos.y(), chartBounds.bottom() - textHeight)));
        
        QRect labelRect(labelPos.x(), labelPos.y(), textWidth, textHeight);
        
        // 绘制连接线
        painter.setPen(QPen(QColor("#bdc3c7"), 1, Qt::SolidLine));
        QPoint lineEndPoint(labelRect.center().x(), labelRect.center().y());
        
        // 绘制从扇形边缘到标签的连接线
        painter.drawLine(edgePoint, lineEndPoint);
        
        // 绘制标签背景
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(255, 255, 255, 220));
        painter.drawRoundedRect(labelRect, 3, 3);
        
        // 绘制标签文本
        painter.setPen(QColor("#2c3e50"));
        painter.setBrush(Qt::NoBrush);
        painter.drawText(labelRect, Qt::AlignCenter, labelText);
        
        startAngle += spanAngle;
    }
}

void ChartWidget::drawBarChart(QPainter& painter)
{
    if (barChartData_.isEmpty()) return;
    
    // 计算最大值
    int maxValue = 0;
    for (const auto& pair : barChartData_) {
        maxValue = qMax(maxValue, pair.second);
    }
    
    if (maxValue <= 0) return;
    
    // 绘制坐标轴
    drawAxis(painter);
    
    // 计算柱状图参数
    int barCount = barChartData_.size();
    int barWidth = qMax(20, (chartArea_.width() - 120) / barCount - 8);
    int chartHeight = chartArea_.height() - 60;
    
    // 绘制柱状图
    for (int i = 0; i < barChartData_.size(); ++i) {
        const auto& pair = barChartData_[i];
        
        int barHeight = static_cast<int>((double)pair.second / maxValue * chartHeight);
        int barX = chartArea_.x() + 50 + i * (barWidth + 10);
        int barY = chartArea_.bottom() - 30 - barHeight;
        
        QRect barRect(barX, barY, barWidth, barHeight);
        
        // 获取颜色和渐变
        QColor color = getChartColor(i);
        QLinearGradient gradient = createBarGradient(barRect, color);
        
        painter.setBrush(QBrush(gradient));
        painter.setPen(QPen(color.darker(120), 1));
        painter.drawRect(barRect);
        
        // 绘制数值标签（自适应宽度，避免被裁）
        painter.setPen(QColor("#2c3e50"));
        painter.setFont(QFont("Microsoft YaHei", 10));
        QString valueText = QString::number(pair.second);
        QFontMetrics fmNum(painter.font());
        int numWidth = fmNum.horizontalAdvance(valueText);
        int textX = barRect.x() + (barRect.width() - numWidth) / 2;
        painter.drawText(textX, barRect.y() - 6, valueText);
        
        // 绘制类别标签
        QRect labelRect(barX, chartArea_.bottom() - 28, barWidth, 24);
        QString cat = pair.first;
        QFontMetrics fmCat(painter.font());
        if (fmCat.horizontalAdvance(cat) > barWidth) {
            cat = fmCat.elidedText(cat, Qt::ElideRight, barWidth - 6);
        }
        painter.drawText(labelRect, Qt::AlignCenter, cat);
    }
}

void ChartWidget::drawLineChart(QPainter& painter)
{
    if (lineChartData_.isEmpty()) return;
    
    // 绘制坐标轴
    drawAxis(painter);
    drawGrid(painter);
    
    // 计算最大值
    int maxValue = 0;
    for (const auto& pair : lineChartData_) {
        maxValue = qMax(maxValue, pair.second);
    }
    
    if (maxValue <= 0) return;
    
    // 计算线图参数
    int chartWidth = chartArea_.width() - 120;
    int chartHeight = chartArea_.height() - 60;
    
    // 准备点
    QList<QPoint> points;
    for (int i = 0; i < lineChartData_.size(); ++i) {
        const auto& pair = lineChartData_[i];
        
        int pointX = chartArea_.x() + 50 + i * chartWidth / (lineChartData_.size() - 1);
        int pointY = chartArea_.bottom() - 30 - 
                    static_cast<int>((double)pair.second / maxValue * chartHeight);
        
        points.append(QPoint(pointX, pointY));
    }
    
    // 绘制线条
    painter.setPen(QPen(QColor("#3498db"), 3));
    for (int i = 0; i < points.size() - 1; ++i) {
        painter.drawLine(points[i], points[i + 1]);
    }
    
    // 绘制数据点
    for (int i = 0; i < points.size(); ++i) {
        QColor color = getChartColor(i);
        painter.setBrush(color);
        painter.setPen(QPen(color.darker(120), 2));
        painter.drawEllipse(points[i], 6, 6);
        
        // 绘制数值标签
        painter.setPen(QColor("#2c3e50"));
        painter.setFont(QFont("Microsoft YaHei", 9));
        painter.drawText(points[i].x() - 10, points[i].y() - 10, 
                        QString::number(lineChartData_[i].second));
    }
}

void ChartWidget::drawDoughnutChart(QPainter& painter)
{
    drawPieChart(painter); // 简化实现，使用饼图逻辑
    
    // 在中心绘制一个白色圆圈创建环形效果
    QRect pieRect = chartArea_;
    int diameter = qMin(pieRect.width(), pieRect.height()) - 100;
    QRect drawRect(pieRect.center().x() - diameter/2, 
                   pieRect.center().y() - diameter/2,
                   diameter, diameter);
    
    int innerDiameter = diameter / 2;
    QRect innerRect(drawRect.center().x() - innerDiameter/2,
                   drawRect.center().y() - innerDiameter/2,
                   innerDiameter, innerDiameter);
    
    painter.setBrush(Qt::white);
    painter.setPen(QPen(Qt::white, 2));
    painter.drawEllipse(innerRect);
}

void ChartWidget::drawAreaChart(QPainter& painter)
{
    if (lineChartData_.isEmpty()) return;
    
    // 绘制坐标轴和网格
    drawAxis(painter);
    drawGrid(painter);
    
    // 计算最大值
    int maxValue = 0;
    for (const auto& pair : lineChartData_) {
        maxValue = qMax(maxValue, pair.second);
    }
    
    if (maxValue <= 0) return;
    
    // 计算面积图参数
    int chartWidth = chartArea_.width() - 120;
    int chartHeight = chartArea_.height() - 60;
    int baselineY = chartArea_.bottom() - 30; // 底部基准线
    
    // 准备点列表
    QList<QPoint> points;
    
    // 添加起始点（左下角）
    points.append(QPoint(chartArea_.x() + 50, baselineY));
    
    // 添加数据点
    for (int i = 0; i < lineChartData_.size(); ++i) {
        const auto& pair = lineChartData_[i];
        
        int pointX = chartArea_.x() + 50 + i * chartWidth / qMax(1, lineChartData_.size() - 1);
        int pointY = baselineY - static_cast<int>((double)pair.second / maxValue * chartHeight);
        
        points.append(QPoint(pointX, pointY));
    }
    
    // 添加结束点（右下角）
    if (!points.isEmpty()) {
        QPoint lastDataPoint = points.last();
        points.append(QPoint(lastDataPoint.x(), baselineY));
    }
    
    // 绘制面积填充
    if (points.size() >= 3) {
        QPolygon polygon(points);
        
        // 创建渐变填充
        QLinearGradient gradient(chartArea_.topLeft(), QPoint(chartArea_.left(), baselineY));
        gradient.setColorAt(0, QColor(52, 152, 219, 120)); // 上部更深
        gradient.setColorAt(1, QColor(52, 152, 219, 30));  // 下部更淡
        
        painter.setBrush(QBrush(gradient));
        painter.setPen(Qt::NoPen);
        painter.drawPolygon(polygon);
    }
    
    // 绘制顶部的线条（去掉起始和结束的基准点）
    if (points.size() >= 3) {
        painter.setPen(QPen(QColor("#3498db"), 3, Qt::SolidLine));
        for (int i = 1; i < points.size() - 2; ++i) {
            painter.drawLine(points[i], points[i + 1]);
        }
    }
    
    // 绘制数据点和标签（只绘制数据点，不包括基准点）
    painter.setFont(QFont("Microsoft YaHei", 9));
    for (int i = 1; i < points.size() - 1; ++i) { // 跳过第一个和最后一个基准点
        QColor color = getChartColor(i - 1);
        
        // 绘制数据点
        painter.setBrush(color);
        painter.setPen(QPen(color.darker(120), 2));
        painter.drawEllipse(points[i], 6, 6);
        
        // 绘制数值标签
        painter.setPen(QColor("#2c3e50"));
        QString valueText = QString::number(lineChartData_[i - 1].second);
        QFontMetrics fm(painter.font());
        int textWidth = fm.horizontalAdvance(valueText);
        painter.drawText(points[i].x() - textWidth/2, points[i].y() - 10, valueText);
    }
    
    // 绘制X轴标签
    painter.setPen(QColor("#2c3e50"));
    for (int i = 0; i < lineChartData_.size(); ++i) {
        const auto& pair = lineChartData_[i];
        int labelX = chartArea_.x() + 50 + i * chartWidth / qMax(1, lineChartData_.size() - 1);
        int labelY = baselineY + 20;
        
        QFontMetrics fm(painter.font());
        QString labelText = pair.first;
        int textWidth = fm.horizontalAdvance(labelText);
        
        // 绘制标签，居中对齐
        painter.drawText(labelX - textWidth/2, labelY, labelText);
    }
}

void ChartWidget::drawLegend(QPainter& painter)
{
    // 绘制图例
    int legendX = chartArea_.right() - 150;
    int legendY = chartArea_.y() + 20;
    int legendItemHeight = 20;
    
    painter.setFont(QFont("Microsoft YaHei", 10));
    
    QStringList labels;
    if (currentChartType_ == "饼图" || currentChartType_ == "环形图") {
        for (const auto& pair : pieChartData_) {
            labels.append(pair.first);
        }
    } else {
        for (const auto& pair : barChartData_) {
            labels.append(pair.first);
        }
        for (const auto& pair : lineChartData_) {
            if (!labels.contains(pair.first)) {
                labels.append(pair.first);
            }
        }
    }
    
    for (int i = 0; i < labels.size(); ++i) {
        QColor color = getChartColor(i);
        
        // 绘制颜色块
        QRect colorRect(legendX, legendY + i * legendItemHeight, 15, 15);
        painter.setBrush(color);
        painter.setPen(QPen(color.darker(120), 1));
        painter.drawRect(colorRect);
        
        // 绘制文字
        painter.setPen(QColor("#2c3e50"));
        painter.drawText(legendX + 20, legendY + i * legendItemHeight + 12, labels[i]);
    }
}

void ChartWidget::drawTitle(QPainter& painter)
{
    Q_UNUSED(painter)
    // 标题已经通过QLabel绘制
}

void ChartWidget::drawGrid(QPainter& painter)
{
    painter.setPen(QPen(QColor("#ecf0f1"), 1, Qt::DotLine));
    
    // 绘制水平网格线
    for (int i = 1; i < 10; ++i) {
        int y = chartArea_.y() + 30 + i * (chartArea_.height() - 60) / 10;
        painter.drawLine(chartArea_.x() + 50, y, chartArea_.right() - 50, y);
    }
    
    // 绘制垂直网格线（如果需要）
    if (currentChartType_ == "折线图" || currentChartType_ == "面积图") {
        int count = qMax(barChartData_.size(), lineChartData_.size());
        for (int i = 0; i <= count; ++i) {
            int x = chartArea_.x() + 50 + i * (chartArea_.width() - 100) / count;
            painter.drawLine(x, chartArea_.y() + 30, x, chartArea_.bottom() - 30);
        }
    }
}

void ChartWidget::drawAxis(QPainter& painter)
{
    painter.setPen(QPen(QColor("#2c3e50"), 2));
    
    // X轴
    painter.drawLine(chartArea_.x() + 50, chartArea_.bottom() - 30, 
                    chartArea_.right() - 50, chartArea_.bottom() - 30);
    
    // Y轴
    painter.drawLine(chartArea_.x() + 50, chartArea_.y() + 30,
                    chartArea_.x() + 50, chartArea_.bottom() - 30);
}

void ChartWidget::drawDataLabels(QPainter& painter)
{
    Q_UNUSED(painter)
    // 数据标签已经在各个图表绘制函数中处理
}

QColor ChartWidget::getChartColor(int index)
{
    // 专业的颜色配色方案
    QList<QColor> colors = {
        QColor("#3498db"), // 蓝色
        QColor("#e74c3c"), // 红色
        QColor("#2ecc71"), // 绿色
        QColor("#f39c12"), // 橙色
        QColor("#9b59b6"), // 紫色
        QColor("#1abc9c"), // 青色
        QColor("#34495e"), // 深蓝灰
        QColor("#e67e22"), // 胡萝卜色
        QColor("#95a5a6"), // 灰色
        QColor("#d35400")  // 南瓜色
    };
    
    return colors[index % colors.size()];
}

QLinearGradient ChartWidget::createBarGradient(const QRect& rect, const QColor& color)
{
    QLinearGradient gradient(rect.topLeft(), rect.bottomLeft());
    gradient.setColorAt(0, color.lighter(120));
    gradient.setColorAt(1, color.darker(110));
    return gradient;
}

QRadialGradient ChartWidget::createPieGradient(const QRect& rect, const QColor& color)
{
    QRadialGradient gradient(rect.center(), rect.width() / 2);
    gradient.setColorAt(0, color.lighter(130));
    gradient.setColorAt(1, color.darker(110));
    return gradient;
}

// 槽函数实现
void ChartWidget::onChartTypeChanged(const QString& chartType)
{
    currentChartType_ = chartType;
    
    // 更新副标题
    if (chartType == "饼图" || chartType == "环形图") {
        chartSubtitleLabel_->setText("问答数据按类别分布");
    } else if (chartType == "柱状图") {
        chartSubtitleLabel_->setText("各类别问题数量对比");
    } else if (chartType == "折线图") {
        chartSubtitleLabel_->setText("问题趋势分析");
    } else if (chartType == "面积图") {
        chartSubtitleLabel_->setText("累积问题数据");
    }
    
    calculateChartData();
    startAnimation();
}

void ChartWidget::onDateRangeChanged()
{
    processTimeSeriesData();
    calculateChartData();
    update();
}

void ChartWidget::onRefreshClicked()
{
    refreshCharts();
}

void ChartWidget::onExportClicked()
{
    exportCharts();
}

void ChartWidget::onAutoRefreshChanged(bool checked)
{
    refreshIntervalSpin_->setEnabled(checked);
    
    if (checked) {
        autoRefreshTimer_->setInterval(refreshIntervalSpin_->value() * 1000);
        autoRefreshTimer_->start();
    } else {
        autoRefreshTimer_->stop();
    }
}

void ChartWidget::onAnimationTimer()
{
    if (isAnimating_) {
        animationStep_++;
        update();
        
        if (animationStep_ >= 20) { // 1秒动画
            isAnimating_ = false;
            animationTimer_->stop();
        }
    }
}

void ChartWidget::refreshCharts()
{
    loadData();
    calculateChartData();
    updateChartArea();
    update();
    emit chartDataChanged();
}

void ChartWidget::exportCharts()
{
    QString defaultName = QString("chart_%1").arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"));
    QString selected = QFileDialog::getSaveFileName(
        this,
        "导出图表",
        defaultName,
        "PNG图片 (*.png);;PDF文档 (*.pdf)"
    );

    if (selected.isEmpty()) return;

    bool ok = false;
    if (selected.endsWith(".png", Qt::CaseInsensitive)) {
        ok = exportToImage(selected);
    } else if (selected.endsWith(".pdf", Qt::CaseInsensitive)) {
        ok = exportToPDF(selected);
    } else {
        // 若用户未加后缀，默认导出为PNG
        selected += ".png";
        ok = exportToImage(selected);
    }

    if (ok) {
        QMessageBox::information(this, "导出成功", QString("图表已导出到：%1").arg(selected));
    } else {
        QMessageBox::warning(this, "导出失败", QString("无法导出到：%1").arg(selected));
    }
}

bool ChartWidget::exportToImage(const QString& filePath)
{
    if (filePath.isEmpty()) return false;
    QPixmap pixmap(size() * 2);
    pixmap.fill(Qt::white);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::TextAntialiasing, true);
    painter.scale(2.0, 2.0);
    render(&painter);

    return pixmap.save(filePath, "PNG");
}

bool ChartWidget::exportToPDF(const QString& filePath)
{
    if (filePath.isEmpty()) return false;
    QPdfWriter pdfWriter(filePath);
    pdfWriter.setPageSize(QPageSize::A4);
    pdfWriter.setPageMargins(QMarginsF(20, 20, 20, 20));
    pdfWriter.setTitle("HR数据统计图表");
    pdfWriter.setCreator("HR智能问答系统");

    QPainter painter(&pdfWriter);
    if (!painter.isActive()) return false;

    QRectF pageRect = pdfWriter.pageLayout().paintRectPixels(pdfWriter.resolution());
    QSizeF chartSize = size();
    double scaleX = pageRect.width() / chartSize.width();
    double scaleY = pageRect.height() / chartSize.height();
    double scale = qMin(scaleX, scaleY) * 0.8;
    painter.scale(scale, scale);
    render(&painter);
    return true;
}

void ChartWidget::startAnimation()
{
    if (!isAnimating_) {
        animationStep_ = 0;
        isAnimating_ = true;
        animationTimer_->start();
    }
}

void ChartWidget::stopAnimation()
{
    isAnimating_ = false;
    animationTimer_->stop();
}

void ChartWidget::updateAnimation()
{
    // 动画更新逻辑
}

// 鼠标事件处理
void ChartWidget::mousePressEvent(QMouseEvent* event)
{
    lastMousePos_ = event->pos();
    isDragging_ = true;
    
    handleMouseClick(event->pos());
    
    QWidget::mousePressEvent(event);
}

void ChartWidget::mouseMoveEvent(QMouseEvent* event)
{
    if (isDragging_) {
        QPoint delta = event->pos() - lastMousePos_;
        panOffset_ += delta;
        lastMousePos_ = event->pos();
        update();
    } else {
        handleMouseHover(event->pos());
    }
    
    QWidget::mouseMoveEvent(event);
}

void ChartWidget::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    updateChartArea();
    update();
}

void ChartWidget::handleMouseClick(const QPoint& pos)
{
    // 处理鼠标点击，用于交互
    if (currentChartType_ == "饼图" || currentChartType_ == "环形图") {
        int slice = getSliceAtPoint(pos);
        if (slice >= 0) {
            // 处理饼图扇形点击
        }
    }
}

void ChartWidget::handleMouseHover(const QPoint& pos)
{
    // 处理鼠标悬停，更新工具提示
    updateTooltip(pos);
}

void ChartWidget::updateTooltip(const QPoint& pos)
{
    // 更新工具提示
    if (isPointInChartArea(pos)) {
        QString tooltip = "图表数据";
        setToolTip(tooltip);
    } else {
        setToolTip("");
    }
}

bool ChartWidget::isPointInChartArea(const QPoint& pos)
{
    return chartArea_.contains(pos);
}

int ChartWidget::getSliceAtPoint(const QPoint& pos)
{
    Q_UNUSED(pos)
    // 计算点击的饼图扇形
    return -1; // 简化实现
}

int ChartWidget::getBarAtPoint(const QPoint& pos)
{
    Q_UNUSED(pos)
    // 计算点击的柱状图柱子
    return -1; // 简化实现
}

int ChartWidget::getPointAtPoint(const QPoint& pos)
{
    Q_UNUSED(pos)
    // 计算点击的折线图点
    return -1; // 简化实现
}

QString ChartWidget::formatNumber(int number)
{
    return QString::number(number);
}

QString ChartWidget::formatPercentage(double percentage)
{
    return QString::number(percentage * 100, 'f', 1) + "%";
}

QRect ChartWidget::calculateChartRect()
{
    return chartArea_;
}

// 添加缺失的函数实现
void ChartWidget::animatePieChart(QPainter& painter)
{
    Q_UNUSED(painter)
    // 饼图动画实现（简化版本）
}

void ChartWidget::animateBarChart(QPainter& painter)
{
    Q_UNUSED(painter)
    // 柱状图动画实现（简化版本）
}

void ChartWidget::animateLineChart(QPainter& painter)
{
    Q_UNUSED(painter)
    // 折线图动画实现（简化版本）
}

void ChartWidget::updateStatistics()
{
    // 更新统计信息
    processUserData();
    processQuestionData();
    processDocumentData();
    calculateChartData();
}

void ChartWidget::saveChartData()
{
    // 保存图表数据（简化实现）
    QSettings settings;
    settings.setValue("chart/lastType", currentChartType_);
    settings.setValue("chart/lastUpdate", QDateTime::currentDateTime());
}

double ChartWidget::calculateAngle(const QPoint& center, const QPoint& point)
{
    Q_UNUSED(center)
    Q_UNUSED(point)
    // 计算角度（简化实现）
    return 0.0;
}

QPoint ChartWidget::calculatePointOnCircle(const QPoint& center, double radius, double angle)
{
    // 计算圆上点的位置
    int x = center.x() + static_cast<int>(radius * cos(angle));
    int y = center.y() + static_cast<int>(radius * sin(angle));
    return QPoint(x, y);
}

