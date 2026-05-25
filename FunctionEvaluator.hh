#pragma once

#include <QJSEngine>
#include <QObject>
#include <QString>
#include <QThread>
#include <QVector>

namespace UA::HiRISE
{
class FunctionEvaluator : public QThread
{
    Q_OBJECT

 public:
    FunctionEvaluator(QString input);

    void run() override;

    double getSum() const { return sum; }
    double getAvg() const { return avg; }
    double getMin() const { return min; }
    double getMax() const { return max; }
    double getCnt() const { return cnt; }
    double getMinAtDN() const { return min_at; }
    double getMaxAtDN() const { return max_at; }

 public slots:
    void setData(QVector<unsigned long long>* data);
    void setProperty(QString name, double value);

 signals:
    void runCompleted();

 private:
    void reset();
    void error();

    QString function;
    long double cnt;
    long double sum;
    double avg;
    double min;
    double max;

    int min_at;
    int max_at;

    QJSEngine engine;

    QVector<unsigned long long>* data;
};
}  // namespace UA::HiRISE
