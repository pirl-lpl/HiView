/*	Count_Sequence

HiROC CVS ID: $Id: Count_Sequence.hh,v 1.4 2012/07/21 22:31:25 castalia Exp $

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

#ifndef HiView_Count_Sequence_hh
#define HiView_Count_Sequence_hh

#include	<QVector>

//	Qwt
#include	"qwt_series_data.h"

namespace UA::HiRISE
{
/**	A <i>Count_Sequence</i> is a sequence of integer data in the form of
	QwtData suitible for plotting on a graph.

	The Count_Sequence was designed to be used by a Histogram_Plot for
	defining the values of a sequence to be plotted as histogram bars.

	@author		Bradford Castalia, UA/HiROC
	@version	$Revision: 1.4 $
*/
class Count_Sequence
:	public QwtSeriesData<QPointF>
{
/*==============================================================================
	Constructors
*/
public:

/**	Constructs an empty Count_Sequence.
*/
Count_Sequence ();

/**	Constructs a copy of a Count_Sequence, optionally setting the
	new sequence {@link base(double) base} and {@link increment(double)
	increment} values.

	@param	count_sequence	The Count_Sequence to be copied.
	@param	base	The new {@link base(double) base} of the sequence.
	@param	increment	The new {@link increment(double) increment}
		of the sequence.
*/
Count_Sequence (const Count_Sequence& count_sequence,
	double base = 0.0, double increment = 1.0);

/**	Constructs a Count_Sequence from a data vector, optionally setting the
	new sequence {@link base(double) base} and {@link increment(double)
	increment} values.

	@param	data_sequence	A QVector of unsigned long long data.
	@param	base	The new {@link base(double) base} of the sequence.
	@param	increment	The new {@link increment(double) increment}
		of the sequence.
*/
Count_Sequence (const QVector<unsigned long long>& data_sequence,
	double base = 0.0, double increment = 1.0);

/**	Constructs a Count_Sequence from a data array, optionally setting the
	new sequence {@link base(double) base} and {@link increment(double)
	increment} values.

	@param	data_sequence	An array of unsigned long long data values.
	@param	size	The number of values to be used from the data array.
	@param	base	The new {@link base(double) base} of the sequence.
	@param	increment	The new {@link increment(double) increment}
		of the sequence.
*/
Count_Sequence (const unsigned long long* data_sequence, int size,
	double base = 0.0, double increment = 1.0);

//!	Destructor.
virtual ~Count_Sequence ();

/*==============================================================================
	Accessors
*/
/**	Assigns another Count_Sequence to this Count_Sequence.

	The {@link data() data} content of the other Count_Sequence is
	assigned to replace the data content of this Count_Sequence. The
	{@link max_count() max count} value is updated.

	@param	count_sequence	A Count_Sequence reference to have its
		data assigned as the data of this Count_Sequence.
	@return	This Count_Sequence.
	@see	data(const Count_Sequence&)
*/
Count_Sequence& operator= (const Count_Sequence& count_sequence);

/**	Sets the Count_Sequence data from the data of another Count_Sequence.

	The data content of the other Count_Sequence is
	assigned to replace the data content of this Count_Sequence. The
	{@link max_count() max count} value is updated.

	@param	count_sequence	A Count_Sequence reference to have its
		data assigned as the data of this Count_Sequence.
	@return	This Count_Sequence.
	@see	operator=(const Count_Sequence&)
	@see	data()
*/
Count_Sequence& data (const Count_Sequence& count_sequence);

/**	Sets the Count_Sequence data from a data vector.

	The data vector is assigned to replace the data content of this
	Count_Sequence. The {@link max_count() max count} value is updated.

	@param	count_sequence	A Count_Sequence reference to have its
		data assigned as the data of this Count_Sequence.
	@return	This Count_Sequence.
	@see	data(const Count_Sequence&)
	@see	data()
*/
Count_Sequence& data (const QVector<unsigned long long>& data_sequence);

/**	Sets the Count_Sequence data from a data array.

	The previous data content is cleared and then the content of the data
	copied to replace the data content of this Count_Sequence. The {@link
	max_count() max count} value is updated.

	@param	count_sequence	A Count_Sequence reference to have its
		data assigned as the data of this Count_Sequence.
	@return	This Count_Sequence.
	@see	data()
*/
Count_Sequence& data (const unsigned long long* data_sequence, int size);

/**	Gets a reference to the data content of this Count_Sequence.

	@return	A QVector<unsigned long long> reference to the data content
		of this Count_Sequence. <b>N.B.</b>: The data content can be
		externally modified; however, {@link calculate_max_count()
		updating the max count value} is should be done after any
		external data changes.
*/
QVector<unsigned long long>& data ()
	{return Data;}

/**	Sets the number of data values in the Count_Sequence.

	If the new amountis zero the current data content is cleared.  If the
	new amount is the same as the current data content {@link size()
	size} nothing is done. Otherwise the data content vector is resized,
	removing excess data values or adding additional zero data values.
	The {@link max_count() max count} value is updated if the data
	content amount changes.

	@param	amount	The number of
*/
void size (int amount);

/**	Sets the Count_Sequence base value.

	The base is the x value of the first data content element.

	@param	base_value	A double base value for the Count_Sequence.
	@return	This Count_Sequence.
	@see	x()
*/
inline Count_Sequence& base (double base_value)
	{Base = base_value; return *this;}

/**	Gets the Count_Sequence base value.

	@return	The base value of the Count_Sequence.
	@see	base(double)
*/
inline double base () const
	{return Base;}

/**	Sets the Count_Sequence increment amount.

	The increment is the x distance between each data content element.

	@param	increment_amount	A double increment amount for the
		Count_Sequence.
	@return	This Count_Sequence.
	@see	x()
*/
inline Count_Sequence& increment (double increment_amount)
	{Increment = increment_amount; return *this;}

/**	Gets the Count_Sequence increment amount.

	@return	The increment amount of the Count_Sequence.
	@see	increment(double)
*/
inline double increment () const
	{return Increment;}

/**	Gets the maximum data content value.

	@return	The maximum data content value.
	@see	calculate_max_count()
*/
inline unsigned long long max_count () const
	{return Max_Count;}

/**	Recalculates the {@link max_count() maximum data content value}.

	The {@link max_count() maximum data content value} is updated
	and returned.

	@return	The maximum data content value.
*/
unsigned long long calculate_max_count ();

/*==============================================================================
	QwtData virtual methods implementations
*/
/**	Gets a copy of this Count_Sequence.

	A new Count_Sequence is constructed from this Count_Sequence.

	@return	A pointer to a new Count_Sequence. <b>N.B.</b>: Ownership of
		the new Count_Sequence is transferred to the caller which is
		reposible for disposing of the object.
*/
virtual QwtSeriesData* copy () const;

/**	Gets the data content size of the Count_Sequence.

	@return	The number of data values in Count_Sequence.
*/
virtual size_t size () const;

/**	Gets the x value of a datum.

	The x value of a datum is the {@link base() base} value plus the
	{@link increment() increment} value times the datum index in the
	sequence.

	@param	index	The index of the datum in the Count_Sequence.
		<b>N.B.</b>: The index value may be greater than or equal to
		the {@link size() size} of the data content.
*/
double x (size_t index) const;

/**	Gets the y value of a datum.

	The y value of a datum is the data content value at the index
	as a double value.

	@param	index	The index of the datum in the Count_Sequence. If the
		index value is greater than or equal to the {@link size() size}
		of then data content -1 is returned.
*/
double y (size_t index) const;

virtual QPointF sample (size_t index) const;

/**	Gets the rectange that bounds the data content.

	The left edge of the rectangle will have the {@link base() base}
	value. The top edge will have the {@link max_count() max count}
	value. The width will be the data content {@link size () size}
	times the {@link increment() increment}.

	@return	A QRectF that describes the bounding box of the
		data content.
*/
virtual QRectF boundingRect () const;

/*==============================================================================
	Data
*/
private:

QVector<unsigned long long>
	Data;

unsigned long long
	Max_Count;

double
	Base,
	Increment;

};

}	//	namespace UA::HiRISE
#endif
