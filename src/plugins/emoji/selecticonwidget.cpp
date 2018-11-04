#include "selecticonwidget.h"
#include "emoji.h"
#include <QCursor>
#include <QToolTip>
#include <QTextDocument>
#include <definitions/optionvalues.h>
#include <utils/qt4qt5compat.h>

SelectIconWidget::SelectIconWidget(IEmoji::Category ACategory, uint AColumns, uint ARows, IEmoji *AEmoji, QWidget *AParent):
	QWidget(AParent),
	FEmoji(AEmoji),
	FPressed(nullptr),
	FEmojiMap((AEmoji->emojiData(ACategory))),
	FColor(0),
	FHasColored(false),
	FNotReady(true),
	FColumns(AColumns),
	FRows(ARows)
{
	FLayout = new QGridLayout(this);
	FLayout->setMargin(2);
	FLayout->setHorizontalSpacing(3);
	FLayout->setVerticalSpacing(3);
	createLabels();
}

SelectIconWidget::~SelectIconWidget()
{}

void SelectIconWidget::updateLabels(int AColor)
{
	int extent = Options::node(OPV_MESSAGES_EMOJI_SIZE_MENU).value().toInt();
	QSize size(extent, extent);
	for (QMap<QLabel*, QString>::Iterator it=FKeyByLabel.begin(); it!=FKeyByLabel.end(); ++it)
	{
		QString key(*it);
		EmojiData data = FEmoji->findData(key);
		if (FNotReady || (FColor!=AColor && !data.diversities.isEmpty()))
		{
			if (AColor && data.diversities.size() >= AColor)
				key = data.diversities[AColor-1];
			QIcon icon = FEmoji->getIcon(key, size);
			it.key()->setPixmap(icon.pixmap(size));
		}
	}
	if (FNotReady)
		FNotReady = false;
	if (FColor != AColor)
		FColor = AColor;
}

void SelectIconWidget::createLabels()
{
	uint row =0;
	uint column = 0;
	int extent = Options::node(OPV_MESSAGES_EMOJI_SIZE_MENU).value().toInt();
	QSize iconSize(extent, extent);
	for (QMap<uint, EmojiData>::ConstIterator it=FEmojiMap.constBegin(); it!=FEmojiMap.constEnd(); ++it)
		if ((*it).present)
		{
			QLabel *label; //(NULL);
			label = new QLabel(this);
			label->setMargin(2);
			label->setAlignment(Qt::AlignCenter);
			label->setFrameShape(QFrame::Box);
			label->setFrameShadow(QFrame::Sunken);
			label->installEventFilter(this);
			label->setPixmap(QPixmap(iconSize));
			label->setToolTip((*it).name);
			FKeyByLabel.insert(label, (*it).id);
			FLayout->addWidget(label, int(row), int(column));
			if (!(*it).diversities.isEmpty())
				FHasColored=true;
			column = (column+1) % FColumns;
			row += column==0 ? 1 : 0;
			FLayout->setRowStretch(int(row), 0);
		}
	if (row < FRows)
		FLayout->setRowStretch(int(row+1), 1);
}

bool SelectIconWidget::eventFilter(QObject *AWatched, QEvent *AEvent)
{
	QLabel *label = qobject_cast<QLabel *>(AWatched);
	if (AEvent->type() == QEvent::Enter)
	{
		label->setFrameShadow(QFrame::Plain);
		QToolTip::showText(QCursor::pos(),label->toolTip());
	}
	else if (AEvent->type() == QEvent::Leave)
	{
		label->setFrameShadow(QFrame::Sunken);
	}
	else if (AEvent->type() == QEvent::MouseButtonPress)
	{
		FPressed = label;
	}
	else if (AEvent->type() == QEvent::MouseButtonRelease)
	{
		if (FPressed == label)
			emit iconSelected(FKeyByLabel.value(label));
		FPressed = nullptr;
	}
	return QWidget::eventFilter(AWatched,AEvent);
}

void SelectIconWidget::showEvent(QShowEvent *AShowEvent)
{
	Q_UNUSED(AShowEvent)
	int index = Options::node(OPV_MESSAGES_EMOJI_SKINCOLOR).value().toInt();
	if ((FHasColored && FColor!=index) || FNotReady)
		updateLabels(index);
	emit hasColoredChanged(FHasColored);
}
