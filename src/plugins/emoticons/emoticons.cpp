#include "emoticons.h"

#include <QSet>

#define SVN_SUBSTORAGES                 "substorages"

Emoticons::Emoticons()
{
  FMessageWidgets = NULL;
  FMessageProcessor = NULL;
  FSettingsPlugin = NULL;
}

Emoticons::~Emoticons()
{

}

void Emoticons::pluginInfo(IPluginInfo *APluginInfo)
{
  APluginInfo->name = tr("Emoticons"); 
  APluginInfo->description = tr("Insert smiley icons in chat windows");
  APluginInfo->version = "1.0";
  APluginInfo->author = "Potapov S.A. aka Lion";
  APluginInfo->homePage = "http://jrudevels.org";
}

bool Emoticons::initConnections(IPluginManager *APluginManager, int &/*AInitOrder*/)
{
  IPlugin *plugin = APluginManager->pluginInterface("IMessageProcessor").value(0,NULL);
  if (plugin)
  {
    FMessageProcessor = qobject_cast<IMessageProcessor *>(plugin->instance());
  }

  plugin = APluginManager->pluginInterface("IMessageWidgets").value(0,NULL);
  if (plugin)
  {
    FMessageWidgets = qobject_cast<IMessageWidgets *>(plugin->instance());
    if (FMessageWidgets)
    {
      connect(FMessageWidgets->instance(),SIGNAL(toolBarWidgetCreated(IToolBarWidget *)),SLOT(onToolBarWidgetCreated(IToolBarWidget *)));
    }
  }

  plugin = APluginManager->pluginInterface("ISettingsPlugin").value(0,NULL);
  if (plugin) 
  {
    FSettingsPlugin = qobject_cast<ISettingsPlugin *>(plugin->instance());
    if (FSettingsPlugin)
    {
      connect(FSettingsPlugin->instance(),SIGNAL(settingsOpened()),SLOT(onSettingsOpened()));
      connect(FSettingsPlugin->instance(),SIGNAL(settingsClosed()),SLOT(onSettingsClosed()));
    }
  }

  return true;
}

bool Emoticons::initObjects()
{
  if (FMessageProcessor)
  {
    FMessageProcessor->insertMessageWriter(this,MWO_EMOTICONS);
  }

  if (FSettingsPlugin != NULL)
  {
    FSettingsPlugin->openOptionsNode(ON_EMOTICONS ,tr("Emoticons"),tr("Select emoticons files"),MNI_EMOTICONS,ONO_EMOTICONS);
    FSettingsPlugin->insertOptionsHolder(this);
  }

  return true;
}

void Emoticons::writeMessage(Message &AMessage, QTextDocument *ADocument, const QString &ALang, int AOrder)
{
  Q_UNUSED(AMessage); Q_UNUSED(ALang);
  static QChar imageChar = QChar::ObjectReplacementCharacter;
  if (AOrder == MWO_EMOTICONS)
  {
    for (QTextCursor cursor = ADocument->find(imageChar); !cursor.isNull();  cursor = ADocument->find(imageChar,cursor))
    {
      QUrl url = cursor.charFormat().toImageFormat().name();
      QString key = keyByUrl(url);
      if (!key.isEmpty())
      {
        cursor.insertText(key);
        cursor.insertText(" ");
      }
    }
  }
}

void Emoticons::writeText(Message &AMessage, QTextDocument *ADocument, const QString &ALang, int AOrder)
{
  Q_UNUSED(AMessage); Q_UNUSED(ALang);
  if (AOrder == MWO_EMOTICONS)
  {
    QRegExp regexp("\\S+");
    for (QTextCursor cursor = ADocument->find(regexp); !cursor.isNull();  cursor = ADocument->find(regexp,cursor))
    {
      QUrl url = FUrlByKey.value(cursor.selectedText());
      if (!url.isEmpty())
        cursor.insertImage(url.toString());
    }
  }
}

QWidget *Emoticons::optionsWidget(const QString &ANode, int &AOrder)
{
  if (ANode == ON_EMOTICONS)
  {
    AOrder = OWO_EMOTICONS;
    EmoticonsOptions *widget = new EmoticonsOptions(this);
    connect(widget,SIGNAL(optionsAccepted()),SIGNAL(optionsAccepted()));
    connect(FSettingsPlugin->instance(),SIGNAL(optionsDialogAccepted()),widget,SLOT(apply()));
    connect(FSettingsPlugin->instance(),SIGNAL(optionsDialogRejected()),SIGNAL(optionsRejected()));
    return widget;
  }
  return NULL;
}

void Emoticons::setIconsets(const QStringList &ASubStorages)
{
  QList<QString> oldStorages = FStorageOrder;

  foreach (QString substorage, ASubStorages)
  {
    if (!FStorageOrder.contains(substorage))
    {
      FStorageOrder.append(substorage);
      FStorages.insert(substorage,new IconStorage(RSR_STORAGE_EMOTICONS,substorage,this));
      insertSelectIconMenu(substorage);
      emit iconsetInserted(substorage,QString::null);
    }
    oldStorages.removeAll(substorage);
  }

  foreach (QString substorage, oldStorages)
  {
    removeSelectIconMenu(substorage);
    FStorageOrder.removeAll(substorage);
    delete FStorages.take(substorage);
    emit iconsetRemoved(substorage);
  }

  FStorageOrder = ASubStorages;
  createIconsetUrls();
}

void Emoticons::insertIconset(const QString &ASubStorage, const QString &ABefour)
{
  if (!FStorageOrder.contains(ASubStorage))
  {
    ABefour.isEmpty() ? FStorageOrder.append(ASubStorage) : FStorageOrder.insert(FStorageOrder.indexOf(ABefour),ASubStorage);
    FStorages.insert(ASubStorage,new IconStorage(RSR_STORAGE_EMOTICONS,ASubStorage,this));
    insertSelectIconMenu(ASubStorage);
    createIconsetUrls();
    emit iconsetInserted(ASubStorage,ABefour);
  }
}

