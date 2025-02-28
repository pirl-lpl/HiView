/*	Script_Evaluator.cc

HiROC CVS ID: $Id: Image_Info_Panel.hh,v 1.14 2014/08/05 17:58:09 stephens Exp $

Copyright (C) 2010-2011  Arizona Board of Regents on behalf of the
Planetary Image Research Laboratory, Lunar and Planetary Laboratory at
the University of Arizona.

This library is free software; you can redistribute it and/or modify it
under the terms of the GNU Lesser General Public License, version 2.1,
as published by the Free Software Foundation.

This library is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this library; if not, write to the Free Software Foundation,
Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA.

*******************************************************************************/
#include "Script_Evaluator.hh"

#ifndef QT_NO_DEBUG_OUTPUT
#include <QDebug>
#include <QJSValueIterator>
#endif

#include <QSetIterator>
#include <QDateTime>

#include <iostream>
#include <limits>

using std::numeric_limits;

namespace UA::HiRISE
{

Script_Exception::Script_Exception(const QString& msg) : message(msg)
{ }

void Script_Exception::raise() const
{
  throw *this;
}

Script_Exception * Script_Exception::clone() const
{
  return new Script_Exception(*this);
}

const QString
  Script_Evaluator::VAR_X_PX = "x_px",
  Script_Evaluator::VAR_Y_PX = "y_px",
  Script_Evaluator::VAR_BAND0_VAL = "red",
  Script_Evaluator::VAR_BAND1_VAL = "green",
  Script_Evaluator::VAR_BAND2_VAL = "blue",
  Script_Evaluator::VAR_SCALE_METERS = "scale_meters",
  Script_Evaluator::VAR_AREA_METERS2 = "area_meters2",
  Script_Evaluator::VAR_COUNT_PIXELS = "pixel_count",
  Script_Evaluator::VAR_WIDTH_METERS = "width_meters",
  Script_Evaluator::VAR_HEIGHT_METERS = "height_meters",
  Script_Evaluator::VAR_WIDTH_PIXELS = "width_px",
  Script_Evaluator::VAR_HEIGHT_PIXELS = "height_px",
  Script_Evaluator::VAR_LENGTH_PIXELS = "length_pixels",
  Script_Evaluator::VAR_LENGTH_METERS = "length_meters";

const QString
  Script_Evaluator::TOOLTIP_VAR_X_PX = "the x coordinate in pixels",
  Script_Evaluator::TOOLTIP_VAR_Y_PX = "the y coordinate in pixels",
  Script_Evaluator::TOOLTIP_VAR_BAND0_VAL = "the first band (red) color data value",
  Script_Evaluator::TOOLTIP_VAR_BAND1_VAL = "the second band (green) color data value",
  Script_Evaluator::TOOLTIP_VAR_BAND2_VAL = "the third band (blue) color data value",
  Script_Evaluator::TOOLTIP_VAR_SCALE_METERS = "the scale in meters",
  Script_Evaluator::TOOLTIP_VAR_AREA_METERS2 = "the area in square meters",
  Script_Evaluator::TOOLTIP_VAR_COUNT_PIXELS = "the count of selected pixel",
  Script_Evaluator::TOOLTIP_VAR_WIDTH_METERS = "the width in meters",
  Script_Evaluator::TOOLTIP_VAR_HEIGHT_METERS = "the height in meters",
  Script_Evaluator::TOOLTIP_VAR_WIDTH_PIXELS = "the width in pixels",
  Script_Evaluator::TOOLTIP_VAR_HEIGHT_PIXELS = "the height in pixels",
  Script_Evaluator::TOOLTIP_VAR_LENGTH_PIXELS = "the length in pixels",
  Script_Evaluator::TOOLTIP_VAR_LENGTH_METERS = "the length in meters";

Script_Evaluator::Script_Evaluator(QObject* parent) :
  QObject(parent), imagevars({}), labelvars({})
{
  selectedBand = BandOrdinal::NONE;

  imagevars << VAR_X_PX << VAR_Y_PX;
  imagevars << VAR_BAND0_VAL << VAR_BAND1_VAL << VAR_BAND2_VAL;
  imagevars << VAR_SCALE_METERS << VAR_AREA_METERS2 << VAR_COUNT_PIXELS;
  imagevars << VAR_WIDTH_METERS << VAR_HEIGHT_METERS << VAR_WIDTH_PIXELS;
  imagevars << VAR_HEIGHT_PIXELS << VAR_LENGTH_METERS << VAR_LENGTH_PIXELS;

  engine = new QJSEngine(this);
  global = engine->globalObject();
  undef = QJSValue::UndefinedValue;
#ifndef QT_NO_DEBUG_OUTPUT
  qDebug() << "Script_Evaluator initialized";
#endif
}

Script_Evaluator::~Script_Evaluator()
{
  engine->setInterrupted(true);
  engine->collectGarbage();
  delete engine;

#ifndef QT_NO_DEBUG_OUTPUT
  qDebug() << "Script_Evaluator destroyed";
#endif
}

void Script_Evaluator::setScript(const QString& input)
{
  // constructing a function to evaluate
  //function = QString("function %1 (%2) { return (%3) }").arg(JS_FUNC_NAME, JS_FUNC_ARGS, input);
  if (engine->evaluate(input).isError())
  {
      qWarning() << "Script '" << input << "'" << "failed to validate";
      selectedBand = BandOrdinal::NONE;

      throw Script_Exception("Invalid Script");
      return;
  }

  function = QString(input);

  //TODO(guym) set selectedBand here
}

void Script_Evaluator::reset()
{
#ifndef QT_NO_DEBUG_OUTPUT
  qDebug() << "Resetting script variables";
#endif

    evaluated_sum = 0L;
    evaluated_avg = 0.0;
    total_pixel_count = 0L;
    evaluated_min = numeric_limits<double>::max();
    evaluated_max = numeric_limits<double>::lowest();

    min_at = -1;
    max_at = -1;

    valid_min = 0;
    valid_max = 0;
}

void Script_Evaluator::clearScriptProperties()
{
#ifndef QT_NO_DEBUG_OUTPUT
  qDebug() << "Clearing script properties";
#endif

  QSetIterator <QString> imgvals(imagevars);
  QSetIterator <QString> lblvals(labelvars);

  //to unset properties from last image, make them undefined
  while (imgvals.hasNext()) global.setProperty(imgvals.next(), undef);
  while (lblvals.hasNext()) global.setProperty(lblvals.next(), undef);

  imagevars.clear();
  labelvars.clear();
}

void Script_Evaluator::erred()
{
#ifndef QT_NO_DEBUG_OUTPUT
  qDebug() << "Error in script evaluation.";
#endif

    evaluated_avg = evaluated_min = evaluated_max = numeric_limits<double>::quiet_NaN();

    evaluated_sum = total_pixel_count = numeric_limits<long double>::quiet_NaN();

    min_at = -1;
    max_at = -1;
}

void Script_Evaluator::run()
{
#ifndef QT_NO_DEBUG_OUTPUT
  qDebug() << "Running script";
  QJSValueIterator it(engine->globalObject());
  while (it.hasNext())
  {
    it.next();
    qDebug() << it.name() << ": " << it.value().toString();
  }
#endif

  if (stats == nullptr) return;

  const QString* var_band_val;

  int band = -1;

  switch (selectedBand)
  {
      case BandOrdinal::FIRST:
        var_band_val = &VAR_BAND0_VAL;
        band = 0;
        break;
      case BandOrdinal::SECOND:
        var_band_val = &VAR_BAND1_VAL;
        band = 1;
        break;
      case BandOrdinal::THIRD:
        var_band_val = &VAR_BAND2_VAL;
        band = 2;
        break;
      default:
        return;
  }

#ifndef QT_NO_DEBUG_OUTPUT
  qDebug() << "Band " << band << " selected";
#endif

    /* Passing an invalid QScriptValue as the this argument to QScriptValue::call()
     * indicates that the Global Object should be used as the this object;
     * in other words, that the function should be invoked as a global function.

    QScriptValue val = engine.evaluate(function), inv = QScriptValue();

    QScriptValue fun = engine.globalObject().property(JS_FUNC_NAME);
    */

    reset();

    const auto & histograms = stats->histograms();

    const auto & data = histograms[band];

    auto lo = valid_min;
    auto hi = (valid_max > 0 && valid_max < data->size()) ? valid_max : data->size();

    QJSValue val;

    // iterate through histogram
    for (auto dn = lo ; dn < hi ; dn++)
    {
        global.setProperty(*var_band_val, dn);

        unsigned long long count = data->at(dn);

        // skip if no data
        if (count < 1) continue;

        val = engine->evaluate(function);
        //val = fun.call(inv, QScriptValueList() << dn);

        if (val.isError())
        {
            // SHOULD report error and use undefined stats
            qDebug() << val.toString();
            emit erred();
            break;
        }

        if (!val.isNumber())
        {
            // SHOULD report error and use undefined stats
            qDebug() << val.toString() << " is not an number";
            emit erred();
            break;
        }

        auto result = val.toNumber();

        total_pixel_count += count;
        evaluated_sum += count * static_cast<long double>(result);

        if (result < evaluated_min)
        {
            evaluated_min = result;
            min_at = dn;
        }

        if (result > evaluated_max)
        {
            evaluated_max = result;
            max_at = dn;
        }
    }

    evaluated_avg = static_cast<double>(evaluated_sum / total_pixel_count);
}

long double Script_Evaluator::getCount() const
{
  return total_pixel_count;
}

long double Script_Evaluator::getSum() const
{
  return evaluated_sum;
}

double Script_Evaluator::getAvg() const
{
  return evaluated_avg;
}

double Script_Evaluator::getMin() const
{
  return evaluated_min;
}

double Script_Evaluator::getMax() const
{
  return evaluated_max;
}

int Script_Evaluator::getMinAt() const
{
  return min_at;
}

int Script_Evaluator::getMaxAt() const
{
  return max_at;
}

void Script_Evaluator::setValidMin(const int value)
{
    valid_min = value;
}

void Script_Evaluator::setValidMax(const int value)
{
    valid_max = value;
}

QString Script_Evaluator::getPixelText() const
{
  return QString("pixel: %1").arg(evaluated_avg, 0, 'g', 3);
}

QString Script_Evaluator::getScriptText() const
{
  return QString("script: %1").arg(evaluated_avg, 0, 'g', 3);
}

void Script_Evaluator::setProperty(const QString& name, const QJSValue& value)
{
    global.setProperty(name, value);
    labelvars << name;

    if (!value.isNumber()) return;

    qreal number = value.toNumber();

    for (const auto& lowname : PDSMETA_LOW_VALUE_NAMES)
    {
        if (QString::compare(name, lowname, Qt::CaseInsensitive) == 0)
        {
            if (valid_min == -1 || number > valid_min) valid_min = number;
            return;
        }
    }

    for (const auto& hiname : PDSMETA_HIGH_VALUE_NAMES)
    {
        if (QString::compare(name, hiname, Qt::CaseInsensitive) == 0)
        {
            if (valid_max == -1 || number < valid_max) valid_max = number;
            return;
        }
    }
}

//recursively iterate through metadata to get properties for engine.
void Script_Evaluator::setMetadata(idaeim::PVL::Aggregate* metadata)
{
    if (metadata == nullptr) return;

    auto end = metadata->end_depth();

    //iterate through the metadata
    for (auto parameters = metadata->begin_depth(); parameters != end; ++parameters)
    {
        //if is_Aggregate() the parameter contains metadata, so make recursive call to this function
        if (parameters->is_Aggregate())
        {
            setMetadata(parameters.aggregate());
            continue;
        }

        QString name = QString::fromStdString(parameters->name()).replace(':', '.');
        idaeim::PVL::Value& value = parameters->value();
        //names containing '^' typically tell the location of a file, so they are unnecissary
        if (name.contains('^')) continue;

        //if array representation is used, convert to QScriptValue array
        if (value.is_Array())
        {
            QJSValue engine_array = engine->newArray();
            array_to_string(static_cast<idaeim::PVL::Array&>(value), engine_array);
            //Properties_List.push_back(name);
            setProperty(name, engine_array);
            continue;
        }
        //otherwise push name onto property list, and set property in engine

            //Properties_List.push_back(name);

        if (value.is_Real())
        {
            setProperty(name, static_cast<idaeim::PVL::Value::Real_type>(value));
        }
        else if (value.is_Integer())
        {
            /*
             * Note, no method signature corresponding to
             * Value::Integer_type (long long) or Value::Unsigned_Integer_type (unsigned long long)
             */
            if (value.is_signed())
            {
                setProperty(name, static_cast<int>(value));
            }
            else
            {
                setProperty(name, static_cast<uint>(value));
            }
        }
        else if (value.is_Date_Time())
        {
            // TODO(guym) check this
            QString tstr = QString::fromStdString(static_cast<idaeim::PVL::Value::String_type>(value).c_str());
            qint64 ms1970 = QDateTime::fromString(tstr, "yyyyMMdd'T'HHmmss").toMSecsSinceEpoch();
            setProperty(name, engine->evaluate(QString("new Date(%1)").arg(ms1970)));
        }
        else // TODO(guym) check is_Identifier or is_Symbol ??
        {
            setProperty(name, QString::fromStdString(static_cast<std::string>(value).c_str()));
        }
    } // for loop

#ifndef QT_NO_DEBUG_OUTPUT
    qDebug() << "Script global properties";
    QJSValueIterator it(global);
    while (it.hasNext())
    {
        it.next();
        qDebug() << it.name() << ": " << it.value().toString();
    }
#endif
} // function declaration
} // namespace UA::HiRISE
