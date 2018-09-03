#include <QDebug>
#include <QDateTime>
#include <QStringList>

#include "jinglesession.h"
#include "jinglestanza.h"
#include "jingle.h"
#include <definitions/namespaces.h>
#include <utils/stanza.h>
#include <utils/logger.h>

QHash<QString, JingleSession*> JingleSession::FSessions;
Jingle* JingleSession::FJingle;

JingleSession::JingleSession(const Jid &AThisParty, const Jid &AOtherParty, const QString &AApplicationNS):
    QObject(FJingle->appByNS(AApplicationNS)->instance()),FValid(false), FOutgoing(true),
    FApplicationNamespace(AApplicationNS),FThisParty(AThisParty), FOtherParty(AOtherParty),
	FSid(getSid()), FActionId(), FAction(IJingle::NoAction), FReason(IJingle::NoReason)
{
	FSessions.insert(FSid, this);
    if (FThisParty.isValid() && FOtherParty.isValid())
    {
        FValid=true;
		connect(this,SIGNAL(sessionInitiated(QString)),
				parent(),SLOT(onSessionInitiated(QString)));
		connect(this,SIGNAL(sessionAccepted(QString)),
				parent(),SLOT(onSessionAccepted(QString)));
		connect(this,SIGNAL(sessionConnected(QString)),
				parent(),SLOT(onSessionConnected(QString)));
		connect(this,SIGNAL(sessionTerminated(QString,IJingle::SessionStatus,IJingle::Reason)),
				parent(),SLOT(onSessionTerminated(QString,IJingle::SessionStatus,IJingle::Reason)));
		connect(this,SIGNAL(sessionInformed(QDomElement)),parent(),
				SLOT(onSessionInformed(QDomElement)));
		connect(this,SIGNAL(actionAcknowledged(QString,IJingle::Action,IJingle::CommandRespond,IJingle::SessionStatus,Jid,IJingle::Reason)),
				parent(),SLOT(onActionAcknowledged(QString,IJingle::Action,IJingle::CommandRespond,IJingle::SessionStatus,Jid,IJingle::Reason)));
    }
}

JingleSession::JingleSession(const JingleStanza &AStanza):
    FValid(false),FOutgoing(false),FThisParty(AStanza.to()),FOtherParty(AStanza.from()),FActionId(),FAction(IJingle::NoAction)
{
    QDomElement jingle=AStanza.firstElement("jingle", NS_JINGLE);
    if (!jingle.isNull())
    {
        QString action=jingle.attribute("action");
        if (action=="session-initiate")
        {
            FSid=jingle.attribute("sid");
            for (QDomElement content=jingle.firstChildElement("content"); !content.isNull(); content=content.nextSiblingElement("content"))
            {
                QDomElement description=content.firstChildElement("description");
                if (!description.isNull())
                {
                    FApplicationNamespace = description.namespaceURI();
                    QDomElement transport=content.firstChildElement("transport");
                    if (!transport.isNull())
                    {
                        setParent(FJingle->appByNS(FApplicationNamespace)->instance());
						if (!addContent(content.attribute("name"), description, transport,
										content.attribute("initiator")==QString("responder")))
                            qWarning() << "addContent() failed!";
                        FValid=true;
						connect(this,SIGNAL(sessionAccepted(QString)),
								parent(), SLOT(onSessionAccepted(QString)));
						connect(this,SIGNAL(sessionTerminated(QString,IJingle::SessionStatus,
															  IJingle::Reason)),
								parent(),SLOT(onSessionTerminated(QString,IJingle::SessionStatus,
																  IJingle::Reason)));
						connect(this,SIGNAL(sessionInformed(QDomElement)),
								parent(),SLOT(onSessionInformed(QDomElement)));
						connect(this,SIGNAL(actionAcknowledged(QString,IJingle::Action,IJingle::CommandRespond,
															   IJingle::SessionStatus,Jid,IJingle::Reason)),
								parent(),SLOT(onActionAcknowledged(QString,IJingle::Action,IJingle::CommandRespond,
																   IJingle::SessionStatus,Jid,IJingle::Reason)));
                    }
                }
            }
			if (!FSessions.contains(FSid))
				FSessions.insert(FSid, this);
            else
                qWarning() << "Session exists!";
        }
    }
}

