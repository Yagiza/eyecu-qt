#ifndef ICONSETDELEGATE_H
#define ICONSETDELEGATE_H

#include <QHash>
#include <QPainter>
#include <QModelIndex>
#include <QStyledItemDelegate>
#include "utilsexport.h"
#include "iconstorage.h"

class UTILS_EXPORT IconsetDelegate :
	public QStyledItemDelegate
{
public:
	enum DataRole {
		IDR_STORAGE             = Qt::UserRole + 128,
		IDR_SUBSTORAGE,
		IDR_ICON_ROW_COUNT,
		IDR_HIDE_STORAGE_NAME
	};
public:
	IconsetDelegate(QObject *AParent = NULL);
// *** <<< eyeCU <<< ***
	IconsetDelegate(const QStringList &AFilter, QObject *AParent = NULL);
// *** >>> eyeCU >>> ***
	~IconsetDelegate();
	virtual void paint(QPainter *APainter, const QStyleOptionViewItem &AOption, const QModelIndex &AIndex) const;
	virtual QSize sizeHint(const QStyleOptionViewItem &AOption, const QModelIndex &AIndex) const;
protected:
	virtual bool editorEvent(QEvent *AEvent, QAbstractItemModel *AModel, const QStyleOptionViewItem &AOption, const QModelIndex &AIndex);
protected:
	void drawBackground(QPainter *APainter, const QStyleOptionViewItemV4 &AIndexOption) const;
	void drawFocusRect(QPainter *APainter, const QStyleOptionViewItemV4 &AIndexOption, const QRect &ARect) const;
    QRect checkButtonRect(const QStyleOptionViewItemV4 &AIndexOption, const QRect &ABounding, const QVariant &AValue) const;
    void drawCheckButton(QPainter *APainter, const QStyleOptionViewItemV4 &AIndexOption, const QRect &ARect, Qt::CheckState AState) const;
    QStyleOptionViewItem indexStyleOption(const QStyleOptionViewItemV4 &AOption, const QModelIndex &AIndex) const;
private:
	mutable QHash<QString, QHash<QString, IconStorage *> > FStorages;
	const QStringList FFilter; // *** <<< eyeCU >>> ***
};

#endif // ICONSETDELEGATE_H
