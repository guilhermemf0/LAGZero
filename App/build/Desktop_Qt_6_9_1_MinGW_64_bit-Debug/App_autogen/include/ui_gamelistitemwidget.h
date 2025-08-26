/********************************************************************************
** Form generated from reading UI file 'gamelistitemwidget.ui'
**
** Created by: Qt User Interface Compiler version 6.9.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_GAMELISTITEMWIDGET_H
#define UI_GAMELISTITEMWIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFrame>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_GameListItemWidget
{
public:
    QHBoxLayout *horizontalLayout;
    QFrame *activeIndicator;
    QLabel *coverLabel;
    QVBoxLayout *verticalLayout;
    QLabel *nameLabel;
    QLabel *avgFpsLabel;
    QSpacerItem *horizontalSpacer;
    QLabel *currentFpsLabel;

    void setupUi(QWidget *GameListItemWidget)
    {
        if (GameListItemWidget->objectName().isEmpty())
            GameListItemWidget->setObjectName("GameListItemWidget");
        GameListItemWidget->resize(400, 80);
        GameListItemWidget->setMinimumSize(QSize(0, 80));
        GameListItemWidget->setMaximumSize(QSize(16777215, 80));
        GameListItemWidget->setStyleSheet(QString::fromUtf8("#GameListItemWidget {\n"
"	background-color: rgba(30, 33, 47, 0.8);\n"
"	border-radius: 8px;\n"
"}\n"
"\n"
"#GameListItemWidget[active=\"true\"] {\n"
"	background-color: rgba(0, 133, 255, 0.3);\n"
"}\n"
"\n"
"#coverLabel {\n"
"	border-radius: 4px;\n"
"	background-color: #111;\n"
"}\n"
"\n"
"#nameLabel {\n"
"	color: #ffffff;\n"
"	font-size: 16px;\n"
"	font-weight: bold;\n"
"}\n"
"\n"
"#avgFpsLabel, #currentFpsLabel {\n"
"	color: #aeb9d6;\n"
"	font-size: 13px;\n"
"}\n"
"\n"
"#currentFpsLabel {\n"
"	color: #00d1ff;\n"
"	font-weight: bold;\n"
"}\n"
"\n"
"#activeIndicator {\n"
"	background-color: #00d1ff;\n"
"	border: none;\n"
"	border-radius: 4px;\n"
"}"));
        horizontalLayout = new QHBoxLayout(GameListItemWidget);
        horizontalLayout->setSpacing(15);
        horizontalLayout->setObjectName("horizontalLayout");
        horizontalLayout->setContentsMargins(5, 5, 15, 5);
        activeIndicator = new QFrame(GameListItemWidget);
        activeIndicator->setObjectName("activeIndicator");
        activeIndicator->setMinimumSize(QSize(8, 0));
        activeIndicator->setMaximumSize(QSize(8, 16777215));
        activeIndicator->setFrameShape(QFrame::StyledPanel);
        activeIndicator->setFrameShadow(QFrame::Raised);

        horizontalLayout->addWidget(activeIndicator);

        coverLabel = new QLabel(GameListItemWidget);
        coverLabel->setObjectName("coverLabel");
        coverLabel->setMinimumSize(QSize(120, 56));
        coverLabel->setMaximumSize(QSize(120, 56));
        coverLabel->setScaledContents(true);
        coverLabel->setAlignment(Qt::AlignCenter);

        horizontalLayout->addWidget(coverLabel);

        verticalLayout = new QVBoxLayout();
        verticalLayout->setSpacing(4);
        verticalLayout->setObjectName("verticalLayout");
        nameLabel = new QLabel(GameListItemWidget);
        nameLabel->setObjectName("nameLabel");
        QSizePolicy sizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(nameLabel->sizePolicy().hasHeightForWidth());
        nameLabel->setSizePolicy(sizePolicy);

        verticalLayout->addWidget(nameLabel);

        avgFpsLabel = new QLabel(GameListItemWidget);
        avgFpsLabel->setObjectName("avgFpsLabel");

        verticalLayout->addWidget(avgFpsLabel);


        horizontalLayout->addLayout(verticalLayout);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        currentFpsLabel = new QLabel(GameListItemWidget);
        currentFpsLabel->setObjectName("currentFpsLabel");
        currentFpsLabel->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout->addWidget(currentFpsLabel);


        retranslateUi(GameListItemWidget);

        QMetaObject::connectSlotsByName(GameListItemWidget);
    } // setupUi

    void retranslateUi(QWidget *GameListItemWidget)
    {
        GameListItemWidget->setWindowTitle(QCoreApplication::translate("GameListItemWidget", "Form", nullptr));
        coverLabel->setText(QString());
        nameLabel->setText(QCoreApplication::translate("GameListItemWidget", "Game Name", nullptr));
        avgFpsLabel->setText(QCoreApplication::translate("GameListItemWidget", "M\303\251dia: -- FPS", nullptr));
        currentFpsLabel->setText(QCoreApplication::translate("GameListItemWidget", "144 FPS", nullptr));
    } // retranslateUi

};

namespace Ui {
    class GameListItemWidget: public Ui_GameListItemWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_GAMELISTITEMWIDGET_H