JingleSession::~JingleSession()
{
	for (QHash<QString, JingleContent *>::const_iterator it=FContents.constBegin(); it!=FContents.constEnd(); it++)
		delete (*it);
	FContents.clear();	

	if (FSessions.value(FSid)==this)
		FSessions.remove(FSid);        // remove it from the list!
}

void JingleSession::setInitiated(IJingleApplication *AApplication)
{
    FStatus=IJingle::Initiated;
    disconnect(parent());
    setParent(AApplication->instance());
	connect(this,SIGNAL(sessionInitiated(QString)),parent(),SLOT(onSessionInitiated(QString)));
	connect(this,SIGNAL(sessionAccepted(QString)),parent(),SLOT(onSessionAccepted(QString)));
	connect(this,SIGNAL(sessionConnected(QString)),parent(),SLOT(onSessionConnected(QString)));
	connect(this,SIGNAL(sessionTerminated(QString,IJingle::SessionStatus,IJingle::Reason)),parent(),SLOT(onSessionTerminated(QString,IJingle::SessionStatus,IJingle::Reason)));
    connect(this,SIGNAL(sessionInformed(QDomElement)),parent(),SLOT(onSessionInformed(QDomElement)));
	connect(this,SIGNAL(actionAcknowledged(QString,IJingle::Action,IJingle::CommandRespond,IJingle::SessionStatus,Jid,IJingle::Reason)),parent(),SLOT(onActionAcknowledged(QString,IJingle::Action,IJingle::CommandRespond,IJingle::SessionStatus,Jid,IJingle::Reason)));
	emit sessionInitiated(FSid);
}

void JingleSession::setAccepted()
{
    FStatus=IJingle::Accepted;
	emit sessionAccepted(FSid);
}

void JingleSession::setConnected()
{
	LOG_DEBUG("JingleSession::setConnected()");
	qDebug() << "JingleSession::setConnected()";
    FStatus=IJingle::Connected;
	emit sessionConnected(FSid);
}

void JingleSession::setTerminated(IJingle::Reason AReason)
{
	for (QHash<QString, JingleContent *>::ConstIterator it=FContents.constBegin(); it!=FContents.constEnd(); ++it)
		FJingle->freeIncomingTransport(*it);
    IJingle::SessionStatus currentStatus=FStatus;
    FStatus=IJingle::Terminated;
    FReason=AReason;
    deleteLater();
	emit sessionTerminated(FSid, currentStatus, AReason);
}

void JingleSession::inform(const JingleStanza &AStanza)
{
    emit sessionInformed(AStanza.jingleElement());
}

void JingleSession::acknowledge(IJingle::CommandRespond ARespond, Jid ARedirect)
{
    IJingle::SessionStatus currentStatus=FStatus;

    switch (FAction)
    {
        case IJingle::SessionInitiate:
            if (ARespond == IJingle::Acknowledge)
                FStatus = IJingle::Initiated;
            else
                deleteLater();
            break;

        case IJingle::SessionAccept:
            if (ARespond == IJingle::Acknowledge)
                FStatus = IJingle::Accepted;
            break;

        case IJingle::SessionTerminate:
            qWarning() << "Something wrong!!!";
            break;

        default:
            break;
    }
	emit actionAcknowledged(FSid, FAction, ARespond, currentStatus, ARedirect, FReason);
}

