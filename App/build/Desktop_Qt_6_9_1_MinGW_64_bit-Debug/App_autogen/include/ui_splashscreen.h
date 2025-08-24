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
    QHBoxLayout *titleLayout;
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
        titleLayout = new QHBoxLayout(titleContainer);
        titleLayout->setSpacing(0);
        titleLayout->setObjectName("titleLayout");
        titleLayout->setContentsMargins(0, 0, 0, 0);

        verticalLayout->addWidget(titleContainer);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        verticalLayout->addItem(verticalSpacer);


        retranslateUi(SplashScreen);

        QMetaObject::connectSlotsByName(SplashScreen);
    } // setupUi

    void retranslateUi(QWidget *SplashScreen)
    {
        SplashScreen->setWindowTitle(QCoreApplication::translate("SplashScreen", "Form", nullptr));
        logoLabel->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class SplashScreen: public Ui_SplashScreen {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SPLASHSCREEN_H
