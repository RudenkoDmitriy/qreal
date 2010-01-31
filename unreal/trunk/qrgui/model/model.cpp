#include "model.h"
#include <QtCore/QDebug>
#include <QtGui/QPolygon>
#include <QtGui/QIcon>
#include <QtCore/QUuid>

using namespace qReal;
using namespace model;
using namespace details;

Model::Model(EditorManager const &editorManager)
	: mEditorManager(editorManager)
{
	mRootItem = new ModelTreeItem(ROOT_ID, NULL);
	init();
}

Model::~Model()
{
	cleanupTree(mRootItem);
	mTreeItems.clear();
}

bool Model::isDiagram(Id const &id) const
{
	return id.element().contains("Diagram", Qt::CaseInsensitive);
}

void Model::init()
{
	mTreeItems.insert(ROOT_ID, mRootItem);
	mApi.setName(ROOT_ID, ROOT_ID.toString());
	loadSubtreeFromClient(mRootItem);

	if (mRootItem->children().count() == 0) {
		Id elementId(Id::loadFromString("qrm:/Kernel_metamodel/Kernel/Kernel_Diagram"), QUuid::createUuid().toString());
		if (mEditorManager.hasElement(elementId.type()))
			addElementToModel(mRootItem, elementId, "", "(anonymous Diagram)", QPointF(0,0), Qt::CopyAction);
	}
}

Qt::ItemFlags Model::flags(QModelIndex const &index) const
{
	if (index.isValid()) {
		return Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsDragEnabled
			| Qt::ItemIsDropEnabled | Qt::ItemIsEnabled;
	} else {
		return Qt::ItemIsDropEnabled;  // У нас rootItem представляется невалидным индексом, а в него надо уметь кидать элементы.
	}
}

QVariant Model::data(QModelIndex const &index, int role) const
{
	if (index.isValid()) {
		ModelTreeItem *item = static_cast<ModelTreeItem*>(index.internalPointer());
		Q_ASSERT(item);
		switch (role) {
			case Qt::DisplayRole:
			case Qt::EditRole:
				return mApi.name(item->id());
			case Qt::DecorationRole:
				return mEditorManager.icon(item->id());
			case roles::idRole:
				return item->id().toVariant();
			case roles::positionRole:
				return mApi.property(item->id(), positionPropertyName(item));
			case roles::fromRole:
				return mApi.from(item->id()).toVariant();
			case roles::toRole:
				return mApi.to(item->id()).toVariant();
			case roles::fromPortRole:
				return mApi.fromPort(item->id());
			case roles::toPortRole:
				return mApi.toPort(item->id());
			case roles::configurationRole:
				return mApi.property(item->id(), configurationPropertyName(item));
		}
		if (role >= roles::customPropertiesBeginRole) {
			QString selectedProperty = findPropertyName(item->id(), role);
			return mApi.property(item->id(), selectedProperty);
		}
		Q_ASSERT(role < Qt::UserRole);
		return QVariant();
	} else {
		return QVariant();
	}
}

bool Model::setData(QModelIndex const &index, QVariant const &value, int role)
{
	if (index.isValid()) {
		ModelTreeItem *item = static_cast<ModelTreeItem*>(index.internalPointer());
		switch (role) {
		case Qt::DisplayRole:
		case Qt::EditRole:
			mApi.setName(item->id(), value.toString());
			break;
		case roles::positionRole:
			mApi.setProperty(item->id(), positionPropertyName(item), value);
			break;
		case roles::configurationRole:
			mApi.setProperty(item->id(), configurationPropertyName(item), value);
			break;
		case roles::fromRole:
			mApi.setFrom(item->id(), value.value<Id>());
			break;
		case roles::toRole:
			mApi.setTo(item->id(), value.value<Id>());
			break;
		case roles::fromPortRole:
			mApi.setFromPort(item->id(), value.toDouble());
			break;
		case roles::toPortRole:
			mApi.setToPort(item->id(), value.toDouble());
			break;
		default:
			if (role >= roles::customPropertiesBeginRole) {
				QString selectedProperty = findPropertyName(item->id(), role);
				mApi.setProperty(item->id(), selectedProperty, value);
				break;
			}
			Q_ASSERT(role < Qt::UserRole);
			return false;
		}
		emit dataChanged(index, index);
		return true;
	}
	return false;
}

