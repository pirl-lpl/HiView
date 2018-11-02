#ifndef FUNCTIONEVALUATOR_H
#define FUNCTIONEVALUATOR_H

#include <QObject>
#include <QThread>
#include <QString>
#include <QVector>
#include <QtScript>

class FunctionEvaluator : public QThread
{
    Q_OBJECT

public:
    FunctionEvaluator(QString input);

    void run();

    inline double getSum() { return sum; }
    inline double getAvg() { return avg; }
    inline double getMin() { return min; }
    inline double getMax() { return max; }
    inline double getCnt() { return cnt; }
    inline double getMinAtDN() { return min_at; }
    inline double getMaxAtDN() { return max_at; }

public slots:
    void setData(QVector<unsigned long long> *data);
    void setProperty(const QString name, const double value);

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

    QScriptEngine engine;

    QVector<unsigned long long> *data;
};

#endif // FUNCTIONEVALUATOR_H
