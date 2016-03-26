#ifndef CHATSTATES_H
#define CHATSTATES_H

#include <QMap>
#include <QTimer>
#include <interfaces/ipluginmanager.h>
#include <interfaces/ichatstates.h>
#include <interfaces/ipresencemanager.h>
#include <interfaces/istanzaprocessor.h>
#include <interfaces/imessagewidgets.h>
#include <interfaces/ioptionsmanager.h>
#include <interfaces/iservicediscovery.h>
#include <interfaces/imessagearchiver.h>
#include <interfaces/idataforms.h>
#include <interfaces/inotifications.h>
#include <interfaces/isessionnegotiation.h>
#include <interfaces/imultiuserchat.h>
#include <interfaces/irostersview.h>
#include <utils/message.h>
#include "statewidget.h"

struct UserParams {
	UserParams() {
		state = IChatStates::StateUnknown;
		notifyId = 0;
	}
	int state;
	int notifyId;
};

struct SelfParams {
	SelfParams() {
		state = IChatStates::StateUnknown;
		lastActive = 0;
	}
	int state;
	uint lastActive;
};

struct ChatParams {
	ChatParams() {
		canSendStates = false;
	}
	SelfParams self;
	UserParams user;
	bool canSendStates;
};

struct RoomParams {
	RoomParams() {
		notifyId = 0;
	}
	int notifyId;
	SelfParams self;
	QHash<Jid, UserParams> user;
};