QString Model::findPropertyName(Id const &id, int const role) const
{
	// В случае свойства, описанного в самом элементе, роль - просто
	// порядковый номер свойства в списке свойств. Этого соглашения
	// надо всюду придерживаться, а то роли "поедут".
	QStringList properties = mEditorManager.getPropertyNames(id.type());
	Q_ASSERT(role - roles::customPropertiesBeginRole < properties.count());
	return properties[role - roles::customPropertiesBeginRole];
}

QVariant Model::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (orientation == Qt::Horizontal && role == Qt::DisplayRole && section == 0)
		return QVariant("name");
	else
		return QVariant();
}

int Model::rowCount(QModelIndex const &parent) const
{
	return parentTreeItem(parent)->children().size();
}

int Model::columnCount(QModelIndex const &parent) const
{
	Q_UNUSED(parent)
	return 1;
}

bool Model::removeRows(int row, int count, QModelIndex const &parent)
{
	ModelTreeItem *parentItem = parentTreeItem(parent);

	if (parentItem->children().size() < row + count)
		return false;
	else {
		for (int i = row; i < row + count; ++i) {
			ModelTreeItem *child = parentItem->children().at(i);

			foreach (Id id, mApi.outgoingLinks(child->id()))
				mApi.setFrom(id, ROOT_ID);
			foreach(Id id, mApi.incomingLinks(child->id()))
				mApi.setTo(id, ROOT_ID);

			removeModelItems(child);

			// TODO: Убрать копипасту.
			int childRow = child->row();
			beginRemoveRows(parent, childRow, childRow);
			child->parent()->removeChild(child);
			mTreeItems.remove(child->id(), child);
			if (mTreeItems.count(child->id()) == 0)
				mApi.removeChild(parentItem->id(), child->id());
			delete child;
			endRemoveRows();
		}
		return true;
	}
}

QString Model::pathToItem(ModelTreeItem const *item) const
{
	if (item != mRootItem) {
		QString path;
		do {
			item = item->parent();
			path = item->id().toString() + ID_PATH_DIVIDER + path;
		} while (item != mRootItem);
		return path;
	}
	else
		return ROOT_ID.toString();
}

void Model::removeConfigurationInClient(ModelTreeItem const * const item)
{
	mApi.removeProperty(item->id(), positionPropertyName(item));
	mApi.removeProperty(item->id(), configurationPropertyName(item));
}

QModelIndex Model::index(ModelTreeItem const * const item) const
{
	QList<int> rowCoords;

	for (ModelTreeItem const *curItem = item;
		curItem != mRootItem; curItem = curItem->parent())
	{
		rowCoords.append(const_cast<ModelTreeItem *>(curItem)->row());
	}

	QModelIndex result;

	for (int i = rowCoords.size() - 1; i >= 0; --i) {
		result = index(rowCoords[i], 0, result);
	}

	return result;
}

void Model::removeModelItems(ModelTreeItem *root)
{
	foreach (ModelTreeItem *child, root->children()) {
		removeModelItems(child);
		int childRow = child->row();
		beginRemoveRows(index(root),childRow,childRow);
		removeConfigurationInClient(child);
		child->parent()->removeChild(child);
		mTreeItems.remove(child->id(),child);
		if (mTreeItems.count(child->id())==0) {
			mApi.removeChild(root->id(),child->id());
		}
		delete child;
		endRemoveRows();
	}
}

QModelIndex Model::index(int row, int column, QModelIndex const &parent) const
{
	ModelTreeItem *parentItem = parentTreeItem(parent);
	if (parentItem->children().size()<=row)
		return QModelIndex();
	ModelTreeItem *item = parentItem->children().at(row);
	return createIndex(row,column,item);
}

