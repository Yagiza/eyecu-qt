#ifndef WIZARDPAGES_H
#define WIZARDPAGES_H

#include <QWizard>
#include <QComboBox>
#include <QMetaType>
#include <QCheckBox>
#include <QTreeWidget>
#include <QLabel>
#include <QGroupBox>
#include <QRadioButton>
#include <QCheckBox>
#include <QCommandLinkButton>
#include <QListWidget>
#include <SelectableTreeWidget>
#include <QTextBrowser>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QSpacerItem>
#include <QStyle>

#include <interfaces/iregistraton.h>
#include <interfaces/iservicediscovery.h>
#include <utils/iconstorage.h>

class TransportWizard : public QWizard
{
	Q_OBJECT
public:
	enum {Page_Intro,Page_Transports,Page_Networks,Page_Gateway,Page_Process,Page_Result,Page_Conclusion};
	TransportWizard(const Jid &AStreamJid, const Jid &ATransportJid = Jid::null, QWidget *Parent = 0);
	void setAutoSubscribe(bool AAutoSubscribe) {FAutoSubscribe = AAutoSubscribe;}

protected slots:
	void onFinished(int AStatus);

private:
	Jid             FStreamJid;
	bool			FAutoSubscribe;
};

//!---------------------
class IntroPage : public QWizardPage
{
	Q_OBJECT
public:
	IntroPage(QWidget *parent = 0);
	// QWizardPage interface
	int nextId() const {return FNextId;}
	bool isComplete() const {return false;}

protected slots:
	void onClicked();

private:
	QCommandLinkButton	*FClbConnectLegacyNetwork;
	QCommandLinkButton	*FClbChangeTransport;
	int					FNextId;
};

//!---------------------
class TransportsPage : public QWizardPage
{
	Q_OBJECT
public:
	TransportsPage(const Jid &AStreamJid, const IServiceDiscovery *AServiceDiscovery, QWidget *parent = 0);
	// QWizardPage interface
	int nextId() const;
	void initializePage();

protected slots:
	void onTransportSelected(const QString &ATransportJid);

private:
	const IServiceDiscovery *FServiceDiscovery;
	const Jid				FStreamJid;
	SelectableTreeWidget	*FTransportsList;
	IconStorage				*FIconStorage;
};

//!---------------------
class NetworksPage : public QWizardPage
{
	Q_OBJECT
public:
	NetworksPage(QWidget *parent = 0);
	QString networkName(const QString &ANetworkId) const {return FNetworkNames.value(ANetworkId);}
	// QWizardPage interface
	int nextId() const;

protected:
	void loadNetworksList();

private:
	SelectableTreeWidget	*FNetworksList;
	IconStorage				*FIconStorage;
	QHash<QString, QString>	FNetworkNames;
	QHash<QString,QString>	FNetworkDescriptions;
};

//!---------------------
class GatewayPage : public QWizardPage
{
	Q_OBJECT
public:
	GatewayPage(const Jid &AStreamJid, IServiceDiscovery *AServiceDiscovery, bool ATransport, QWidget *parent = 0);
	const QHash<Jid, QHash<QString, QHash<QString, QString> > > &exceptFields() const {return FExceptFields;}

	// QWizardPage interface
	void initializePage();
	bool validatePage();
	int nextId() const {return TransportWizard::Page_Process;}

protected:
	enum TransportStatus
	{
		Unavailable,
		Available,
		Unknown
	};

	void setItemStatus(QTreeWidgetItem *AItem, TransportStatus AStatus);
	void loadTransportList();
	void appendLocalTransport(const IDiscoInfo &ADiscoInfo); //TODO: Get rid of it

protected slots:
	void onDiscoInfoReceived(const IDiscoInfo &ADiscoInfo);
	void onDiscoItemsReceived(const IDiscoItems &ADiscoItems);

private:
	QLabel				*FlblGatewaysList;
	SelectableTreeWidget *FTransportList;
	IServiceDiscovery	*FServiceDiscovery;
	IconStorage			*FIconStorageWizards;
	IconStorage			*FIconStorageServices;
	IconStorage			*FIconStorageMenu;
	QHash<Jid, QHash<QString, QHash<QString, QString> > > FExceptFields;
	IDiscoItems			FDiscoItems;
	QList<Jid>			FPendingItems;
	QList<Jid>			FPendingItemsListed;
	const Jid			FStreamJid;
	QString				FNetwork;
};

//! ---Page_Process----
class ProcessPage : public QWizardPage
{
	Q_OBJECT
public:
	ProcessPage(Jid &AStreamJid, IRegistration *ARegistration,GatewayPage *AGatewayPage ,QWidget *parent = 0);
	void createGateway();
	IRegisterSubmit getSubmit();

	// QWizardPage interface
	void initializePage();
	int nextId() const;

protected:
	void	addWidget(const IDataField &AField, const Jid &ATransportJid, QVBoxLayout *ALayout, bool ADisable);
	void	localTextLabel();
	QString	getLocalText(QString AKey);

protected slots:
	void onRegisterFields(const QString &AId, const IRegisterFields &AFields);
	void onRegisterError(const QString &AId, const XmppError &AError);
	void onTextChanged(QString AText);
	void onComBoxChanged(QString AText);
	void onCheckBoxClicked(bool AState);
	void onMultiTextChanged();
	void onListMultiSelectionChanged();
	void onOldFieldsReceived();
	void onOpenPage();

signals:
	void oldFieldsReceived();

private:
	Jid         FStreamJid;
	IRegistration *FRegistration;
	GatewayPage	*FGatewayPage;
	QScrollArea *FScrollArea;
	QVBoxLayout *FLayout;
	QCheckBox   *FAutoRegCheckBox;

	QLabel      *FInstrLabel;
	QLabel      *FErrorLabel;
	Jid         FServiceFrom;
	Jid         FServiceTo;
	QString     FRequestIdTo;
	QString     FRequestIdFrom;
	IRegisterSubmit FSubmit;
	bool        FHasForm;
	IDataForm	FForm;

	QHash<QString,QVariant> FTmpFields;

	QHash<Jid, QHash<QString, QHash<QString, QString> > > FExceptFields;

	QHash<QString,QString> FLocalText;
	IRegisterFields FOldFields;
	bool		FOldFieldsReceived;
	bool		FNewFieldsReceived;
	const int	FFieldWidth;
};

//!-------ResultPage--------------
class ResultPage : public QWizardPage
{
	Q_OBJECT
public:
	ResultPage(Jid &AStreamJid, IRegistration *ARegistration, ProcessPage *AProcess, QWidget *parent = 0);

	// QWizardPage interface
	int nextId() const {return FNextId;}
	bool isComplete() const;
	void initializePage();

protected slots:
	void onRegisterError(const QString &AId, const XmppError &AError);
	void onRegisterSuccessful(const QString &AId);
private:
	QVBoxLayout		*FLayout;
	QLabel          *FErrorLabel;
	Jid             FStreamJid;
	IRegistration   *FRegistration;
	IRegisterSubmit FSubmit;
	ProcessPage     *FProcess;
	QString         FRequestId;
	bool            FWizardGo;
	int				FNextId;
};

//!---------------------
class ConclusionPage : public QWizardPage
{
	Q_OBJECT
public:
	ConclusionPage(NetworksPage *ANetworkPage, QWidget *parent = 0);
	void initializePage();
	int nextId() const;
private:
	QLabel *FLblTitle;
	QLabel *FLblText1;
	QLabel *FLblText2;
	QLabel *FLblText3;
	NetworksPage *FNetworkPage;
};

#endif // WIZARDPAGES_H