// Create jingle content from existing <description /> and <transport /> elements
JingleContent *JingleSession::addContent(const QString &AName,
										 const QDomElement &ADescription,
										 const QDomElement &ATransport,
										 bool AFromResponder)
{
	qDebug() << "JingleSession::addContent(" << AName << ", ...)";
    if (FContents.contains(AName))
    {
        qWarning() << "Content " << AName << "exists already!\nReturning NULL";
		return nullptr;
    }

    if (ADescription.namespaceURI()!=FApplicationNamespace)
    {
        qWarning() << "Content " << AName
                   << "has wrong namespace: " << ADescription.namespaceURI()
                   << "\nReturning NULL";
		return nullptr;
    }
	JingleContent *content = new JingleContent(AName, FSid, ADescription, ATransport, AFromResponder);
    FContents.insert(AName, content);
    return content;
}

// Create an empty jingle content with session application namespace
JingleContent *JingleSession::addContent(const QString &AName, const QString &AMediaType,
										 int AComponentCount, IJingleTransport *ATransport,
										 bool AFromResponder)
{
	qDebug() << "JingleSession::addContent(" << AName << "," << AMediaType << "," << AComponentCount << ", ...)";
    if (FContents.contains(AName))
		return nullptr;
	JingleContent *content=new JingleContent(AName, FSid, AComponentCount, FApplicationNamespace,
											 AMediaType, ATransport->ns(), AFromResponder);
	if (ATransport->fillIncomingTransport(content))
	{
		FContents.insert(AName, content);
		return content;
	}
	delete content;
	return nullptr;
}

JingleContent *JingleSession::getContent(QIODevice *AIODevice)
{
	return FContentByDevice.value(AIODevice);
}

bool JingleSession::deleteContent(const QString &AName)
{
    if (FContents.contains(AName))
    {
        JingleContent *content=FContents.take(AName);
        delete content;
        return true;
    }
    return false;
}

bool JingleSession::initiate()
{
	qDebug() << "JingleSession::initiate()";
    JingleStanza stanza(FThisParty, FOtherParty, FSid, IJingle::SessionInitiate);
    if (!FValid || FContents.isEmpty())    // Invalid session
        return false;
    stanza.setInitiator(FThisParty.full());
    for (QHash<QString, JingleContent *>::const_iterator it=FContents.constBegin(); it!=FContents.constEnd(); it++)
        (*it)->addElementToStanza(stanza);
    FActionId=stanza.id();
    FAction=IJingle::SessionInitiate;
	qDebug() << "stanza=" << stanza.toString();
    return FJingle->sendStanzaOut(stanza);
}

bool JingleSession::accept()
{
	qDebug() << "JingleSession::accept()";
    JingleStanza stanza(FThisParty, FOtherParty, FSid, IJingle::SessionAccept);

    if (!FValid || FContents.isEmpty())    // Invalid session
        return false;

    stanza.setResponder(FThisParty.full());
    for (QHash<QString, JingleContent *>::const_iterator it=FContents.constBegin(); it!=FContents.constEnd(); it++)
        (*it)->addElementToStanza(stanza);

    FActionId=stanza.id();
    FAction=IJingle::SessionAccept;
	qDebug() << "Sending output stanza:" << stanza.toString();
    return FJingle->sendStanzaOut(stanza);
}

bool JingleSession::terminate(IJingle::Reason AReason)
{
	qDebug() << "JingleSession::terminate(" << AReason << ")";
    JingleStanza stanza(FThisParty, FOtherParty, FSid, IJingle::SessionTerminate);
    FActionId=stanza.id();
    FAction=IJingle::SessionTerminate;
    stanza.setReason(AReason);
    if (FJingle->sendStanzaOut(stanza))
    {
		setTerminated(AReason);
        return true;
    }
    return false;
}

bool JingleSession::sendAction(IJingle::Action AAction, const QDomElement &AJingleElement)
{
    JingleStanza stanza(FThisParty, FOtherParty, FSid, AAction);
    FActionId=stanza.id();
    FAction=AAction;
    stanza.importJingleElement(AJingleElement);
    return FJingle->sendStanzaOut(stanza);
}

