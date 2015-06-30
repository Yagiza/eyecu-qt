#ifndef JINGLESESSION_H
#define JINGLESESSION_H

#include <QString>
#include <QDomDocument>
#include <QDebug>
#include <utils/jid.h>
#include <interfaces/ijingle.h>
#include "jinglestanza.h"

class JingleSession;
class Jingle;

class JingleContent: public IJingleContent
{
friend class JingleSession;
public:
    const QString &name() const {return FName;}
    const QString &sid() const {return FSid;}
    const QString &streamJid() const {return FStreamJid;}
    bool  fromResponder() const {return FContentFromResponder;}
    QStringList candidateIds() const;
    const QDomElement candidate(const QString &AId) const;    
    QList<QDomElement> candidates() const;

    bool chooseCandidate(const QString &AId);
    bool enumerateCandidates();
    QDomElement nextCandidate();

    QString transportNS() const;
    const QDomElement &description() const {return FDescription;}
    const QDomElement &transportOutgoing() const {return FTransportOutgoing;}
    const QDomElement &transportIncoming() const {return FTransportIncoming;}
    const QDomDocument &document() const {return FDocument;}
    bool        setOutgoingTransport(const QDomElement &ATransport);
    QIODevice * inputDevice(const QString &AId) const {return FInputDevices.value(AId);}
    void        setInputDevice(const QString &AId, QIODevice *ADevice);
    QIODevice * outputDevice(const QString &AId) const {return FOutputDevices.value(AId);}
    void        setOutputDevice(const QString &AId, QIODevice *ADevice);

protected:
    JingleContent(const QString &AName, const QString &ASid, const QString &AStreamJid, const QDomElement &ADescription, const QDomElement &ATransport, bool AFromResponder);
    JingleContent(const QString &AName, const QString &ASid, const QString &AStreamJid, const QString &AApplicationNameSpace, const QString &AMediaType, const QString &ATransportNameSpace, bool AFromResponder);
    ~JingleContent();
    QDomElement     addElementToStanza(JingleStanza &AStanza);

private:    
    QString         FName;
    QString         FSid;
    QString         FStreamJid; //! Temporary
    bool            FContentFromResponder;
    QDomDocument    FDocument;
    QDomElement     FDescription;
    QDomElement     FTransportOutgoing;
    QDomElement     FTransportIncoming;
    QHash<QString, QIODevice*> FInputDevices;
    QHash<QString, QIODevice*> FOutputDevices;
    QMap<long, QDomElement> FTransportCandidates;
    QMap<long, QDomElement>::ConstIterator FTransportCandidateItreator;
};

class JingleSession: public QObject
{
    Q_OBJECT

public:
    enum Direction
    {
        Incoming,
        Outgoing
    };

    JingleSession(const Jid &AThisParty, const Jid &AOtherParty, const QString &AApplicationNS);
    JingleSession(const JingleStanza &AStanza);
    ~JingleSession();

    void setInitiated(IJingleApplication *AApplication);
    void setAccepted();
    void setConnected();
    void setTerminated(IJingle::Reason AReason);

    void inform(const JingleStanza &AStanza);
    void acknowledge(IJingle::CommandRespond ARespond, Jid ARedirect);
    bool initiate();    
    bool accept();
    bool terminate(IJingle::Reason AReason);

    bool sendAction(IJingle::Action AAction, const QDomElement &AJingleElement);
    bool sendAction(IJingle::Action AAction, const QDomNodeList &AJingleElements);

    bool selectTransportCandidate(const QString &AContentName, const QString &ACandidateId);

    bool isOk() const { return FSessions.value(FThisParty).value(FSid)==this;}
    bool isValid() const {return FValid;}

    const Jid   &thisParty() const {return FThisParty;}
    const Jid   &otherParty() const {return FOtherParty;}
    const Jid   &initiator() const {return FOutgoing?FThisParty:FOtherParty;}
    const Jid   &responder() const {return FOutgoing?FOtherParty:FThisParty;}
    QDomElement &transport(const QString &AContent) {return FContents[AContent]->FTransportIncoming;}
    QDomElement &description(const QString &AContent) {return FContents[AContent]->FDescription;}

    static JingleSession* sessionBySessionId(const Jid &AStreamJid, const QString &ASid) {return FSessions.contains(AStreamJid)?FSessions[AStreamJid].value(ASid):NULL;}
    static JingleSession* sessionByStanzaId(const Jid &AStreamJid, const QString &AId);
    static void setJingle(Jingle *AJingle);

    QObject *instance() {return this;}
    const QString &applicationNS() const {return FApplicationNamespace;}
    const QString &sid() const {return FSid;}

    const QString &lastActionId() const {return FActionId;}
    bool  isOutgoing() const {return FOutgoing;}
    bool  isConnected() const {return FConnected;}
    IJingle::SessionState state() const;
    IJingle::SessionStatus status() const {return FStatus;}
    IJingle::Action lastAction() const {return FAction;}

    JingleContent *addContent(const QString &AName, const QDomElement &ADescription, const QDomElement &ATransport, bool AFromResponder);
    JingleContent *addContent(const QString &AName, const QString &AMediaType, const QString &ATransportNameSpace, bool AFromResponder);
    JingleContent *getContent(const QString &AName) const {return FContents.value(AName);}
    bool deleteContent(const QString &AName);    

    const QHash<QString, JingleContent *> contents() const;

protected:
    static QString getSid(const Jid &AStreamJid);

protected slots:
    void onTimeout();
    void onDeviceReadyRead();

signals:
    void sessionInitiated(const Jid &AStreamJid, const QString &ASid);
    void sessionAccepted(const Jid &AStreamJid, const QString &ASid);
    void sessionConnected(const Jid &AStreamJid, const QString &ASid);    
    void sessionTerminated(const Jid &AStreamJid, const QString &ASid, IJingle::SessionStatus APreviousStatus, IJingle::Reason AReason);
    void sessionInformed(const QDomElement &AInfoElement);
    void receivingData(const Jid &AStreamJid, const QString &ASid);
    void actionAcknowledged(const Jid &AStreamJid, const QString &ASid, IJingle::Action AAction, IJingle::CommandRespond ARespond, IJingle::SessionStatus APreviousStatus, Jid ARedirectJid, IJingle::Reason AReason);

private:
    bool        FValid;
    bool        FInitiated;
    bool        FConnected;
    const bool  FOutgoing;
    IJingle::SessionStatus FStatus;
    QString     FApplicationNamespace;
    const Jid   FThisParty;
    const Jid   FOtherParty;
    QString     FSid;
    QString     FActionId;
    IJingle::Action FAction;
    IJingle::Reason FReason;
    QHash<QString, JingleContent *> FContents;

    static QHash<Jid, QHash<QString, JingleSession*> >  FSessions;
    static Jingle   *FJingle;
};

#endif // JINGLESESSION_H