#include "weatherwidget.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QProcessEnvironment>
#include <QNetworkRequest>
#include <QUrlQuery>
#include <QHeaderView>

WeatherWidget::WeatherWidget(QWidget* parent)
    : QWidget(parent)
    , iconLabel_(nullptr)
    , tempLabel_(nullptr)
    , descLabel_(nullptr)
    , cityLabel_(nullptr)
    , updateTimeLabel_(nullptr)
    , refreshBtn_(nullptr)
    , searchEdit_(nullptr)
    , searchBtn_(nullptr)
    , modeCombo_(nullptr)
    , stacked_(nullptr)
    , hourlyTable_(nullptr)
    , dailyTable_(nullptr)
    , timer_(new QTimer(this))
    , nam_(new QNetworkAccessManager(this))
    , geoNam_(new QNetworkAccessManager(this))
    , geoSearchNam_(new QNetworkAccessManager(this))
    , lat_(29.5630)   // 默认：重庆
    , lon_(106.5516)
{
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    bool okLat=false, okLon=false;
    double envLat = env.value("WEATHER_LAT").toDouble(&okLat);
    double envLon = env.value("WEATHER_LON").toDouble(&okLon);
    if (okLat && okLon) { lat_ = envLat; lon_ = envLon; }
    cityName_ = env.value("WEATHER_CITY", "重庆 · 中国");

    setupUI();
    applyCardStyle();

    connect(timer_, &QTimer::timeout, this, &WeatherWidget::refreshWeather);
    connect(nam_, &QNetworkAccessManager::finished, this, &WeatherWidget::onReplyFinished);
    connect(geoNam_, &QNetworkAccessManager::finished, this, &WeatherWidget::onGeoReplyFinished);
    connect(geoSearchNam_, &QNetworkAccessManager::finished, this, &WeatherWidget::onGeoSearchFinished);
    timer_->start(30 * 60 * 1000); // 30分钟
    QTimer::singleShot(100, this, &WeatherWidget::refreshWeather);
}

WeatherWidget::~WeatherWidget() {}

void WeatherWidget::setCoordinate(double lat, double lon, const QString& cityDisplayName)
{
    lat_ = lat; lon_ = lon; if (!cityDisplayName.isEmpty()) cityName_ = cityDisplayName;
    refreshWeather();
}

