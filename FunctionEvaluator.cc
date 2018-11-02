#include "FunctionEvaluator.hh"

#include <limits>
#include <iostream>
using namespace std;

static QString JS_FUNC_NAME = "value";
static QString JS_FUNC_ARGS = "dn";

FunctionEvaluator::FunctionEvaluator(QString input)
{
    // constructing a function to evaluate
    function = QString("function %1 (%2) { return (%3) }").arg(JS_FUNC_NAME, JS_FUNC_ARGS, input);

    reset();
}

void FunctionEvaluator::reset()
{
    sum = 0.0;
    avg = 0.0;
    cnt = 0.0;
    min = numeric_limits<double>::max();
    max = -min; //C++11 numeric_limits<double>::lowest();

    min_at = -1;
    max_at = -1;
}

void FunctionEvaluator::error()
{
    sum = avg = cnt = min = max = NAN;

    min_at = -1;
    max_at = -1;
}

void FunctionEvaluator::run()
{
    qDebug() << function;

    /* Passing an invalid QScriptValue as the this argument to QScriptValue::call()
     * indicates that the Global Object should be used as the this object;
     * in other words, that the function should be invoked as a global function.
     */
    QScriptValue val = engine.evaluate(function), inv = QScriptValue();

    QScriptValue fun = engine.globalObject().property(JS_FUNC_NAME);

    reset();

    for (int dn = 0 ; dn < data->size() ; dn++)
    {
        unsigned long long count = data->at(dn);

        // skip if no data
        if (count < 1) continue;

        // SKIP DN = 0 ?
        // SKIP DN = 1, 2, 1022, 1023

        val = fun.call(inv, QScriptValueList() << dn);

        if (val.isError())
        {
            // SHOULD report error and use undefined stats
            qDebug() << val.toString();
            error();
            break;
        }

        if (!val.isNumber())
        {
            // SHOULD report error and use undefined stats
            qDebug() << val.toString() << " is not an number";
            error();
            break;
        }

        double num = val.toNumber();

        cnt += count;
        sum += count * num;

        if (num < min)
        {
            min = num;
            min_at = dn;
        }

        if (num > max)
        {
            max = num;
            max_at = dn;
        }
    }

    avg = sum / cnt;
/*
    cout << "*** Statistics ***" << endl;
    cout << " cnt = " << cnt << endl; // like number of pixels in region
    cout << " sum = " << sum << endl; // sum of pixel values
    cout << " min = " << min << " at dn " << min_at << endl; // lowest value from user's function
    cout << " max = " << max << " at dn " << max_at << endl; // highest value from user's function
    cout << " avg = " << avg << endl; // average value from user's function
    cout << endl;
*/
    emit runCompleted();
}

void FunctionEvaluator::setData(QVector<unsigned long long> *data)
{
    this->data = data;
}

void FunctionEvaluator::setProperty(const QString name, const double value)
{
    engine.globalObject().setProperty(name, value);
}
