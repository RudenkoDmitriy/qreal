#include "generatorBase/simpleGenerators/binding.h"

using namespace generatorBase::simple;
using namespace qReal;

Binding::ConverterInterface::~ConverterInterface()
{
}

Binding::MultiConverterInterface::~MultiConverterInterface()
{
}

QString Binding::EmptyConverter::convert(const QString &data) const
{
	return data;
}

Binding *Binding::createStatic(const QString &label, const QString &value)
{
	return new Binding(label, value, false);
}

Binding *Binding::createStaticConverting(const QString &label, const QString &value
		, const ConverterInterface *converter)
{
	return new Binding(label, value, false, converter);
}

Binding *Binding::createDirect(const QString &label, const QString &property)
{
	return new Binding(label, property, true);
}

Binding *Binding::createConverting(const QString &label, const QString &property
		, const ConverterInterface *converter)
{
	return new Binding(label, property, true, converter);
}

Binding *Binding::createMultiTarget(const QString &label, const QString &property
		, const MultiConverterInterface *converter)
{
	return new Binding(label, property, converter);
}

Binding::Binding(const QString &label, const QString &propertyOrValue, bool takeFromRepo)
	: mLabel(label)
	, mProperty(takeFromRepo ? propertyOrValue : "")
	, mValue(takeFromRepo ? "" : propertyOrValue)
	, mConverter(new EmptyConverter)
	, mMultiConverter(nullptr)
{
}

Binding::Binding(const QString &label, const QString &propertyOrValue, bool takeFromRepo
		, const ConverterInterface *converter)
	: mLabel(label)
	, mProperty(takeFromRepo ? propertyOrValue : "")
	, mValue(takeFromRepo ? "" : propertyOrValue)
	, mConverter(converter)
	, mMultiConverter(nullptr)
{
}

Binding::Binding(const QString &label, const QString &property
		, const MultiConverterInterface *converter)
	: mLabel(label)
	, mProperty(property)
	, mConverter(nullptr)
	, mMultiConverter(converter)
{
}

Binding::~Binding()
{
	delete mConverter;
}

void Binding::apply(const qrRepo::RepoApi &repo
		, const Id &id, QString &data)
{
	const QString property = mProperty.isEmpty()
			? mValue
			: mProperty == "name"
					? repo.name(id)
					: repo.property(id, mProperty).toString();

	if (mConverter) {
		data.replace(mLabel, mConverter->convert(property));
	} else {
		applyMulti(property, data);
	}
}

void Binding::applyMulti(const QString &property, QString &data)
{
	QString result;
	const QStringList parts = mMultiConverter->convert(property);
	foreach (const QString &part, parts) {
		QString dataCopy = data;
		dataCopy.replace(mLabel, part);
		result += dataCopy;
	}

	data = result;
}
