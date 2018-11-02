/*	Stats

HiROC CVS ID: $Id: Stats.hh,v 1.11 2014/08/05 17:58:09 stephens Exp $

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

#ifndef HiView_Stats_hh
#define HiView_Stats_hh

#include	"Plastic_Image.hh"	//	For the Histogram data type.

#include	<QVector>


namespace UA
{
namespace HiRISE
{
/**	<i>Stats</i> for an image region.

	The Stats class is a carrier for a set of statistical information
	associated with an arbitrary image region represented by the
	histogram of pixel values for the region.

	Summary statistics are {@link calculate() calculated} from the
	Histogram bound to a statistics set. A Histogram is a vector of pixel
	value counts with one entry for each possible pixel value. Histograms
	must be provided externally, they are not owned by the Stats object.
	By {@link histogram(Histogram*, int) binding a Histogram to a
	statistics set} the set becomes eligible to have its statistics
	summary (@link calculate(int) calculated}; a set may be unbound from
	a histgram to disable it.

	Statistics sets are provided for up to three (MAX_STATISTICS_SETS)
	image data bands. A statistics set is bound to a histogram for a
	region of image data and provides summary statistics for that
	histogram:

	- values counted
	- lowest value
	- highest value
	- minimum count
	- maximum count
	- mean value
	- median value
	- standard deviation of values
	- area

	Summary statistics are calcluated over the range of the histogram
	from a lower limit value to an upper limit value, inclusive. The
	histogram areas (total of counts) below and above the limits are
	provided along with the other summary statistics.

	@author		Bradford Castalia, UA/HiROC
	@version	$Revision: 1.11 $
*/
class Stats
{
public:
/*==============================================================================
	Types
*/
typedef Plastic_Image::Histogram	Histogram;

/*==============================================================================
	Constants
*/
//!	Class identification name with source code version and date.
static const char* const
	ID;


//!	Maximum number of statistics sets supported.
enum
	{
	MAX_STATISTICS_SETS	= 3
	};

enum Range_Selection
	{
	LOWER_RANGE		= -1,
	VALID_RANGE		= 0,
	UPPER_RANGE		= 1
	};

/*==============================================================================
	Constructors
*/
/**	Construct a Stats object.

	No histograms will be bound to the statistics sets. The statistics
	summary will be cleared.
*/
explicit Stats ();

/**	Copy a Stats object.

	The statistics sets will be bound to the same histograms as in the
	Stats object being copied. All the statistics summary values will
	be copied over.
*/
Stats (const Stats& stats);

/**	Assign another Stats object to this Stats object.

	The statistics sets will be bound to the same histograms as in the
	Stats object being assigned. All the statistics summary values will
	be copied over from the assigned object.

	@param	stats	A Stats object to be assigned to this Stats object.
*/
Stats& operator= (const Stats& stats);

/**	Destroy the Stats object.
*/
virtual ~Stats ();

/*==============================================================================
	Accessors
*/
/**	Get the number of statistics sets bound to source histogram data.

	Each statistics set is expected to be {@link histogram(Histogram*,
	int) bound by a histogram} to a unique image data band. The number of
	statistics sets is determined by the count of non-NULL {@link
	histogram(QVector<unsigned long long>*, int) histograms} that have
	been bound to this Stats object.

	@return	The number of statistics sets currently bound to source
		histogram data.
*/
int sets () const;

/**	Get the lower limit of valid image values used in {@link calculate(int)
	calculating} statistics.

	@return	The lower limit value.
	@see	lower_limit(int)
*/
inline int lower_limit () const
	{return Lower_Limit;}

/**	Get the upper limit of valid image values used in {@link calculate(int)
	calculating} statistics.

	<b>N.B.</b>: The upper limit returned is the offset from the maximum
	possible histogram value. To obtain the actual upper limit value
	subtract the upper limit from the statistics set histogram {@link
	total_values() total values} minus one. The result will be -1 if the
	statistics set is not bound to a histogram or the histogram is empty.

	@return	The upper limit value offset (a non-negative number) from the
		maximum possible value.
	@see	lower_limit(int)
*/
inline int upper_limit () const
	{return Upper_Limit;}

/**	Set the limits of valid image values used in {@link calculate(int)
	calculating} statistics.

	Values below the minimum and above the maximum limits are excluded
	from statistics calculations. The limits should be set to the minimum
	and maximum valid image values; e.g. extreme values are often
	reserved for a special purpose and not to be considered real image
	values.

	Note that the upper limit is expressed as the offset from the maximum
	histogram entry ({@link total_values(int) total values - 1), which is
	effectively how the lower limit is also expressed. The limit offsets
	apply to all histograms and  will be carried over when a different
	size {@link histogram(Histogram*, int) histogram is bound} to a
	statistics set.

	If either of the current limit values changes the summary statistics
	are {@link calculate(int) recalculated}.

	@param	min	The offset from the minimum possible histogram value
		(this is the same as the minimum value) to be included in
		statistics calculations.
	@param	max	The offset from the maximum possible histogram value
		to be included in statistics calculations. A negative value
		is set to zero.
*/
void limits (int min, int max);

/**	Get the total number of values that can be counted by a histogram.

	The number of histogram bins determines the total number of values
	that can potentially be counted. Since image pixel sample values are
	assumed to be unsigned integers, the total number of values is
	expected to correspond to one greater than the maximum pixel sample
	value; i.e. pixel samples values will be in the range 0 -
	(total_values - 1).

	<b>N.B.</b>: The values outside the {@link lower_limit() lower limit}
	to {@link upper_limit() upper limit} range are <b>NOT</b> excluded.

	@param	index	The index of the histogram from which the value
		is determined.
	@return The total number of values that can be counted by the
		histogram. This will be zero if the statistics set is not bound
		to a histogram.
	@throws	illegal_argument if the index value is outside the range
		0 <= index < MAX_STATISTICS_SETS.
*/
inline int total_values (int index) const
	{return Histograms[index_range_check (index)] ?
		Histograms[index]->size () : 0;}

/**	Get the number of values with a non-zero count.

	This statistic provides the number of {@link histogram{int)
	histogram} entries within the {@link lower_limit() lower limit} to
	{@link upper_limit() upper limit} range, inclusive, that were found
	to have a non-zero value when the summary statistics were last {@link
	calculate(int) calculated}.

	@param	index	The index of the statistics set for which the value
		is to be obtained.
	@return	The number of non-zero histogram entries that were counted.
		This will be zero if the statistics set is not bound to a
		histogram.
	@throws	illegal_argument if the index value is outside the range
		0 <= index < MAX_STATISTICS_SETS.
*/
inline int	values_counted (int index) const
	{return Values_Counted[index_range_check (index)];}

/**	Get the value with lowest non-zero count.

	Only values in within the {@link lower_limit() lower limit} to
	{@link upper_limit() upper limit} range, inclusive, are considered.

	@param	index	The index of the statistics set for which the value
		is to be obtained.
	@return	The entry of the histogram with the lowest non-zero count.
		This will be -1 if the statistics set is not bound to a
		histogram.
	@throws	illegal_argument if the index value is outside the range
		0 <= index < MAX_STATISTICS_SETS.
*/
inline int lowest_value (int index) const
	{return Lowest_Value[index_range_check (index)];}

/**	Get the value with highest non-zero count.

	Only values in within the {@link lower_limit() lower limit} to
	{@link upper_limit() upper limit} range, inclusive, are considered.

	@param	index	The index of the statistics set for which the value
		is to be obtained.
	@return	The entry of the histogram with the highest non-zero count.
		This will be -1 if the statistics set is not bound to a
		histogram.
	@throws	illegal_argument if the index value is outside the range
		0 <= index < MAX_STATISTICS_SETS.
*/
inline int highest_value (int index) const
	{return Highest_Value[index_range_check (index)];}

/**	Get the minimum histogram count value.

	Only values in within the {@link lower_limit() lower limit} to
	{@link upper_limit() upper limit} range, inclusive, are considered.

	@param	index	The index of the statistics set for which the value
		is to be obtained.
	@return	The minimum histogram count value. This will be zero if the
		statistics set is not bound to a histogram.
	@throws	illegal_argument if the index value is outside the range
		0 <= index < MAX_STATISTICS_SETS.
*/
inline unsigned long long minimum_count (int index)
	{return Minimum_Count[index_range_check (index)];}

/**	Get the maximum histogram count value.

	Only values in within the {@link lower_limit() lower limit} to
	{@link upper_limit() upper limit} range, inclusive, are considered.

	@param	index	The index of the statistics set for which the value
		is to be obtained.
	@return	The maximum histogram count value. This will be zero if the
		statistics set is not bound to a histogram.
	@throws	illegal_argument if the index value is outside the range
		0 <= index < MAX_STATISTICS_SETS.
*/
inline unsigned long long maximum_count (int index)
	{return Maximum_Count[index_range_check (index)];}

/**	Get the area of a histogram.

	The area is the total of all the count values in a histogram between
	the {link lower_limit() lower limit} and {@link upper_limit() upper
	limit} values, inclusive. However, if the range specification is
	LOWER_RANGE or UPPER_RANGE, the total count of values below the
	lower limit or above the upper limit, respectively, will be
	returned.

	@param	index	The index of the statistics set for which the value
		is to be obtained.
	@param	range	A Range_Selection, which may be one of VALID_RANGE
	(the default), LOWER_RANGE or UPPER_RANGE.
	@return	The area of the histogram in the specifed value range. This
		will be zero if the statistics set is not bound to a histogram.
	@throws	illegal_argument if the index value is outside the range
		0 <= index < MAX_STATISTICS_SETS.
*/
unsigned long long area (int index, Range_Selection range = VALID_RANGE) const;

/**	Get the mean value determined from the histogram.

	The mean value is the histogram entry where the mean of the histogram
	counts occurs. Only values in within the {@link lower_limit() lower
	limit} to {@link upper_limit() upper limit} range, inclusive, are
	considered.

	@param	index	The index of the statistics set for which the value
		is to be obtained.
	@return	The mean value of the histogram. This will be zero if the
		statistics set is not bound to a histogram.
	@throws	illegal_argument if the index value is outside the range
		0 <= index < MAX_STATISTICS_SETS.
*/
inline double mean_value (int index)
	{return Mean_Value[index_range_check (index)];}

/**	Get the median determined from the histogram.

	The median value is the histogram entry where the median of the
	histogram counts occurs.  Only values in within the {@link
	lower_limit() lower limit} to {@link upper_limit() upper limit}
	range, inclusive, are considered.

	@param	index	The index of the statistics set for which the value
		is to be obtained.
	@return	The median value of the histogram. This will be zero if the
		statistics set is not bound to a histogram.
	@throws	illegal_argument if the index value is outside the range
		0 <= index < MAX_STATISTICS_SETS.
*/
inline double median_value (int index)
	{return Median_Value[index_range_check (index)];}

/**	Get the standard deviation of the histogram.

	Only values in within the {@link lower_limit() lower limit} to
	{@link upper_limit() upper limit} range, inclusive, are considered.

	@param	index	The index of the statistics set for which the value
		is to be obtained.
	@return	The standard deviation of the histogram counts. This will be
		zero if the statistics set is not bound to a histogram.
	@throws	illegal_argument if the index value is outside the range
		0 <= index < MAX_STATISTICS_SETS.
*/
inline double standard_deviation_of_values (int index)
	{return Std_Dev_of_Values[index_range_check (index)];}

/**	Get the upper histogram percentage at a data value.

	The percentage of the histogram {@link area() area} above the
	specified data value is calculated, excluding any area above the
	{@link upper_limit() upper limit}.

	@param	value	The lower limit, exclusive, of histogram count
		entries to be included in the calculation of histogram area.
	@param	index	The index of the statistics set for which the value
		is to be obtained.
	@return	The percentage of the histogram area above the specified data
		value, excluding any area above the upper limit. This will be zero
		if the statistics set is not bound to a histogram.
	@throws	illegal_argument if the index value is outside the range
		0 <= index < MAX_STATISTICS_SETS.
	@see	upper_value_at_percent(double, int, int)
*/
double upper_percent_at_value (int value, int index) const;

/**	Get the lower histogram percentage at a data value.

	The percentage of the histogram {@link area() area} below the
	specified data value is calculated, excluding any area below
	the {@link lower_limit() lower limit}.

	@param	value	The upper limit, exclusive, of histogram count
		entries to be included in the calculation of histogram area.
	@param	index	The index of the statistics set for which the value
		is to be obtained.
	@return	The percentage of the histogram area below the specified data
		value, excluding any area below the lower limit. This will be
		zero if the statistics set is not bound to a histogram.
	@throws	illegal_argument if the index value is outside the range
		0 <= index < MAX_STATISTICS_SETS.
	@see	lower_value_at_percent(double, int, int)
*/
double lower_percent_at_value (int value, int index) const;

/**	Get the data value demarking where a given percentage of the histogram
	area is at and above the value.

	The histogram is searched, decreasing from the {@link
	highest_value(int) highest non-zero count value}, for the entry where
	the area at and above the value becomes equal to or greater than the
	specified percentage, excluding any area above the upper limit.

	<b>N.B.</b>: The demarkation value is the lowest value within the
	upper percentage area. The percentage area at and above the value may
	be greater than the specified percentage, but the value will be the
	highest value at and above which the upper percentage is at least the
	specifed percentage.

	@param	percent	The percentage of the histogram area to occur
		at and above the demarkation value.
	@param	index	The index of the statistics set for which the value
		is to be obtained.
	@return	The data value demarking the specified percentage at and
		above the value, excluding any area above the upper limit. This
		will be -1 if the statistics set is not bound to a histogram.
	@throws	illegal_argument if the index value is outside the range
		0 <= index < MAX_STATISTICS_SETS.
	@see	upper_percent_at_value(int, int, int)
	@see	area(int, Range_Selection)
*/
int upper_value_at_percent (double percent, int index) const;

/**	Get the data value demarking where a given percentage of the histogram
	area is at and below the value.

	The histogram is searched, increasing from the {@link
	lowest_value(int) lowest non-zero count value}, for the entry where
	the area at and below the value becomes equal to or greater than the
	specified percentage, excluding any area below the lower limit.

	<b>N.B.</b>: The demarkation value is the highest value within the
	lower percentage area. The percentage area at and below the value may
	be greater than the specified percentage, but the value will be the
	lowest value at and below which the lower percentage is at least the
	specifed percentage.

	@param	percent	The percentage of the histogram area to occur
		at and below the demarkation value.
	@param	index	The index of the statistics set for which the value
		is to be obtained.
	@return	The data value demarking the specified percentage at and
		below the value, excluding any area below the lower limit. This
		will be -1 if the statistics set is not bound to a histogram.
	@throws	illegal_argument if the index value is outside the range
		0 <= index < MAX_STATISTICS_SETS.
	@see	upper_percent_at_value(int, int)
	@see	area(int, Range_Selection)
*/
int lower_value_at_percent (double percent, int index) const;

/**	Bind a Histogram data vector to the Stats.

	<b>N.B.</b>: The Stats object does NOT take ownership of the Histogram.

	The summary statistics are {@link calculate(int) re-calculated}.

	The {@link lower_limit() lower limit} and {@link upper_limit()
	upper limit} values are not reset. If changing a statistis set
	histogram binding changes the {@link total_values() total values}
	for the set the 

	@param	data	A Histogram pointer to be bound to this Stats object.
		This may be NULL to de-activate a statistics data set.
	@param	index	The index of the statistics set to which the histogram
		data is to be bound.
	@return	This Stats object.
	@throws	illegal_argument if the index value is outside the range
		0 <= index < MAX_STATISTICS_SETS.
*/
Stats& histogram (Histogram* data, int index);

/**	Get the Histogram bound to a statistics data set.

	@param	index	The index of a statistics set.
	@return	A pointer to a Histogram. This will be NULL if the
		statistics set is not bound to a histogram.
	@throws	illegal_argument if the index value is outside the range
		0 <= index < MAX_STATISTICS_SETS.
	@see	histogram(Histogram*, int)
*/
inline Histogram* histogram (int index)
	{return Histograms[index_range_check (index)];}

/**	Get a vector of the histograms for all statistics data sets.

	@return	A QVector of Histogram pointers, one for each statistics
		set. Any or all of the pointers may be NULL if the
		corresponding statistics set is not bound to a histogram.
*/
inline QVector<Histogram*>& histograms ()
	{return Histograms;}

/*==============================================================================
	Manipulators
*/
/**	Recalculate the summary statistics for one or all statistics sets.

	The summary statistics for the statistics set are cleared, except
	the contents of the histogram are not zero-ed out since the
	histogram data is the source of the summary statistics. If a
	statistics set is not {@link histogram(Histogram*, int) bound to
	a Histogram}, the summary statistics are left cleared. Otherwise
	the histogram data is scanned to produce new summary statistics
	values.

	Note: The statistics are calculated for the histogram values between
	the {link lower_limit() lower limit} and {@link upper_limit() upper
	limit} values, inclusive. 

	@param	index	The index of the statistics set for which the value
		is to be obtained. If negative, all statistics sets will be
		re-calculated.
	@throws	illegal_argument if the index value is not valid.
*/
void calculate (int index = -1);

/**	Clear all statistics sets.

	All summary statistics will be set to zero, except the {@link
	lowest_value(int) lowest value} and {@link highest_value(int) highest
	value} will be set to -1.

	Each statistics set that has a {@link histogram (Histogram*, int)
	histogram bound} to it will have the histogram contents cleared to
	all zeros.
*/
void clear ();

/*==============================================================================
	Helpers
*/
private:

static int index_range_check (int index, int max = 0);

/*==============================================================================
	Data
*/
int
	Values_Counted[3],
	Lowest_Value[3],
	Highest_Value[3],
	Lower_Limit,
	Upper_Limit;

unsigned long long
	Minimum_Count[3],
	Maximum_Count[3],
	Valid_Area[3],
	Lower_Area[3],
	Upper_Area[3];

double
	Mean_Value[3],
	Median_Value[3],
	Std_Dev_of_Values[3];

QVector<Histogram*>
	Histograms;

};


}	//	namespace HiRISE
}	//	namespace UA
#endif
