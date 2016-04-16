#include "definitions/optionvalues.h"

#include "avataroptionswidget.h"
#include "ui_avataroptionswidget.h"

AvatarOptionsWidget::AvatarOptionsWidget(const OptionsNode &ANode, bool AOfflineAvailable, QWidget *parent) :
    QWidget(parent),
	ui(new Ui::AvatarOptionsWidget),
	FNode(ANode)
{
    ui->setupUi(this);

	if (!ANode.value("display").toBool())
	{
		ui->cbDisplayGray->setDisabled(true);
		ui->cmbAvatarPosition->setDisabled(true);
		ui->cmbAvatarSize->setDisabled(true);
	}
    reset();

	if (AOfflineAvailable)
		connect(ui->cbDisplayGray, SIGNAL(stateChanged(int)), SIGNAL(modified()));
	else
		ui->cbDisplayGray->hide();
	connect(ui->cbShow, SIGNAL(stateChanged(int)), SIGNAL(modified()));	
	connect(ui->cmbAvatarPosition, SIGNAL(currentIndexChanged(int)), SIGNAL(modified()));
	connect(ui->cmbAvatarSize, SIGNAL(currentIndexChanged(int)), SIGNAL(modified()));
}

AvatarOptionsWidget::~AvatarOptionsWidget()
{
    delete ui;
}

void AvatarOptionsWidget::apply()
{	
	FNode.setValue(ui->cbShow->isChecked(), "display");
	if (!ui->cbDisplayGray->isHidden())
		FNode.setValue(ui->cbDisplayGray->isChecked(), "display-gray");
	FNode.setValue(ui->cmbAvatarPosition->currentIndex(), "position");
	FNode.setValue(ui->cmbAvatarSize->currentIndex(), "size");
    emit childApply();
}

void AvatarOptionsWidget::reset()
{
	ui->cbShow->setChecked(FNode.value("display").toBool());
	if (!ui->cbDisplayGray->isHidden())
		ui->cbDisplayGray->setChecked(FNode.value("display-gray").toBool());
	ui->cmbAvatarPosition->setCurrentIndex(FNode.value("position").toInt());
	ui->cmbAvatarSize->setCurrentIndex(FNode.value("size").toInt());
    emit childReset();
}

void AvatarOptionsWidget::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type())
    {
        case QEvent::LanguageChange:
            ui->retranslateUi(this);
            break;
        default:
            break;
    }
}