QModelIndex Model::parent(QModelIndex const &index) const
{
	if (index.isValid()) {
		ModelTreeItem *item = static_cast<ModelTreeItem*>(index.internalPointer());
		ModelTreeItem *parentItem = item->parent();
		if (parentItem == mRootItem || parentItem == NULL)
			return QModelIndex();
		else
			return createIndex(parentItem->row(), 0, parentItem);
	} else
		return QModelIndex();
}

Qt::DropActions Model::supportedDropActions() const
{
	return Qt::CopyAction | Qt::MoveAction | Qt::LinkAction;
}

QStringList Model::mimeTypes() const
{
	QStringList types;
	types.append(DEFAULT_MIME_TYPE);
	return types;
}

QMimeData* Model::mimeData(QModelIndexList const &indexes) const
{
	QByteArray data;
	QDataStream stream(&data, QIODevice::WriteOnly);
	foreach (QModelIndex index, indexes) {
		if (index.isValid()) {
			ModelTreeItem *item = static_cast<ModelTreeItem*>(index.internalPointer());
			stream << item->id().toString();
			stream << pathToItem(item);
			stream << mApi.property(item->id(), "name");
			stream << mApi.property(item->id(), positionPropertyName(item)).toPointF();
		} else {
			stream << ROOT_ID.toString();
			stream << QString();
			stream << ROOT_ID.toString();
			stream << QPointF();
		}
	}
	QMimeData *mimeData = new QMimeData();
	mimeData->setData(DEFAULT_MIME_TYPE, data);
	return mimeData;
}

bool Model::dropMimeData(QMimeData const *data, Qt::DropAction action, int row, int column, QModelIndex const &parent)
{
	Q_UNUSED(row)
	Q_UNUSED(column)
	if (action == Qt::IgnoreAction)
		return true;
	else {
		ModelTreeItem *parentItem = parentTreeItem(parent);

		QByteArray dragData = data->data(DEFAULT_MIME_TYPE);
		QDataStream stream(&dragData, QIODevice::ReadOnly);
		QString idString;
		QString pathToItem;
		QString name;
		QPointF position;
		stream >> idString;
		stream >> pathToItem;
		stream >> name;
		stream >> position;
		Id id = Id::loadFromString(idString);
		Q_ASSERT(id.idSize() == 4);  // Бросать в модель мы можем только конкретные элементы.

		if (mTreeItems.contains(id))  // Пока на диаграмме не может быть больше одного экземпляра
			// одной и той же сущности, бросать существующие элементы нельзя.
			return false;

		return addElementToModel(parentItem, id, pathToItem, name, position, action) != NULL;
	}
}

ModelTreeItem *Model::addElementToModel(ModelTreeItem *parentItem, Id const &id,
	QString const &oldPathToItem, QString const &name, QPointF const &position, Qt::DropAction action)
{
	Q_UNUSED(oldPathToItem)
	Q_UNUSED(action)

	if (isDiagram(id)) {
		if (!isDiagram(parentItem->id()) && parentItem != mRootItem) {
			qDebug() << "Диаграмму нельзя добавить в элемент.";
			return NULL;
		}
	}
	else {
		if (parentItem == mRootItem) {
			qDebug() << "Элемент можно добавить только на диаграмму";
			return NULL;
		}
	}

	int newRow = parentItem->children().size();
	beginInsertRows(index(parentItem), newRow, newRow);
		ModelTreeItem *item = new ModelTreeItem(id, parentItem);
		parentItem->addChild(item);
		mTreeItems.insert(id, item);
		mApi.addChild(parentItem->id(), id);
		mApi.setProperty(id, "name", name);
		mApi.setProperty(id, "from", ROOT_ID.toVariant());
		mApi.setProperty(id, "to", ROOT_ID.toVariant());
		mApi.setProperty(id, "fromPort", 0.0);
		mApi.setProperty(id, "toPort", 0.0);
		mApi.setProperty(id, "links", IdListHelper::toVariant(IdList()));
		mApi.setProperty(id, positionPropertyName(item), position);
		mApi.setProperty(id, configurationPropertyName(item), QVariant(QPolygon()));

		QStringList properties = mEditorManager.getPropertyNames(id.type());
		foreach (QString property, properties) {
			// Здесь должна быть инициализация значениями по умолчанию
			// (а ещё лучше, если не здесь). Считать этот код временным хаком,
			// пока нет системы типов.
			mApi.setProperty(id, property, "");
		}
	endInsertRows();
	return item;
}