void Emoticons::removeIconset(const QString &ASubStorage)
{
  if (FStorageOrder.contains(ASubStorage))
  {
    removeSelectIconMenu(ASubStorage);
    FStorageOrder.removeAll(ASubStorage);
    delete FStorages.take(ASubStorage);
    createIconsetUrls();
    emit iconsetRemoved(ASubStorage);
  }
}

QUrl Emoticons::urlByKey(const QString &AKey) const
{
  return FUrlByKey.value(AKey);
}

QString Emoticons::keyByUrl(const QUrl &AUrl) const
{
  return FUrlByKey.key(AUrl);
}

void Emoticons::createIconsetUrls()
{
  FUrlByKey.clear();
  foreach(QString substorage, FStorageOrder)
  {
    IconStorage *storage = FStorages.value(substorage);
    foreach(QString key, storage->fileKeys())
    {
      if (!FUrlByKey.contains(key))
        FUrlByKey.insert(key,QUrl::fromLocalFile(storage->fileFullName(key)));
    }
  }
}

SelectIconMenu *Emoticons::createSelectIconMenu(const QString &ASubStorage, QWidget *AParent)
{
  SelectIconMenu *menu = new SelectIconMenu(AParent);
  menu->setIconset(ASubStorage);
  connect(menu->instance(),SIGNAL(iconSelected(const QString &, const QString &)),
    SLOT(onIconSelected(const QString &, const QString &)));
  connect(menu->instance(),SIGNAL(destroyed(QObject *)),SLOT(onSelectIconMenuDestroyed(QObject *)));
  return menu;
}

void Emoticons::insertSelectIconMenu(const QString &ASubStorage)
{
  foreach(IToolBarWidget *widget, FToolBarsWidgets)
  {
    SelectIconMenu *menu = createSelectIconMenu(ASubStorage,widget->instance());
    FToolBarWidgetByMenu.insert(menu,widget);
    QToolButton *button = widget->toolBarChanger()->addToolButton(menu->menuAction(),TBG_MWTBW_EMOTICONS,false);
    button->setToolButtonStyle(Qt::ToolButtonIconOnly);
    button->setPopupMode(QToolButton::InstantPopup);
  }
}

void Emoticons::removeSelectIconMenu(const QString &ASubStorage)
{
  QMap<SelectIconMenu *,IToolBarWidget *>::iterator it = FToolBarWidgetByMenu.begin();
  while (it != FToolBarWidgetByMenu.end())
  {
    SelectIconMenu *menu = it.key();
    IToolBarWidget *widget = it.value();
    if (menu->iconset() == ASubStorage)
    {
      widget->toolBarChanger()->removeAction(menu->menuAction());
      it = FToolBarWidgetByMenu.erase(it);
      delete menu;
    }
    else
      it++;
  }
}

void Emoticons::onToolBarWidgetCreated(IToolBarWidget *AWidget)
{
  if (AWidget->editWidget() != NULL)
  {
    FToolBarsWidgets.append(AWidget);
    foreach(QString substorage, FStorageOrder)
    {
      SelectIconMenu *menu = createSelectIconMenu(substorage,AWidget->instance());
      FToolBarWidgetByMenu.insert(menu,AWidget);
      QToolButton *button = AWidget->toolBarChanger()->addToolButton(menu->menuAction(),TBG_MWTBW_EMOTICONS);
      button->setToolButtonStyle(Qt::ToolButtonIconOnly);
      button->setPopupMode(QToolButton::InstantPopup);
    }
    connect(AWidget->instance(),SIGNAL(destroyed(QObject *)),SLOT(onToolBarWidgetDestroyed(QObject *)));
  }
}

void Emoticons::onToolBarWidgetDestroyed(QObject *AObject)
{
  QList<IToolBarWidget *>::iterator it = FToolBarsWidgets.begin();
  while (it != FToolBarsWidgets.end())
  {
    if (qobject_cast<QObject *>((*it)->instance()) == AObject)
      it = FToolBarsWidgets.erase(it);
    else
      it++;
  }
}

void Emoticons::onIconSelected(const QString &ASubStorage, const QString &AIconKey)
{
  Q_UNUSED(ASubStorage);
  SelectIconMenu *menu = qobject_cast<SelectIconMenu *>(sender());
  if (FToolBarWidgetByMenu.contains(menu))
  {
    IEditWidget *widget = FToolBarWidgetByMenu.value(menu)->editWidget();
    if (widget)
    {
      widget->textEdit()->setFocus();
      widget->textEdit()->textCursor().insertText(AIconKey);
      widget->textEdit()->textCursor().insertText(" ");
    }
  }
}

void Emoticons::onSelectIconMenuDestroyed(QObject *AObject)
{
  foreach(SelectIconMenu *menu, FToolBarWidgetByMenu.keys())
    if (qobject_cast<QObject *>(menu) == AObject)
      FToolBarWidgetByMenu.remove(menu);
}

void Emoticons::onSettingsOpened()
{
  ISettings *settings = FSettingsPlugin->settingsForPlugin(EMOTICONS_UUID);
  setIconsets(settings->value(SVN_SUBSTORAGES,FileStorage::availSubStorages(RSR_STORAGE_EMOTICONS)).toStringList());
}

void Emoticons::onSettingsClosed()
{
  ISettings *settings = FSettingsPlugin->settingsForPlugin(EMOTICONS_UUID);
  settings->setValue(SVN_SUBSTORAGES,FStorageOrder);
}

Q_EXPORT_PLUGIN2(plg_emoticons, Emoticons)