void WeatherWidget::setupUI()
{
    // 作为悬浮微件：无边框、圆角投影
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground, true);

    // 顶部尽量紧凑，避免影响原页面
    searchEdit_ = new QLineEdit();
    searchEdit_->setPlaceholderText("城市...");
    searchEdit_->setMaximumWidth(160);
    searchBtn_ = new QPushButton("搜");
    searchBtn_->setFixedSize(32, 24);
    searchBtn_->setStyleSheet("QPushButton{background:#10b981;color:white;border:none;border-radius:4px;padding:0 6px;} QPushButton:hover{background:#059669;}");
    modeCombo_ = new QComboBox();
    modeCombo_->addItems({"实时","小时","7天"});
    modeCombo_->setMaximumWidth(80);

    QHBoxLayout* header = new QHBoxLayout();
    header->setContentsMargins(0,0,0,0);
    header->setSpacing(6);
    header->addWidget(searchEdit_);
    header->addWidget(searchBtn_);
    header->addStretch();
    header->addWidget(modeCombo_);

    iconLabel_ = new QLabel();
    iconLabel_->setFixedSize(32, 32);
    tempLabel_ = new QLabel("--°C");
    tempLabel_->setStyleSheet("font-size:20px;font-weight:700;color:#1f2937;");
    descLabel_ = new QLabel("加载中...");
    descLabel_->setStyleSheet("font-size:11px;color:#6b7280;");
    cityLabel_ = new QLabel(cityName_);
    cityLabel_->setStyleSheet("font-size:12px;font-weight:600;color:#374151;");
    updateTimeLabel_ = new QLabel("");
    updateTimeLabel_->setStyleSheet("font-size:10px;color:#9ca3af;");
    refreshBtn_ = new QPushButton("刷");
    refreshBtn_->setFixedSize(28, 24);
    refreshBtn_->setStyleSheet("QPushButton{background:#3b82f6;color:white;border:none;border-radius:4px;padding:0 6px;} QPushButton:hover{background:#2563eb;}");
    connect(refreshBtn_, &QPushButton::clicked, this, &WeatherWidget::refreshWeather);

    QVBoxLayout* right = new QVBoxLayout();
    right->setContentsMargins(0,0,0,0);
    right->setSpacing(2);
    right->addWidget(cityLabel_);
    right->addWidget(tempLabel_);
    right->addWidget(descLabel_);
    right->addWidget(updateTimeLabel_);

    QHBoxLayout* row = new QHBoxLayout();
    row->setContentsMargins(8,2,8,2);
    row->setSpacing(6);
    row->addWidget(iconLabel_);
    row->addLayout(right);
    row->addStretch();
    row->addWidget(refreshBtn_);

    // 搜索结果下拉
    resultList_ = new QListWidget();
    resultList_->setVisible(false);
    resultList_->setMaximumHeight(160);
    resultList_->setStyleSheet("QListWidget{background:white;border:1px solid #e5e7eb;border-radius:6px;} QListWidget::item{padding:6px;} QListWidget::item:selected{background:#e3f2fd;}");

    // 数据表
    stacked_ = new QStackedWidget();
    hourlyTable_ = new QTableWidget();
    hourlyTable_->setColumnCount(3);
    hourlyTable_->setHorizontalHeaderLabels({"时间","温度","天气"});
    hourlyTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    hourlyTable_->verticalHeader()->setVisible(false);
    hourlyTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    hourlyTable_->setSelectionMode(QAbstractItemView::NoSelection);
    hourlyTable_->setShowGrid(false);
    dailyTable_ = new QTableWidget();
    dailyTable_->setColumnCount(4);
    dailyTable_->setHorizontalHeaderLabels({"日期","最高","最低","天气"});
    dailyTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    dailyTable_->verticalHeader()->setVisible(false);
    dailyTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    dailyTable_->setSelectionMode(QAbstractItemView::NoSelection);
    dailyTable_->setShowGrid(false);
    stacked_->addWidget(hourlyTable_);
    stacked_->addWidget(dailyTable_);
    stacked_->setVisible(false); // 默认只显示实时卡片，不挤占空间

    // 根布局
    QVBoxLayout* root = new QVBoxLayout(this);
    root->setContentsMargins(0,0,0,0);
    root->setSpacing(6);
    root->addLayout(header);
    root->addWidget(resultList_);
    root->addLayout(row);
    root->addWidget(stacked_);

    // 交互
    connect(searchBtn_, &QPushButton::clicked, [this]() {
        QString name = searchEdit_->text().trimmed();
        if (name.isEmpty()) return;
        pendingQuery_ = name;
        QUrl url("https://geocoding-api.open-meteo.com/v1/search");
        QUrlQuery qs; qs.addQueryItem("name", name); qs.addQueryItem("count", "10"); qs.addQueryItem("language", "zh");
        url.setQuery(qs);
        QNetworkRequest req(url);
        req.setHeader(QNetworkRequest::UserAgentHeader, "HRSystem-Weather/1.0");
        geoSearchNam_->get(req);
    });
    connect(searchEdit_, &QLineEdit::returnPressed, searchBtn_, &QPushButton::click);
    connect(modeCombo_, &QComboBox::currentIndexChanged, [this](int idx){
        if (idx==0) { stacked_->setVisible(false); }
        else { stacked_->setVisible(true); stacked_->setCurrentIndex(idx-1); }
    });
    connect(resultList_, &QListWidget::itemClicked, [this](QListWidgetItem* item){
        if (!item) return;
        QVariantList coords = item->data(Qt::UserRole).toList();
        if (coords.size() == 2) {
            lat_ = coords[0].toDouble();
            lon_ = coords[1].toDouble();
        }
        cityName_ = item->text();
        cityLabel_->setText(cityName_);
        resultList_->setVisible(false);
        refreshWeather();
    });
}

void WeatherWidget::applyCardStyle()
{
    setObjectName("weatherCard");
    setStyleSheet("#weatherCard{background:white;border:1px solid #e5e7eb;border-radius:12px;} ");
}

void WeatherWidget::refreshWeather()
{
    // Open-Meteo 综合接口：实时+逐小时+日级
    QUrl url("https://api.open-meteo.com/v1/forecast");
    QUrlQuery q;
    q.addQueryItem("latitude", QString::number(lat_));
    q.addQueryItem("longitude", QString::number(lon_));
    q.addQueryItem("current_weather", "true");
    q.addQueryItem("hourly", "temperature_2m,weathercode");
    q.addQueryItem("daily", "weathercode,temperature_2m_max,temperature_2m_min");
    q.addQueryItem("forecast_days", "7");
    q.addQueryItem("timezone", "auto");
    url.setQuery(q);
    nam_->get(QNetworkRequest(url));
}

void WeatherWidget::onReplyFinished(QNetworkReply* r)
{
    if (!r) return;
    QByteArray data = r->readAll();
    r->deleteLater();
    parseAndRender(data);
}