void Model::changeParent(QModelIndex const &element, QModelIndex const &parent, QPointF const &position)
{
	if (!parent.isValid() || element.parent() == parent)
		return;

	int destinationRow = parentTreeItem(parent)->children().size();

	if (beginMoveRows(element.parent(), element.row(), element.row(), parent, destinationRow)) {
		ModelTreeItem *elementItem = static_cast<ModelTreeItem*>(element.internalPointer());
		QVariant configuration = mApi.property(elementItem->id(), configurationPropertyName(elementItem));
		elementItem->parent()->removeChild(elementItem);
		ModelTreeItem *parentItem = parentTreeItem(parent);

		mApi.addParent(elementItem->id(), parentItem->id());
		mApi.removeParent(elementItem->id(), elementItem->parent()->id());

		elementItem->setParent(parentItem);
		parentItem->addChild(elementItem);

		mApi.setProperty(elementItem->id(), positionPropertyName(elementItem), position);
		mApi.setProperty(elementItem->id(), configurationPropertyName(elementItem), configuration);
		endMoveRows();
	}
}

void Model::loadSubtreeFromClient(ModelTreeItem * const parent)
{
	foreach (Id childId, mApi.children(parent->id())) {
		ModelTreeItem *child = loadElement(parent, childId);
		loadSubtreeFromClient(child);
	}
}

ModelTreeItem *Model::loadElement(ModelTreeItem *parentItem, Id const &id)
{
	int newRow = parentItem->children().size();
	beginInsertRows(index(parentItem), newRow, newRow);
		ModelTreeItem *item = new ModelTreeItem(id, parentItem);
		checkProperties(id);
		parentItem->addChild(item);
		mTreeItems.insert(id, item);
	endInsertRows();
	return item;
}

void Model::checkProperties(Id const &id)
{
	if (!editorManager().hasElement(id.type()))
		return;
	QStringList propertiesThatShallBe = editorManager().getPropertyNames(id.type());
	foreach (QString property, propertiesThatShallBe)
		if (!api().hasProperty(id, property))
			mApi.setProperty(id, property, "");  // Типа значение по умолчанию.
}

QPersistentModelIndex Model::rootIndex() const
{
	return index(mRootItem);
}

QString Model::positionPropertyName(ModelTreeItem const *item) const
{
	return "position + " + pathToItem(item);
}

QString Model::configurationPropertyName(ModelTreeItem const *item) const
{
	return "configuration + " + pathToItem(item);
}

void Model::exterminate()
{
	mApi.exterminate();
	cleanupTree(mRootItem);
	mTreeItems.clear();
	delete mRootItem;
	mRootItem = new ModelTreeItem(ROOT_ID, NULL);
	reset();
	init();
}

void Model::cleanupTree(ModelTreeItem *root)
{
	foreach (ModelTreeItem *childItem, root->children()) {
		cleanupTree(childItem);
		delete childItem;
	}
	root->clearChildren();
}

qrRepo::RepoApi const & Model::api() const
{
	return mApi;
}

EditorManager const &Model::editorManager() const
{
	return mEditorManager;
}

ModelTreeItem *Model::parentTreeItem(QModelIndex const &parent) const
{
	return parent.isValid()
		? static_cast<ModelTreeItem*>(parent.internalPointer())
		: mRootItem
	;
}
