#include "editbookmarksdialog.h"

#include <QHeaderView>
#include <QMessageBox>
#include <definitions/resources.h>
#include <definitions/menuicons.h>
#include <utils/iconstorage.h>
#include <utils/logger.h>
#include "utils/qt4qt5compat.h"

enum Columns {
	COL_NAME,
	COL_VALUE,
	COL_NICK,
	COL_SORT,
	COL_COUNT
};

#define TDR_TYPE                Qt::UserRole+1
#define TDR_NAME                Qt::UserRole+2
#define TDR_ROOMJID             Qt::UserRole+3
#define TDR_AUTO                Qt::UserRole+4
#define TDR_NICK                Qt::UserRole+5
#define TDR_PASSWORD            Qt::UserRole+6
#define TDR_URL                 Qt::UserRole+7

EditBookmarksDialog::EditBookmarksDialog(IBookmarks *ABookmarks, const Jid &AStreamJid, const QList<IBookmark> &AList, QWidget *AParent) : QDialog(AParent)
{
	REPORT_VIEW;
	ui.setupUi(this);
	setAttribute(Qt::WA_DeleteOnClose,true);
	setWindowTitle(tr("Edit bookmarks - %1").arg(AStreamJid.uBare()));
	IconStorage::staticStorage(RSR_STORAGE_MENUICONS)->insertAutoIcon(this,MNI_BOOKMARKS_EDIT,0,0,"windowIcon");

	FBookmarks = ABookmarks;
	FStreamJid = AStreamJid;

	ui.tbwBookmarks->setRowCount(AList.count());
	for (int row=0; row<AList.count(); ++row)
	{
		IBookmark bookmark = AList.at(row);
		setBookmarkToRow(row,bookmark);
	}
	QHeaderView *header = ui.tbwBookmarks->horizontalHeader();
	header->hideSection(COL_SORT);
#if QT_VERSION < 0x050000
	header->setClickable(true);
#else
	header->setSectionsClickable(true);
#endif
	header->SETRESIZEMODE(COL_NAME,QHeaderView::ResizeToContents);
	header->SETRESIZEMODE(COL_VALUE,QHeaderView::Stretch);
	header->SETRESIZEMODE(COL_NICK,QHeaderView::ResizeToContents);

	connect(header,SIGNAL(sectionClicked(int)),SLOT(onSortingStateChange(int)));

	connect(ui.pbtAdd,SIGNAL(clicked()),SLOT(onEditButtonClicked()));
	connect(ui.pbtEdit,SIGNAL(clicked()),SLOT(onEditButtonClicked()));
	connect(ui.pbtDelete,SIGNAL(clicked()),SLOT(onEditButtonClicked()));
	connect(ui.pbtMoveUp,SIGNAL(clicked()),SLOT(onEditButtonClicked()));
	connect(ui.pbtMoveDown,SIGNAL(clicked()),SLOT(onEditButtonClicked()));
	connect(ui.bbxButtons,SIGNAL(accepted()),SLOT(onDialogAccepted()));

	connect(ui.tbwBookmarks,SIGNAL(itemDoubleClicked(QTableWidgetItem *)),SLOT(onTableItemDoubleClicked(QTableWidgetItem *)));
}

EditBookmarksDialog::~EditBookmarksDialog()
{
	emit dialogDestroyed();
}

Jid EditBookmarksDialog::streamJid() const
{
	return FStreamJid;
}

IBookmark EditBookmarksDialog::getBookmarkFromRow(int ARow) const
{
	IBookmark bookmark;
	QTableWidgetItem *tableItem = ui.tbwBookmarks->item(ARow,COL_NAME);
	bookmark.type = tableItem->data(TDR_TYPE).toInt();
	bookmark.name = tableItem->data(TDR_NAME).toString();
	bookmark.room.roomJid = tableItem->data(TDR_ROOMJID).toString();
	bookmark.room.autojoin = tableItem->data(TDR_AUTO).toBool();
	bookmark.room.nick = tableItem->data(TDR_NICK).toString();
	bookmark.room.password = tableItem->data(TDR_PASSWORD).toString();
	bookmark.url.url = tableItem->data(TDR_URL).toString();
	return bookmark;
}

