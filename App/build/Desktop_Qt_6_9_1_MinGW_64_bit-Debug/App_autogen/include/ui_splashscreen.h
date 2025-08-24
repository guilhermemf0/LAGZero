/********************************************************************************
** Form generated from reading UI file 'splashscreen.ui'
**
** Created by: Qt User Interface Compiler version 6.9.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SPLASHSCREEN_H
#define UI_SPLASHSCREEN_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_SplashScreen
{
public:
    QVBoxLayout *verticalLayout;
    QSpacerItem *verticalSpacer_2;
    QLabel *logoLabel;
    QWidget *titleContainer;
    QHBoxLayout *horizontalLayout;
    QLabel *lagLabel;
    QLabel *zeroLabel;
    QSpacerItem *verticalSpacer;

    void setupUi(QWidget *SplashScreen)
    {
        if (SplashScreen->objectName().isEmpty())
            SplashScreen->setObjectName("SplashScreen");
        SplashScreen->resize(450, 350);
        verticalLayout = new QVBoxLayout(SplashScreen);
        verticalLayout->setObjectName("verticalLayout");
        verticalSpacer_2 = new QSpacerItem(20, 40, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        verticalLayout->addItem(verticalSpacer_2);

        logoLabel = new QLabel(SplashScreen);
        logoLabel->setObjectName("logoLabel");
        logoLabel->setMinimumSize(QSize(128, 128));
        logoLabel->setMaximumSize(QSize(128, 128));
        logoLabel->setPixmap(QPixmap(QString::fromUtf8(":/images/logo.png")));
        logoLabel->setScaledContents(true);
        logoLabel->setAlignment(Qt::AlignCenter);

        verticalLayout->addWidget(logoLabel, 0, Qt::AlignHCenter);

        titleContainer = new QWidget(SplashScreen);
        titleContainer->setObjectName("titleContainer");
        horizontalLayout = new QHBoxLayout(titleContainer);
        horizontalLayout->setSpacing(0);
        horizontalLayout->setObjectName("horizontalLayout");
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
        lagLabel = new QLabel(titleContainer);
        lagLabel->setObjectName("lagLabel");
        lagLabel->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout->addWidget(lagLabel);

        zeroLabel = new QLabel(titleContainer);
        zeroLabel->setObjectName("zeroLabel");

        horizontalLayout->addWidget(zeroLabel);


        verticalLayout->addWidget(titleContainer, 0, Qt::AlignHCenter);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        verticalLayout->addItem(verticalSpacer);


        retranslateUi(SplashScreen);

        QMetaObject::connectSlotsByName(SplashScreen);
    } // setupUi

    void retranslateUi(QWidget *SplashScreen)
    {
        SplashScreen->setWindowTitle(QCoreApplication::translate("SplashScreen", "Form", nullptr));
        logoLabel->setText(QString());
        lagLabel->setText(QCoreApplication::translate("SplashScreen", "LAG", nullptr));
        zeroLabel->setText(QCoreApplication::translate("SplashScreen", "ZERO", nullptr));
    } // retranslateUi

};

namespace Ui {
    class SplashScreen: public Ui_SplashScreen {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SPLASHSCREEN_H