class ChatStates :
	public QObject,
	public IPlugin,
	public IChatStates,
	public IStanzaHandler,
	public IArchiveHandler,
	public IOptionsDialogHolder,
	public ISessionNegotiator
{
	Q_OBJECT;
	Q_INTERFACES(IPlugin IChatStates IStanzaHandler IArchiveHandler IOptionsDialogHolder ISessionNegotiator);
public:
	ChatStates();
	~ChatStates();
	//IPlugin
	virtual QObject *instance() { return this; }
	virtual QUuid pluginUuid() const { return CHATSTATES_UUID; }
	virtual void pluginInfo(IPluginInfo *APluginInfo);
	virtual bool initConnections(IPluginManager *APluginManager, int &AInitOrder);
	virtual bool initObjects();
	virtual bool initSettings();
	virtual bool startPlugin();
	//IArchiveHandler
	virtual bool archiveMessageEdit(int AOrder, const Jid &AStreamJid, Message &AMessage, bool ADirectionIn);
	//IOptionsHolder
	virtual QMultiMap<int, IOptionsDialogWidget *> optionsDialogWidgets(const QString &ANodeId, QWidget *AParent);
	//ISessionNegotiator
	virtual int sessionInit(const IStanzaSession &ASession, IDataForm &ARequest);
	virtual int sessionAccept(const IStanzaSession &ASession, const IDataForm &ARequest, IDataForm &ASubmit);
	virtual int sessionApply(const IStanzaSession &ASession);
	virtual void sessionLocalize(const IStanzaSession &ASession, IDataForm &AForm);
	//IStanzaHandler
	virtual bool stanzaReadWrite(int AHandlerId, const Jid &AStreamJid, Stanza &AStanza, bool &AAccept);
	//IChatStates
	virtual bool isReady(const Jid &AStreamJid) const;
	virtual int permitStatus(const Jid &AContactJid) const;
	virtual void setPermitStatus(const Jid &AContactJid, int AStatus);
	virtual bool isEnabled(const Jid &AContactJid, const Jid &AStreamJid = Jid::null) const;
	virtual bool isSupported(const Jid &AStreamJid, const Jid &AContactJid) const;
	virtual int userChatState(const Jid &AStreamJid, const Jid &AContactJid) const;
	virtual int selfChatState(const Jid &AStreamJid, const Jid &AContactJid) const;
	virtual int userRoomState(const Jid &AStreamJid, const Jid &AUserJid) const;
	virtual int selfRoomState(const Jid &AStreamJid, const Jid &ARoomJid) const;
signals:
	void permitStatusChanged(const Jid &AContactJid, int AStatus) const;
	void supportStatusChanged(const Jid &AStreamJid, const Jid &AContactJid, bool ASupported) const;
	void userChatStateChanged(const Jid &AStreamJid, const Jid &AContactJid, int AState) const;
	void selfChatStateChanged(const Jid &AStreamJid, const Jid &AContactJid, int AState) const;
	void userRoomStateChanged(const Jid &AStreamJid, const Jid &AUserJid, int AState) const;
	void selfRoomStateChanged(const Jid &AStreamJid, const Jid &ARoomJid, int AState) const;
protected:
	void registerDiscoFeatures();
	QString stateCodeToTag(int AState) const;
	int stateTagToCode(const QString &AState) const;
	void sendStateMessage(Message::MessageType AType, const Jid &AStreamJid, const Jid &AContactJid, int AState) const;
protected:
	bool isChatCanSend(const Jid &AStreamJid, const Jid &AContactJid) const;
	void resetSupported(const Jid &AContactJid = Jid::null);
	void setSupported(const Jid &AStreamJid, const Jid &AContactJid, bool ASupported);
	void setChatUserState(const Jid &AStreamJid, const Jid &AContactJid, int AState);
	void setChatSelfState(const Jid &AStreamJid, const Jid &AContactJid, int AState, bool ASend = true);
	void notifyChatState(const Jid &AStreamJid, const Jid &AContactJid);
protected:
	bool isRoomCanSend(const Jid &AStreamJid, const Jid &ARoomJid) const;
	void setRoomUserState(const Jid &AStreamJid, const Jid &AUserJid, int AState);
	void setRoomSelfState(const Jid &AStreamJid, const Jid &ARoomJid, int AState, bool ASend = true);
	void notifyUserState(const Jid &AStreamJid, const Jid &AUserJid);
	void notifyRoomState(const Jid &AStreamJid, const Jid &ARoomJid);
protected slots:
	void onPresenceOpened(IPresence *APresence);
	void onPresenceItemReceived(IPresence *APresence, const IPresenceItem &AItem, const IPresenceItem &ABefore);
	void onPresenceClosed(IPresence *APresence);
protected slots:
	void onChatWindowCreated(IMessageChatWindow *AWindow);
	void onChatWindowActivated();
	void onChatWindowTextChanged();
	void onChatWindowDestroyed(IMessageChatWindow *AWindow);
protected slots:
	void onMultiChatWindowCreated(IMultiUserChatWindow *AWindow);
	void onMultiChatWindowActivated();
	void onMultiChatWindowTextChanged();
	void onMultiChatUserChanged(IMultiUser *AUser, int AData, const QVariant &ABefore);
	void onMultiChatWindowDestroyed(IMultiUserChatWindow *AWindow);
protected slots:
	void onUpdateSelfStates();
	void onOptionsOpened();
	void onOptionsClosed();
	void onOptionsChanged(const OptionsNode &ANode);
	void onStanzaSessionTerminated(const IStanzaSession &ASession);
private:
	IDataForms *FDataForms;
	IServiceDiscovery *FDiscovery;
	INotifications *FNotifications;
	IMessageWidgets *FMessageWidgets;
	IOptionsManager *FOptionsManager;
	IPresenceManager *FPresenceManager;
	IStanzaProcessor *FStanzaProcessor;
	IMessageArchiver *FMessageArchiver;
	ISessionNegotiation *FSessionNegotiation;
	IMultiUserChatManager *FMultiChatManager;
private:
	QMap<Jid, int> FSHIChatMessagesIn;
	QMap<Jid, int> FSHIChatMessagesOut;
	QMap<Jid, int> FSHIRoomMessagesIn;
	QMap<Jid, int> FSHIRoomMessagesOut;
private:
	QTimer FUpdateTimer;
	QMap<Jid, int> FPermitStatus;
private:
	QMap<Jid, QList<Jid> > FNotSupported;
	QMap<Jid, QMap<Jid, ChatParams> > FChatParams;
	QMap<Jid, QMap<Jid, QString> > FStanzaSessions;
	QMap<QTextEdit *, IMessageChatWindow *> FChatByEditor;
private:
	QMap<Jid, QMap<Jid, RoomParams> > FRoomParams;
	QMap<QTextEdit *, IMultiUserChatWindow *> FRoomByEditor;
};

#endif // CHATSTATES_H
