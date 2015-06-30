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
    enum {Page_Intro,Page_Networks,Page_Gateway,Page_Process,Page_Result,Page_Conclusion};
	explicit TransportWizard(const Jid &AStreamJid, IRegistration *ARegistration, IServiceDiscovery *AServiceDiscovery, QWidget *parent = 0);
signals:
    void getGateway();
    void getRegister();
protected slots:
    void onCurIdChange(int id);
protected:
    void accept();
private:
    Jid             FStreamJid;
    IRegistration   *FRegistration;
};

//!---------------------
class IntroPage : public QWizardPage
{
    Q_OBJECT
public:
    IntroPage(QWidget *parent = 0);
    int nextId() const;
};

//!---------------------
class NetworksPage : public QWizardPage
{
    Q_OBJECT
public:
    NetworksPage(QWidget *parent = 0);
	QString networkName(const QString &ANetworkId) const {return FNetworkHash.value(ANetworkId);}
	// QWizardPage interface
    int nextId() const;
	void initializePage();

protected:
    void loadNetworksList();

private:
	SelectableTreeWidget	*FNetworksList;
	IconStorage				*FIconStorage;
	QHash<QString, QString> FNetworkHash;
};

//!---------------------
class GatewayPage : public QWizardPage
{
    Q_OBJECT
public:
	GatewayPage(IServiceDiscovery *AServiceDiscovery, const Jid &AStreamJid, QWidget *parent = 0);
    QList <QDomElement> getExcepFields(){return FExcepFields; }

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
    void loadGatewayList();
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
	QList<QDomElement>	FExcepFields;
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
	IRegisterSubmit getSubmit(){doSubmit(); return FSubmit;}

	// QWizardPage interface
	void initializePage();
    int nextId() const;

public slots:
    void createGateway();
    void doSubmit();

protected:
	QWidget	*getWidget(const IDataField &AField);
	void	localTextLabel();
	QString	getLocalText(QString AKey);
	bool	checkField(const IDataField AField, QString AGateWay);

protected slots:
    void onRegisterFields(const QString &AId, const IRegisterFields &AFields);
    void onRegisterError(const QString &AId, const XmppError &AError);
    void onClicked(bool st);
    void onTextChanged(QString text);
    void onComBoxChanged(QString text);
    void onFormClicked(bool st);
    void onMultiTextChanged();
    void onUserEditChanged(QString text);
    void onPassEditChanged(QString text);
    void onEmailEditChanged(QString text);
    void onUrlEditChanged(QString text);
    void onListMultiChanged(QString text);
	void onLinkActivated();

private:
    Jid         FStreamJid;
    IRegistration *FRegistration;
	GatewayPage	*FGatewayPage;
	QScrollArea *FScrollArea;
    QGridLayout *FGridLayout;
	QCheckBox   *FAutoRegCheckBox;

    QLabel      *FInstrLabel;
    QLabel      *FErrorLabel;
    Jid         FServiceTo;
    QString     FRequestId;
    IRegisterSubmit FSubmit;
    bool        FDirection;
	IDataForm	FForm;
	QHash<QString,QVariant> FTmpFields;
    QString     FUserName;
    QString     FPassword;
    QString     FEmail;
    QString     FUrl;
	QList <QDomElement>	FExcepFields;
    QHash<QString,QString> FLocalText;
	const int	FFieldWidth;
};

//!-------ResultPage--------------
class ResultPage : public QWizardPage
{
    Q_OBJECT
public:
    ResultPage(Jid &AStreamJid, IRegistration *ARegistration,ProcessPage *AProcess,QWidget *parent = 0);
    int nextId() const;
    bool isComplete() const;
public slots:
    void onGetRegister();
protected slots:
    void onRegisterError(const QString &AId, const XmppError &AError);
    void onRegisterSuccessful(const QString &AId);
private:
    QVBoxLayout *layout;
    QLabel          *FErrorLabel;
    Jid             FStreamJid;
    IRegistration   *FRegistration;
    IRegisterSubmit FSubmit;
    ProcessPage     *FProcess;
    QString         FRequestId;
    bool            FWizardGo;
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
