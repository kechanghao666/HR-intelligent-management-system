#ifndef WEATHERWIDGET_H
#define WEATHERWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QStackedWidget>
#include <QTableWidget>
#include <QListWidget>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QDateTime>
#include <QMouseEvent>

// 一个轻量的实时天气小组件：使用 Open-Meteo 无需密钥接口
// 可通过环境变量覆盖默认位置：
//   WEATHER_LAT, WEATHER_LON, WEATHER_CITY

class WeatherWidget : public QWidget
{
    Q_OBJECT
public:
    explicit WeatherWidget(QWidget* parent = nullptr);
    ~WeatherWidget();

    void setCoordinate(double lat, double lon, const QString& cityDisplayName = QString());

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;

private slots:
    void refreshWeather();
    void onReplyFinished(QNetworkReply* reply);
    void onGeoReplyFinished(QNetworkReply* reply);
    void onGeoSearchFinished(QNetworkReply* reply);

private:
    void setupUI();
    void applyCardStyle();
    void parseAndRender(const QByteArray& data);
    QString mapWeatherCodeToText(int code) const;
    QString mapWeatherCodeToIcon(int code) const;
    void fetchLocation();
    void renderHourly();
    void renderDaily();
    void updateBackground(int code, bool isDay);

private:
    QLabel* iconLabel_;
    QLabel* tempLabel_;
    QLabel* descLabel_;
    QLabel* cityLabel_;
    QLabel* updateTimeLabel_;
    QPushButton* refreshBtn_;
    QLineEdit* searchEdit_;
    QPushButton* searchBtn_;
    QComboBox* modeCombo_;
    QStackedWidget* stacked_;
    QListWidget* resultList_;
    QTableWidget* hourlyTable_;
    QTableWidget* dailyTable_;

    QTimer* timer_;
    QNetworkAccessManager* nam_;
    QNetworkAccessManager* geoNam_;
    QNetworkAccessManager* geoSearchNam_;

    double lat_;
    double lon_;
    QString cityName_;

    // 缓存预测数据
    QVector<QDateTime> hourlyTimes_;
    QVector<double> hourlyTemps_;
    QVector<int> hourlyCodes_;
    QVector<QDate> dailyDates_;
    QVector<double> dailyMax_;
    QVector<double> dailyMin_;
    QVector<int> dailyCodes_;

    // drag
    bool dragging_ = false;
    QPoint dragOffset_;

    // search state
    QString pendingQuery_;
};

#endif // WEATHERWIDGET_H


