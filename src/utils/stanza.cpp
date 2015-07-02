#include "stanza.h"

#include <QTextStream>
#include <QDebug>
#include "jid.h"

static inline bool IsValidXmlChar(quint32 ACode)
{
	// w3c xml spec: [2] Char ::= #x9 | #xA | #xD | [#x20-#xD7FF] | [#xE000-#xFFFD] | [#x10000-#x10FFFF]
	return ACode == 0x9
		|| ACode == 0xA
		|| ACode == 0xD
		|| (ACode >= 0x20 && ACode <= 0xD7FF)
		|| (ACode >= 0xE000 && ACode <= 0xFFFD)
		|| (ACode >= 0x10000 && ACode <= 0x10FFFF);
}

StanzaData::StanzaData(const QString &ATagName)
{
	FDoc.appendChild(FDoc.createElement(ATagName));
}

StanzaData::StanzaData(const QDomElement &AElem)
{
	FDoc.appendChild(FDoc.importNode(AElem,true));
}

StanzaData::StanzaData(const StanzaData &AOther) : QSharedData(AOther)
{
	FDoc = AOther.FDoc.cloneNode(true).toDocument();
}

Stanza::Stanza(const QString &ATagName)
{
	d = new StanzaData(ATagName);
}

Stanza::Stanza(const QDomElement &AElem)
{
	d = new StanzaData(AElem);
}

Stanza::~Stanza()
{

}

void Stanza::detach()
{
	d.detach();
}

bool Stanza::isValid() const
{
	if (element().isNull())
		return false;

	if (type()=="error" && firstElement("error").isNull())
		return false;

	return true;
}

bool Stanza::isFromServer() const
{
	if (!to().isEmpty())
	{
		Jid toJid = to();
		Jid fromJid = from();
		return fromJid.isEmpty() || fromJid==toJid || fromJid==toJid.bare() || fromJid==toJid.domain();
	}
	return false;
}

QDomDocument Stanza::document() const
{
	return d->FDoc;
}

QDomElement Stanza::element() const
{
	return d->FDoc.documentElement();
}

QString Stanza::attribute(const QString &AName) const
{
	return d->FDoc.documentElement().attribute(AName);
}

Stanza &Stanza::setAttribute(const QString &AName, const QString &AValue)
{
	if (!AValue.isEmpty())
		d->FDoc.documentElement().setAttribute(AName,AValue);
	else
		d->FDoc.documentElement().removeAttribute(AName);
	return *this;
}

QString Stanza::tagName() const
{
	return d->FDoc.documentElement().tagName();
}

Stanza &Stanza::setTagName(const QString &ATagName)
{
	d->FDoc.documentElement().setTagName(ATagName);
	return *this;
}

QString Stanza::type() const
{
	return attribute("type");
}

Stanza &Stanza::setType(const QString &AType)
{
	setAttribute("type",AType);
	return *this;
}

QString Stanza::id() const
{
	return attribute("id");
}

Stanza &Stanza::setId(const QString &AId)
{
	setAttribute("id",AId);
	return *this;
}

QString Stanza::to() const
{
	return attribute("to");
}

Stanza &Stanza::setTo(const QString &ATo)
{
	setAttribute("to",ATo);
	return *this;
}

QString Stanza::from() const
{
	return attribute("from");
}

Stanza &Stanza::setFrom(const QString &AFrom)
{
	setAttribute("from",AFrom);
	return *this;
}

QString Stanza::lang() const
{
	return attribute("xml:lang");
}

Stanza &Stanza::setLang(const QString &ALang)
{
	setAttribute("xml:lang",ALang);
	return *this;
}

QDomElement Stanza::firstElement(const QString &ATagName, const QString &ANamespace) const
{
	return findElement(d->FDoc.documentElement(),ATagName,ANamespace);
}

QDomElement Stanza::addElement(const QString &ATagName, const QString &ANamespace)
{
	return d->FDoc.documentElement().appendChild(createElement(ATagName,ANamespace)).toElement();
}

QDomElement Stanza::createElement(const QString &ATagName, const QString &ANamespace)
{
	if (ANamespace.isEmpty())
		return d->FDoc.createElement(ATagName);
	else
		return d->FDoc.createElementNS(ANamespace,ATagName);
}

QDomText Stanza::createTextNode(const QString &AData)
{
	return d->FDoc.createTextNode(AData);
}

QString Stanza::toString(int AIndent) const
{
	QString str;
	QTextStream ts(&str, QIODevice::WriteOnly);
	element().save(ts, AIndent);
	return replaceInvalidXmlChars(str);
}

QByteArray Stanza::toByteArray() const
{
    return toString(0).toUtf8();
}

bool Stanza::isValidXmlChar(quint32 ACode)
{
	return IsValidXmlChar(ACode);
}

QString Stanza::replaceInvalidXmlChars(QString &AXml, const QChar &AWithChar)
{
	QChar quoteChar;
	bool inTag = false;
	bool inQuote = false;
	int xmlLength = AXml.length();
	for(int pos = 0; pos<xmlLength; ++pos)
	{
		QChar c = AXml.at(pos);
		if (c == '<')
		{
			inTag = true;
		}
		else if (c == '>')
		{
			if (!inQuote)
				inTag = false;
		}
		else if (c == '\'' || c == '\"')
		{
			if(inTag)
			{
				if(!inQuote)
				{
					inQuote = true;
					quoteChar = c;
				}
				else if (c == quoteChar)
				{
					inQuote = false;
				}
			}
		}

		if (inTag && !inQuote)
		{
			// don't replace invalid chars in element or attribute names
		}
		else if (IsValidXmlChar(c.unicode()))
		{
			// c is valid char
		}
		else if (c.isHighSurrogate() && pos+1<xmlLength && AXml.at(pos+1).isLowSurrogate())
		{
			// unicode surrogate pairs is always valid
			pos++;
		}
		else
		{
			// replace invalid char
			AXml[pos] = AWithChar;
		}
	}
// *** <<< eyeCU <<< ***
	AXml.replace(QChar::Nbsp, "&#160;");
	AXml.replace('\x09', "&#009;");
// *** >>> eyeCU >>> ***
	return AXml;
}

QDomElement Stanza::findElement(const QDomElement &AParent, const QString &ATagName, const QString &ANamespace)
{
	QDomElement elem = AParent.firstChildElement(ATagName);
	if (!ANamespace.isNull())
	{
		while (!elem.isNull() && elem.namespaceURI()!=ANamespace)
			elem = elem.nextSiblingElement(ATagName);
	}
	return elem;
}