void EditBookmarksDialog::setBookmarkToRow(int ARow, const IBookmark &ABookmark)
{
	QTableWidgetItem *nameItem = new QTableWidgetItem;
	nameItem->setText(ABookmark.name);

	if (ABookmark.type==IBookmark::TypeRoom && ABookmark.room.autojoin)
	{
		QFont font = nameItem->font();
		font.setBold(true);
		nameItem->setFont(font);
	}

	nameItem->setData(TDR_TYPE,ABookmark.type);
	nameItem->setData(TDR_NAME,ABookmark.name);
	nameItem->setData(TDR_ROOMJID,ABookmark.room.roomJid.bare());
	nameItem->setData(TDR_AUTO,ABookmark.room.autojoin);
	nameItem->setData(TDR_NICK,ABookmark.room.nick);
	nameItem->setData(TDR_PASSWORD,ABookmark.room.password);
	nameItem->setData(TDR_URL,ABookmark.url.url.toString());
	ui.tbwBookmarks->setItem(ARow,COL_NAME,nameItem);

	QTableWidgetItem *valueItem = new QTableWidgetItem;
	valueItem->setText(ABookmark.type==IBookmark::TypeUrl ? ABookmark.url.url.toString() : ABookmark.room.roomJid.uBare());
	ui.tbwBookmarks->setItem(nameItem->row(),COL_VALUE,valueItem);

	QTableWidgetItem *nickItem = new QTableWidgetItem;
	nickItem->setText(ABookmark.room.nick);
	ui.tbwBookmarks->setItem(nameItem->row(),COL_NICK,nickItem);

	QTableWidgetItem *sortItem = new QTableWidgetItem;
	sortItem->setText(nameItem->text());
	ui.tbwBookmarks->setItem(nameItem->row(),COL_SORT,sortItem);
}

void EditBookmarksDialog::onEditButtonClicked()
{
	QPushButton *button = qobject_cast<QPushButton *>(sender());
	if (button == ui.pbtAdd)
	{
		IBookmark bookmark;
		if (FBookmarks->showEditBookmarkDialog(&bookmark,this)->exec() == QDialog::Accepted)
		{
			ui.tbwBookmarks->setRowCount(ui.tbwBookmarks->rowCount()+1);
			setBookmarkToRow(ui.tbwBookmarks->rowCount()-1,bookmark);
		}
	}
	else if (button == ui.pbtEdit)
	{
		int row = ui.tbwBookmarks->currentRow();
		if (row >= 0)
		{
			IBookmark bookmark = getBookmarkFromRow(row);
			if (FBookmarks->showEditBookmarkDialog(&bookmark,this)->exec() == QDialog::Accepted)
				setBookmarkToRow(row,bookmark);
		}
	}
	else if (button == ui.pbtDelete)
	{
		QTableWidgetItem *tableItem = ui.tbwBookmarks->currentItem();
		if (tableItem)
			ui.tbwBookmarks->removeRow(tableItem->row());
	}
	else if (button == ui.pbtMoveUp)
	{
		QTableWidgetItem *tableItem = ui.tbwBookmarks->currentItem();
		if (tableItem && tableItem->row()>0)
		{
			int row = tableItem->row();
			IBookmark bookmark1 = getBookmarkFromRow(row);
			IBookmark bookmark2 = getBookmarkFromRow(row-1);
			setBookmarkToRow(row,bookmark2);
			setBookmarkToRow(row-1,bookmark1);
			ui.tbwBookmarks->setCurrentCell(row-1,COL_NAME);
		}
	}
	else if (button == ui.pbtMoveDown)
	{
		QTableWidgetItem *tableItem = ui.tbwBookmarks->currentItem();
		if (tableItem && tableItem->row()<ui.tbwBookmarks->rowCount()-1)
		{
			int row = tableItem->row();
			IBookmark bookmark1 = getBookmarkFromRow(row);
			IBookmark bookmark2 = getBookmarkFromRow(row+1);
			setBookmarkToRow(row,bookmark2);
			setBookmarkToRow(row+1,bookmark1);
			ui.tbwBookmarks->setCurrentCell(row+1,COL_NAME);
		}
	}
}

void EditBookmarksDialog::onDialogAccepted()
{
	QList<IBookmark> bookmarks;
	for (int row=0; row<ui.tbwBookmarks->rowCount(); row++)
		bookmarks.append(getBookmarkFromRow(row));

	if (!FBookmarks->setBookmarks(FStreamJid,bookmarks))
		QMessageBox::warning(this,tr("Error"),tr("Cant save bookmarks to server"));
	else
		accept();
}

void EditBookmarksDialog::onTableItemDoubleClicked(QTableWidgetItem *AItem)
{
	IBookmark bookmark = getBookmarkFromRow(AItem->row());
	if (FBookmarks->showEditBookmarkDialog(&bookmark,this)->exec() == QDialog::Accepted)
		setBookmarkToRow(AItem->row(),bookmark);
}

void EditBookmarksDialog::onSortingStateChange(int AColumn)
{
	QHeaderView *header = qobject_cast<QHeaderView *>(sender());
	QTableWidget *table = header!=NULL ? qobject_cast<QTableWidget *>(header->parentWidget()) : NULL;
	if (table) 
	{
		if (FLastSortSection!=AColumn || header->sortIndicatorOrder()!=Qt::AscendingOrder) 
		{
			// first or double click
			FLastSortSection = AColumn;
			table->sortItems(AColumn, header->sortIndicatorOrder());
		} 
		else 
		{
			// triple click
			FLastSortSection = -1;
			table->sortItems(COL_SORT, Qt::AscendingOrder);
		}
	}
}