void WeatherWidget::onGeoReplyFinished(QNetworkReply* r)
{
    if (!r) return;
    QByteArray data = r->readAll();
    r->deleteLater();
    QJsonParseError e; QJsonDocument doc = QJsonDocument::fromJson(data, &e);
    if (e.error == QJsonParseError::NoError && doc.isObject()) {
        QJsonObject o = doc.object();
        // ip-api.com 字段: lat, lon, city, country
        if (o.contains("lat") && o.contains("lon")) {
            lat_ = o.value("lat").toDouble(lat_);
            lon_ = o.value("lon").toDouble(lon_);
            QString city = o.value("city").toString();
            QString country = o.value("country").toString();
            if (!city.isEmpty()) {
                cityName_ = city + (country.isEmpty()?"":(" · " + country));
                cityLabel_->setText(cityName_);
            }
        }
    }
    refreshWeather();
}

void WeatherWidget::parseAndRender(const QByteArray& data)
{
    QJsonParseError e; QJsonDocument doc = QJsonDocument::fromJson(data, &e);
    if (e.error != QJsonParseError::NoError || !doc.isObject()) return;
    QJsonObject obj = doc.object();
    QJsonObject cur = obj.value("current_weather").toObject();
    double temp = cur.value("temperature").toDouble();
    int code = cur.value("weathercode").toInt();
    bool isDay = cur.value("is_day").toInt(1) == 1;
    tempLabel_->setText(QString::number(qRound(temp)) + "°C");
    descLabel_->setText(mapWeatherCodeToText(code));
    updateTimeLabel_->setText("更新: " + QDateTime::currentDateTime().toString("HH:mm"));
    iconLabel_->setText(mapWeatherCodeToIcon(code));
    iconLabel_->setAlignment(Qt::AlignCenter);
    iconLabel_->setStyleSheet("font-size:28px;");

    // 动态背景
    updateBackground(code, isDay);

    // 解析逐小时
    hourlyTimes_.clear(); hourlyTemps_.clear(); hourlyCodes_.clear();
    QJsonObject hourly = obj.value("hourly").toObject();
    QJsonArray tArr = hourly.value("time").toArray();
    QJsonArray tempArr = hourly.value("temperature_2m").toArray();
    QJsonArray codeArr = hourly.value("weathercode").toArray();
    int hCount = qMin(tArr.size(), qMin(tempArr.size(), codeArr.size()));
    for (int i=0;i<hCount;i++) {
        QString t = tArr.at(i).toString();
        hourlyTimes_.append(QDateTime::fromString(t, Qt::ISODate));
        hourlyTemps_.append(tempArr.at(i).toDouble());
        hourlyCodes_.append(codeArr.at(i).toInt());
    }
    renderHourly();

    // 解析日级
    dailyDates_.clear(); dailyMax_.clear(); dailyMin_.clear(); dailyCodes_.clear();
    QJsonObject daily = obj.value("daily").toObject();
    QJsonArray dTime = daily.value("time").toArray();
    QJsonArray dMax = daily.value("temperature_2m_max").toArray();
    QJsonArray dMin = daily.value("temperature_2m_min").toArray();
    QJsonArray dCode = daily.value("weathercode").toArray();
    int dCount = qMin(dTime.size(), qMin(dMax.size(), qMin(dMin.size(), dCode.size())));
    for (int i=0;i<dCount;i++) {
        dailyDates_.append(QDate::fromString(dTime.at(i).toString(), Qt::ISODate));
        dailyMax_.append(dMax.at(i).toDouble());
        dailyMin_.append(dMin.at(i).toDouble());
        dailyCodes_.append(dCode.at(i).toInt());
    }
    renderDaily();
}

QString WeatherWidget::mapWeatherCodeToText(int code) const
{
    // 简化映射（Open-Meteo WMO codes）
    if (code==0) return "晴";
    if (code==1||code==2) return "多云";
    if (code==3) return "阴";
    if (code>=45 && code<=48) return "雾";
    if (code>=51 && code<=67) return "小到中雨";
    if (code>=71 && code<=77) return "小到中雪";
    if (code>=80 && code<=82) return "阵雨";
    if (code>=95) return "雷暴";
    return "天气";
}

QString WeatherWidget::mapWeatherCodeToIcon(int code) const
{
    // 使用 Emoji 作为极简图标，避免引入资源依赖
    if (code==0) return "☀️";
    if (code==1||code==2) return "⛅";
    if (code==3) return "☁️";
    if (code>=45 && code<=48) return "🌫️";
    if (code>=51 && code<=67) return "🌧️";
    if (code>=71 && code<=77) return "🌨️";
    if (code>=80 && code<=82) return "🌦️";
    if (code>=95) return "⛈️";
    return "🌡️";
}

void WeatherWidget::fetchLocation()
{
    // 使用 ip-api 免费定位，HTTP 明文，适用于非敏感定位
    QNetworkRequest req(QUrl("http://ip-api.com/json/"));
    geoNam_->get(req);
}