bool JingleSession::sendAction(IJingle::Action AAction, const QDomNodeList &AJingleElements)
{
    JingleStanza stanza(FThisParty, FOtherParty, FSid, AAction);
    FActionId=stanza.id();
    FAction=AAction;
    for (quint16 i=0; i<AJingleElements.length(); i++)
        stanza.importJingleElement(AJingleElements.at(i).toElement());
    return FJingle->sendStanzaOut(stanza);
}

bool JingleSession::selectTransportCandidate(const QString &AContentName, const QString &ACandidateId)
{
    if (FContents.contains(AContentName))
        return FContents[AContentName]->chooseCandidate(ACandidateId);
    return false;
}

const QHash<QString, JingleContent *> JingleSession::contents() const
{
	return FContents;
}

JingleSession *JingleSession::sessionByStanzaId(const QString &AId)
{
	for (QHash<QString, JingleSession*>::iterator it=FSessions.begin(); it!=FSessions.end(); ++it)
		if ((*it)->FActionId==AId)
			return *it;
	return nullptr;
}

QString JingleSession::getSid()
{
    QString sid;
    uint dt=QDateTime::currentDateTime().toTime_t();	
	for (sid=QString("jingleSid%1").arg(dt, 0, 16); FSessions.contains(sid);
		 sid=QString("jingleSid%1").arg(++dt, 0, 16))
		;
    return sid;
}

void JingleSession::setJingle(Jingle *AJingle)
{
    FJingle=AJingle;
}

IJingle::SessionState JingleSession::state() const
{
    switch (FStatus)
    {
        case IJingle::Initiated:
        case IJingle::Accepted:
            return IJingle::Pending;
        case IJingle::Connected:
            return IJingle::Active;
        case IJingle::Terminated:
            return IJingle::Ended;
        default:
            return IJingle::NoState;
    }
}

// ------------------- JingleContent ---------------------
JingleContent::JingleContent(const QString &AName, const QString &ASid, const QDomElement &ADescription, const QDomElement &ATransport, bool AFromResponder):
	FName(AName),FSid(ASid),FContentFromResponder(AFromResponder),FComponentCount(0)
{
    FDescription=FDocument.importNode(ADescription, true).toElement();
    FTransportOutgoing=FDocument.importNode(ATransport, true).toElement();
    FTransportIncoming=FTransportOutgoing.cloneNode(false).toElement();

	for (QDomElement e=FTransportOutgoing.firstChildElement("candidate");
		 !e.isNull(); e=e.nextSiblingElement("candidate"))
	{
		int comp = e.attribute("component").toInt();
		if (FComponentCount < comp)
			FComponentCount = comp;
	}
}

JingleContent::JingleContent(const QString &AName, const QString &ASid, int AComponentCount,
							 const QString &AApplicationNameSpace, const QString &AMediaType,
							 const QString &ATransportNS, bool AFromResponder):
	FName(AName),FSid(ASid),FContentFromResponder(AFromResponder),FComponentCount(AComponentCount),
    FDescription(FDocument.createElementNS(AApplicationNameSpace, "description")),
	FTransportIncoming(FDocument.createElementNS(ATransportNS, "transport"))
{
	qDebug() << "JingleContent(" << AName << "," << ASid << "," << AComponentCount << ", ...)";
    FDescription.setAttribute("media", AMediaType);
}

JingleContent::~JingleContent() // Cleanup device list
{
	JingleSession *session = JingleSession::sessionBySessionId(FSid);
	if (session)
	{
		for(QMap<int, QIODevice*>::ConstIterator it = FIODevices.constBegin();
			it!=FIODevices.constEnd(); it++)
		{
			(*it)->deleteLater();
			session->FContentByDevice.remove(*it);
		}
	}
	else
		qWarning(QString("Session not found: %1").arg(FSid).toLatin1().data());
}

QDomElement JingleContent::addElementToStanza(JingleStanza &AStanza)
{
    QDomElement content=AStanza.createJingleElement("content");
    content.setAttribute("creator", FContentFromResponder?"responder":"initiator");
    content.setAttribute("name", FName);
    content.appendChild(AStanza.document().importNode(FDescription, true).toElement());
    content.appendChild(AStanza.document().importNode(FTransportIncoming, true).toElement());
    return content;
}

