/** @file resizeHandler.h
* 	@brief Class for handling resize behavior of NodeElement. Provides methods for container resizing.
**/

#pragma once

#include "editor/nodeElement.h"

class ResizeHandler
{
public:
	/**
	 * Constructs a ResizeHandler.
	 * @param resizingNode Node that is actually dealt with.
	 */
	ResizeHandler(NodeElement * const resizingNode);

	/**
	 * Resizes node trying to use newContents as new shape
	 * of node (ignoring newContents position) and to move
	 * node to newPos.
	 * These parameters are corrected by child configuration
	 * in most cases.
	 * @param newContents Recommendation for new shape of node.
	 * @param newPos Recommendation for new position of node.
	 */
	void resize(QRectF newContents, QPointF newPos, bool needResizeParent = true) const;

private:
	/// Minimum size of node
	static const int mMinSize = 10;

	/**
	 * Sorts child items in case node has
	 * sortChildren container property.
	 */
	void sortChildrenIfNeeded() const;

	/**
	 * Returns maximum of child item widths.
	 * @return Maximum of child widths.
	 */
	qreal maxChildWidth() const;

	/**
	 * Gripes contents to QRectF() in case node has
	 * minimizesToChildren container property.
	 * @param contents It would be gripped to QRectF() in case
	 * node has minimizesToChildren container property.
	 */
	void gripeIfMinimizesToChildrenContainer(QRectF &contents) const;

	/// Calls resize() method for parent item.
	void resizeParent() const;

	/**
	 * Changes contents to size that not smaller than folded contents.
	 * @param contents It will be not smaller than folded contents.
	 */
	void normalizeSize(QRectF &contents) const;

	/**
	 * Resizes newContents and moves newPos according to child configuration.
	 * @param newContents It will be modified according to children.
	 * @param newPos It will be modified according to children.
	 */
	void resizeAccordingToChildren(QRectF &newContents, QPointF &newPos) const;

	/**
	 * Returns vector of minimum negative XY child deflection from top left corner.
	 * @return Vector of minimum negative XY child deflection from top left corner.
	 */
	QPointF childDeflection() const;

	/**
	 * Moves children with shift with
	 * controling child position (it can't be upper
	 * or more left than (sizeOfForestalling, sizeOfForestalling).
	 * @param shift Child shift.
	 */
	void moveChildren(const QPointF &shift) const;

	/**
	 * Returns bounding rectangle for childItem.
	 * @param childItem For this item bounding rectangle will be returned.
	 * @param contents It will be used for place holder bounding calculating.
	 * @return Bounding rectangle for childItem.
	 */
	QRectF childBoundingRect(const QGraphicsItem * const childItem, const QRectF &contents) const;

	/**
	 * Expands contents according to child configuration.
	 * @param contents It will be expanded according to child configuration.
	 */
	void expandByChildren(QRectF& contents) const;

	/**
	 * Prints position of child NodeElement items.
	 * It used for debug purposes.
	 */
	void printChildPos() const;

	QList<NodeElement *> sortedChildrenList() const;

	/// Node that is actually dealt with.
	NodeElement * const mTargetNode;

	/// ElementImpl of node that is actually dealt with.
	ElementImpl * const mElementImpl;
};