void WeatherWidget::onGeoSearchFinished(QNetworkReply* r)
{
    if (!r) return;
    QByteArray data = r->readAll();
    r->deleteLater();
    QJsonParseError e; QJsonDocument doc = QJsonDocument::fromJson(data, &e);
    if (e.error == QJsonParseError::NoError && doc.isObject()) {
        QJsonObject o = doc.object();
        QJsonArray results = o.value("results").toArray();
        resultList_->clear();
        if (!results.isEmpty()) {
            for (const QJsonValue& v : results) {
                QJsonObject item = v.toObject();
                QString n = item.value("name").toString();
                QString admin1 = item.value("admin1").toString();
                QString country = item.value("country").toString();
                double la = item.value("latitude").toDouble();
                double lo = item.value("longitude").toDouble();
                QString display = n;
                if (!admin1.isEmpty()) display += " · " + admin1;
                if (!country.isEmpty()) display += " · " + country;
                QListWidgetItem* li = new QListWidgetItem(display);
                li->setData(Qt::UserRole, QVariantList{la, lo});
                resultList_->addItem(li);
            }
            resultList_->setVisible(true);
            // 自动选第一个结果
            if (resultList_->count() > 0) {
                resultList_->setCurrentRow(0);
                QMetaObject::invokeMethod(resultList_, "itemClicked", Qt::QueuedConnection,
                                          Q_ARG(QListWidgetItem*, resultList_->item(0)));
            }
        }
    }
}

void WeatherWidget::renderHourly()
{
    if (!hourlyTable_) return;
    int rows = qMin(12, hourlyTimes_.size());
    hourlyTable_->setRowCount(rows);
    for (int i=0;i<rows;++i) {
        QString ts = hourlyTimes_[i].toLocalTime().toString("MM-dd HH:mm");
        hourlyTable_->setItem(i,0,new QTableWidgetItem(ts));
        hourlyTable_->setItem(i,1,new QTableWidgetItem(QString::number(qRound(hourlyTemps_[i]))+"°C"));
        hourlyTable_->setItem(i,2,new QTableWidgetItem(mapWeatherCodeToText(hourlyCodes_[i])));
        hourlyTable_->setRowHeight(i, 24);
    }
    int h = hourlyTable_->horizontalHeader()->height() + rows*24 + 8;
    hourlyTable_->setFixedHeight(h);
    stacked_->setFixedHeight(h);
}

void WeatherWidget::renderDaily()
{
    if (!dailyTable_) return;
    int drows = qMin(7, dailyDates_.size());
    dailyTable_->setRowCount(drows);
    for (int i=0;i<drows;++i) {
        dailyTable_->setItem(i,0,new QTableWidgetItem(dailyDates_[i].toString("MM-dd")));
        dailyTable_->setItem(i,1,new QTableWidgetItem(QString::number(qRound(dailyMax_[i]))+"°C"));
        dailyTable_->setItem(i,2,new QTableWidgetItem(QString::number(qRound(dailyMin_[i]))+"°C"));
        dailyTable_->setItem(i,3,new QTableWidgetItem(mapWeatherCodeToText(dailyCodes_[i])));
        dailyTable_->setRowHeight(i, 24);
    }
    int dh = dailyTable_->horizontalHeader()->height() + drows*24 + 8;
    dailyTable_->setFixedHeight(dh);
    stacked_->setFixedHeight(dh);
}

void WeatherWidget::updateBackground(int code, bool isDay)
{
    QString start = isDay?"#F6FAFF":"#0f172a";
    QString end   = isDay?"#FFFFFF":"#1f2937";
    if (code>=51) { start = isDay?"#dbeafe":"#0b132b"; end = isDay?"#e5e7eb":"#1c2541"; }
    if (code>=80) { start = isDay?"#e0f7fa":"#0b1020"; end = isDay?"#f1f5f9":"#111827"; }
    setStyleSheet(
        QString("#weatherCard{background:qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 %1, stop:1 %2);"
                "border:1px solid rgba(229,231,235,0.8);border-radius:12px;"
                "box-shadow:0 6px 18px rgba(0,0,0,0.12);}").arg(start, end)
    );
}

void WeatherWidget::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        dragging_ = true;
        dragOffset_ = event->globalPosition().toPoint() - frameGeometry().topLeft();
    }
    QWidget::mousePressEvent(event);
}

void WeatherWidget::mouseMoveEvent(QMouseEvent* event)
{
    if (dragging_ && (event->buttons() & Qt::LeftButton)) {
        move(event->globalPosition().toPoint() - dragOffset_);
    }
    QWidget::mouseMoveEvent(event);
}