QStringList JingleContent::candidateIds() const
{
    QDomNodeList candidateList = FTransportOutgoing.elementsByTagName("candidate");
    QStringList  ids;
	for (int i=0; i<candidateList.length(); i++)
        ids.append(candidateList.item(i).toElement().attribute("id"));
    return ids;
}

const QDomElement JingleContent::candidate(const QString &AId) const
{
    QDomNodeList candidateList = FTransportOutgoing.elementsByTagName("candidate");
	for (int i=0; i<candidateList.length(); i++)
    {
        QDomElement element=candidateList.item(i).toElement();
        if (element.attribute("id")==AId)
            return element;
    }
    return QDomElement();
}

QList<QDomElement> JingleContent::candidates() const
{
    QDomNodeList candidateList=transportIncoming().elementsByTagName("candidate");
    QList<QDomElement> list;
	for (int i=0; i<candidateList.length(); i++)
        list.append(candidateList.item(i).toElement());
    return list;
}

bool JingleContent::chooseCandidate(const QString &AId)
{
    QDomNodeList candidates=FTransportIncoming.elementsByTagName("candidate");
    QDomElement found;
	for (int i=0; i<candidates.length(); i++)
    {
        QDomElement element=candidates.item(i).toElement();
        if (element.attribute("id")==AId)
        {
            found=element;
            break;
        }
    }

    if (found.isNull())
        return false;

	for (int i=0; i<candidates.length(); i++)
    {
        QDomElement element=candidates.item(i).toElement();
        if (element!=found)
            FTransportIncoming.removeChild(element);
    }
    return true;
}

bool JingleContent::enumerateCandidates()
{
    for (QDomElement c=FTransportOutgoing.firstChildElement("candidate"); !c.isNull(); c=c.nextSiblingElement("candidate")) // Iterate thru candidates
    {
        QString id=c.attribute("id");
        if (!id.isNull())
            FTransportCandidates.insert(id.toLong(), c);
    }

    if (!FTransportCandidates.isEmpty())
    {
        FTransportCandidateItreator=FTransportCandidates.constBegin();
        return true;
    }
    return false;
}

QDomElement JingleContent::nextCandidate()
{
    if (!FTransportCandidates.isEmpty() && FTransportCandidateItreator!=FTransportCandidates.constEnd())
        return *(FTransportCandidateItreator++);
    else
        return QDomElement();
}

QString JingleContent::transportNS() const
{
    return FTransportIncoming.namespaceURI();
}

bool JingleContent::setOutgoingTransport(const QDomElement &ATransport)
{
    if (ATransport.namespaceURI() == FTransportIncoming.namespaceURI())
    {
        FTransportOutgoing = FDocument.importNode(ATransport, true).toElement();
        return true;
    }
    else
		return false;
}

int JingleContent::componentCount() const
{
	return FComponentCount;
}

int JingleContent::component(QIODevice *ADevice) const
{
	return FIODevices.key(ADevice, 0);
}

bool JingleContent::setIoDevice(int AComponentId, QIODevice *ADevice)
{
	if (AComponentId <1 || AComponentId > FComponentCount)
		return false;
	if (FIODevices.value(AComponentId) != ADevice)
    {
		JingleSession *session = JingleSession::sessionBySessionId(FSid);
		if (session)
		{
			if (FIODevices.contains(AComponentId))
			{
				QIODevice *device = FIODevices.take(AComponentId);
				session->FContentByDevice.remove(device);
				device->deleteLater();
			}

			if (ADevice)
			{
				FIODevices.insert(AComponentId, ADevice);
				session->FContentByDevice.insert(ADevice, this);
			}
		}
		else
		{
			qWarning(QString("Session not found: %1").arg(FSid).toLatin1().data());
			return false;
		}
    }
	return true;
}
