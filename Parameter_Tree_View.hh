/*	Parameter_Tree_View

HiROC CVS ID: $Id: Parameter_Tree_View.hh,v 1.6 2012/08/31 06:53:25 castalia Exp $

Copyright (C) 2012  Arizona Board of Regents on behalf of the
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

#ifndef Parameter_Tree_View_hh
#define Parameter_Tree_View_hh

#include	<QTreeView>

//	Forward references.
template<typename T> class QList;

namespace idaeim {
namespace PVL {
class Parameter;
class Aggregate;
}}


namespace UA
{
namespace HiRISE
{
//	Forward references.
class Parameter_Tree_Model;

/**	A <i>Parameter_Tree_View</i> is a QTreeView subclass specialization
	for use with a Parameter_Tree_Model that provides Value sub-tree
	views embedded in cells occupied by Arrays.

	@author		Bradford Castalia and Parker Snell, UA/HiROC
	@version	$Revision: 1.6 $
*/
class Parameter_Tree_View
:	public QTreeView
{
//	Qt Object declaration.
Q_OBJECT

public:

/*==============================================================================
	Types
*/
typedef QList<idaeim::PVL::Parameter*>	Parameter_List;

/*==============================================================================
	Constants
*/
//! Class identification name with source code version and date.
static const char* const
	ID;


//!	Default setting for animating node expansing and collapsing.
static const bool
	DEFAULT_ANIMATION;

/*==============================================================================
	Constructors
*/
/**	Constructs a Parameter_Tree_View on an Aggregate.

	<b>N.B.</b>: The Parameter_Tree_View does not take ownership of the
	parameters. The parameters must not be deleted and should not be
	changed during the lifespan of this Parameter_Tree_View. The {@link
	model() Parameter_Tree_Model) constructed for the parameters is
	implemented as read-only; no editing operations are supported. The
	model that is created is owned by this Parameter_Tree_View.

	@param	parameters	A pointer to an idaeim::PVL::Aggregate. If NULL
		an empty Parameter_Tree_View is constructed.
*/
explicit Parameter_Tree_View (idaeim::PVL::Aggregate* parameters = NULL,
	QWidget* parent = NULL);

/**	Constructs a Parameter_Tree_View on a Parameter_Tree_Model.

	@param	model	A pointer to a Parameter_Tree_Model. <b>N.b.</b>: The
		Parameter_Tree_View does not take ownership of the model. The
		model may be shared by other views.
*/
explicit Parameter_Tree_View (Parameter_Tree_Model* model,
	QWidget* parent = NULL);

//!	Destructor.
virtual ~Parameter_Tree_View ();

/*==============================================================================
	Accessors
*/
/**	Set new parameters to be viewed.

	<b>N.B.</b>: The Parameter_Tree_View does not take ownership of the
	parameters. The parameters must not be deleted and should not be
	changed during the lifespan of this Parameter_Tree_View. The {@link
	model() Parameter_Tree_Model) constructed for the parameters is
	implemented as read-only; no editing operations are supported. The
	model that is created is owned by this Parameter_Tree_View.

	@param	params	A pointer to an idaeim::PVL::Aggregate. If NULL
		the Parameter_Tree_View becomes empty.
*/
virtual void parameters (idaeim::PVL::Aggregate* params);

/**	Get the parameters being viewed.

	@return	A pointer to the idaeim::PVL::Aggregate currently bound to
		the model.
*/
idaeim::PVL::Aggregate* parameters () const;

/**	Test if the root of the tree hierarchy is visible.

	@return	true if the top level root Aggregate node of the tree model
		is visible; false if the root node is not visible.
*/
bool root_visible () const;

/**	Enable or disable visibility of the root node of the tree hierarchy.

	@param	enable	If true, the top level root Aggregate node of the
		tree model is to be visible; if false, the root node is not to be
		visible.
*/
void root_visible (bool enable);

/**	Get the default name of the root tree node.

	@return	A String with the default name of the root node.
	@see	root_name() const
*/
static QString default_root_name ();

/**	Get the name of the root tree node.

	@return	A String with the name of the root node. This will be the
		{@link #
		if the {@link root_name(const QString&) root name} has not been set.
*/
QString root_name () const;

/**	Set the name of the root tree node.

	<b>N.B.</b>: The root node name will only appear in a viewer when the
	{@link root_visible(bool) root node is visible}.

	@param	name	A Qstring providing the name of the root node.
	@see	root_name()
*/
void root_name (const QString& name);

/**	Get the comments for the Parameter at a model index.

	@param	index	A QModelIndex for the Parameter from which to get the
		comments.
	@return	A QString containing the Parameter comments. This may be
		the empty string if the Parameter has no comments or if the
		index is invalid.
*/
QString comments (const QModelIndex& index) const;

/**	Get a list of selected {@link parameters() parameters}.

	@return	A list of parameters that have been selected.
*/
Parameter_List selected_parameters () const;

/*------------------------------------------------------------------------------
	Reimplemented methods
*/
/**	Set a new data model to be viewed.

	<b>N.B.</b>: The existing model is not deleted, nor is the
	selectionModel that was constructed on the previous
	Parameter_Tree_Model. These may be shared, so it is the
	responsibility of the application to delete these models when they
	are no longer in use.

	@param	model	A pointer to a Parameter_Tree_Model. <b>N.b.</b>: The
		Parameter_Tree_View does not take ownership of the model. The
		model may be shared by other views.
	@throws	invalid_argument	If the model is not a Parameter_Tree_Model.
*/
virtual void setModel (QAbstractItemModel* model);

/**	Enables or disables node expanding and collapsing animation.

	@param	enable	If true, animation is enabled; if false nodes will be
		expanded and collapsed without animation.
*/
void setAnimated (bool enable);

/*==============================================================================
	Helpers
*/
private:

void initialize ();

/**	Scan this Parameter_Tree_View for every Array present in the
	Parameter_Tree_Model.

	Where an Assignment Parameter in the tree has an Array Value a
	QTreeView bound to a Value_Tree_Model for the Array is provided as
	the itemWidget to display in the view's cell for the Array.

	@return	true if at least one array subtree was created; false
		otherwise.
*/
bool subtrees ();

/*==============================================================================
	Data
*/
private:

Parameter_Tree_Model
	*Tree_Model;
bool
	Tree_Model_Locally_Owned;

};


}	// end namespace HiRISE
}	// end namespace UA
#endif
