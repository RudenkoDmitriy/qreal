#include "serializer.h"

#include <QtCore/QDir>
#include <QtCore/QDebug>
#include <QtCore/QPointF>
#include <QtCore/QCoreApplication>
#include <QtGui/QPolygon>

#include <qrkernel/settingsManager.h>
#include <qrkernel/exception/exception.h>
#include <qrutils/outFile.h>
#include <qrutils/xmlUtils.h>
#include <qrutils/fileSystemUtils.h>

#include "folderCompressor.h"
#include "classes/logicalObject.h"
#include "classes/graphicalObject.h"

using namespace qrRepo;
using namespace details;
using namespace utils;
using namespace qReal;

const QString unsavedDir = "unsaved";

Serializer::Serializer(const QString& saveDirName)
	: mWorkingDir(QCoreApplication::applicationDirPath() + "/" + unsavedDir)
	, mWorkingFile(saveDirName)
{
	clearWorkingDir();
	/// @todo: throw away this legacy piece of sh.t
	SettingsManager::setValue("temp", mWorkingDir);
	QDir dir(QCoreApplication::applicationDirPath());
	if (!dir.cd(mWorkingDir)) {
		QDir().mkdir(mWorkingDir);
	}
}

void Serializer::clearWorkingDir() const
{
	clearDir(mWorkingDir);
}

void Serializer::removeFromDisk(const Id &id) const
{
	QDir dir;
	dir.remove(pathToElement(id));
}

void Serializer::setWorkingFile(const QString &workingFile)
{
	mWorkingFile = workingFile;
}

void Serializer::saveToDisk(QList<Object *> const &objects, QHash<QString, QVariant> const &metaInfo) const
{
	Q_ASSERT_X(!mWorkingFile.isEmpty()
		, "Serializer::saveToDisk(...)"
		, "may be Repository of RepoApi (see Models constructor also) has been initialised with empty filename?");

	foreach (const Object * const object, objects) {
		const QString filePath = createDirectory(object->id(), object->isLogicalObject());

		QDomDocument doc;
		QDomElement root = object->serialize(doc);
		doc.appendChild(root);

		OutFile out(filePath);
		doc.save(out(), 2);
	}

	saveMetaInfo(metaInfo);

	QFileInfo fileInfo(mWorkingFile);
	QString fileName = fileInfo.baseName();

	QDir compressDir(SettingsManager::value("temp").toString());
	QDir dir = fileInfo.absolutePath();

	QFile previousSave(dir.absolutePath() + "/" + fileName +".qrs");
	if (previousSave.exists()) {
		previousSave.remove();
	}

	const QString filePath = fileInfo.absolutePath() + "/" + fileName + ".qrs";
	FolderCompressor::compressFolder(compressDir.absolutePath(), filePath);

	// Hiding autosaved files
	if (fileName.contains("~")) {
		FileSystemUtils::makeHidden(filePath);
	}

	clearDir(mWorkingDir);
}

void Serializer::loadFromDisk(QHash<qReal::Id, Object*> &objectsHash, QHash<QString, QVariant> &metaInfo)
{
	clearWorkingDir();
	if (!mWorkingFile.isEmpty()) {
		decompressFile(mWorkingFile);
	}

	loadFromDisk(SettingsManager::value("temp").toString(), objectsHash);
	loadMetaInfo(metaInfo);
}

void Serializer::loadFromDisk(const QString &currentPath, QHash<qReal::Id, Object*> &objectsHash)
{
	QDir dir(currentPath + "/tree");
	if (dir.cd("logical")) {
		loadModel(dir, objectsHash);
		dir.cdUp();
		dir.cd("graphical");
		loadModel(dir, objectsHash);
	}
}

void Serializer::loadModel(const QDir &dir, QHash<qReal::Id, Object*> &objectsHash)
{
	foreach (const QFileInfo &fileInfo, dir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot)) {
		const QString path = fileInfo.filePath();
		if (fileInfo.isDir()) {
			loadModel(path, objectsHash);
		} else if (fileInfo.isFile()) {
			QDomDocument doc = xmlUtils::loadDocument(path);
			const QDomElement element = doc.documentElement();

			// To ensure backwards compatibility. Replace this by separate tag names when save updating mechanism
			// will be implemented.
			Object * const object = element.hasAttribute("logicalId") && element.attribute("logicalId") != "qrm:/"
					? dynamic_cast<Object *>(new GraphicalObject(element))
					: dynamic_cast<Object *>(new LogicalObject(element))
					;

			objectsHash.insert(object->id(), object);
		}
	}
}

void Serializer::saveMetaInfo(QHash<QString, QVariant> const &metaInfo) const
{
	QDomDocument document;
	QDomElement root = document.createElement("metaInformation");
	document.appendChild(root);
	for (const QString &key : metaInfo.keys()) {
		QDomElement element = document.createElement("info");
		element.setAttribute("key", key);
		element.setAttribute("type", metaInfo[key].typeName());
		element.setAttribute("value", ValuesSerializer::serializeQVariant(metaInfo[key]));
		root.appendChild(element);
	}

	const QString filePath = mWorkingDir + "/metaInfo.xml";
	OutFile out(filePath);
	out() << document.toString(4);
}

void Serializer::loadMetaInfo(QHash<QString, QVariant> &metaInfo) const
{
	metaInfo.clear();

	const QString filePath = mWorkingDir + "/metaInfo.xml";
	if (!QFile::exists(filePath)) {
		return;
	}

	const QDomDocument document = xmlUtils::loadDocument(filePath);
	for (QDomElement child = document.documentElement().firstChildElement("info")
			; !child.isNull()
			; child = child.nextSiblingElement("info"))
	{
		metaInfo[child.attribute("key")] = ValuesSerializer::deserializeQVariant(
				child.attribute("type"), child.attribute("value"));
	}
}

void Serializer::clearDir(const QString &path)
{
	if (path.isEmpty()) {
		return;
	}

	QDir dir(path);
	if (!dir.exists()) {
		return;
	}

	foreach (const QFileInfo &fileInfo, dir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot)) {
		if (fileInfo.isDir()) {
			clearDir(fileInfo.filePath());
			dir.rmdir(fileInfo.fileName());
		} else {
			dir.remove(fileInfo.fileName());
		}
	}
}

QString Serializer::pathToElement(const Id &id) const
{
	QString dirName = mWorkingDir;

	QStringList partsList = id.toString().split('/');
	Q_ASSERT(partsList.size() >=1 && partsList.size() <= 5);
	for (int i = 1; i < partsList.size() - 1; ++i) {
		dirName += "/" + partsList[i];
	}

	return dirName + "/" + partsList[partsList.size() - 1];
}

QString Serializer::createDirectory(const Id &id, bool logical) const
{
	QString dirName = mWorkingDir + "/tree";
	dirName += logical ? "/logical" : "/graphical";

	const QStringList partsList = id.toString().split('/');
	Q_ASSERT(partsList.size() >= 1 && partsList.size() <= 5);
	for (int i = 1; i < partsList.size() - 1; ++i) {
		dirName += "/" + partsList[i];
	}

	QDir dir;
	dir.rmdir(mWorkingDir);
	dir.mkpath(dirName);

	return dirName + "/" + partsList[partsList.size() - 1];
}

void Serializer::decompressFile(const QString &fileName)
{
	FolderCompressor::decompressFolder(fileName, mWorkingDir);
}
