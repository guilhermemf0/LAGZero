/****************************************************************************
** Meta object code from reading C++ file 'mainwindow.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.9.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../mainwindow.h"
#include <QtGui/qtextcursor.h>
#include <QtNetwork/QSslError>
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'mainwindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.9.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {
struct qt_meta_tag_ZN10MainWindowE_t {};
} // unnamed namespace

template <> constexpr inline auto MainWindow::qt_create_metaobjectdata<qt_meta_tag_ZN10MainWindowE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "MainWindow",
        "onHardwareUpdated",
        "",
        "QMap<QString,HardwareInfo>",
        "deviceInfos",
        "onRtssStatusUpdated",
        "found",
        "installPath",
        "onDownloadRtssClicked",
        "onNavigationButtonClicked",
        "onTempNavigationButtonClicked",
        "onSettingsButtonClicked",
        "onParticlesEnabledChanged",
        "state",
        "onSaveReportsChanged",
        "onGameSessionStarted",
        "exeName",
        "windowTitle",
        "uint32_t",
        "processId",
        "onGameSessionEnded",
        "averageFps",
        "onActiveGameFpsUpdate",
        "currentFps",
        "onApiSearchFinished",
        "ApiGameResult",
        "result",
        "onImageDownloaded",
        "localPath",
        "originalUrl",
        "openReportsFolder",
        "updateSessionInfo",
        "onHelperMissing",
        "onChartDurationChanged",
        "index",
        "onEditGameRequested",
        "executableName",
        "onRemoveGameRequested"
    };

    QtMocHelpers::UintData qt_methods {
        // Slot 'onHardwareUpdated'
        QtMocHelpers::SlotData<void(const QMap<QString,HardwareInfo> &)>(1, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 3, 4 },
        }}),
        // Slot 'onRtssStatusUpdated'
        QtMocHelpers::SlotData<void(bool, const QString &)>(5, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Bool, 6 }, { QMetaType::QString, 7 },
        }}),
        // Slot 'onDownloadRtssClicked'
        QtMocHelpers::SlotData<void()>(8, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onNavigationButtonClicked'
        QtMocHelpers::SlotData<void()>(9, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onTempNavigationButtonClicked'
        QtMocHelpers::SlotData<void()>(10, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onSettingsButtonClicked'
        QtMocHelpers::SlotData<void()>(11, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onParticlesEnabledChanged'
        QtMocHelpers::SlotData<void(int)>(12, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 13 },
        }}),
        // Slot 'onSaveReportsChanged'
        QtMocHelpers::SlotData<void(int)>(14, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 13 },
        }}),
        // Slot 'onGameSessionStarted'
        QtMocHelpers::SlotData<void(const QString &, const QString &, uint32_t)>(15, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 16 }, { QMetaType::QString, 17 }, { 0x80000000 | 18, 19 },
        }}),
        // Slot 'onGameSessionEnded'
        QtMocHelpers::SlotData<void(uint32_t, const QString &, double)>(20, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 18, 19 }, { QMetaType::QString, 16 }, { QMetaType::Double, 21 },
        }}),
        // Slot 'onActiveGameFpsUpdate'
        QtMocHelpers::SlotData<void(uint32_t, int)>(22, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 18, 19 }, { QMetaType::Int, 23 },
        }}),
        // Slot 'onApiSearchFinished'
        QtMocHelpers::SlotData<void(const ApiGameResult &)>(24, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 25, 26 },
        }}),
        // Slot 'onImageDownloaded'
        QtMocHelpers::SlotData<void(const QString &, const QUrl &)>(27, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 28 }, { QMetaType::QUrl, 29 },
        }}),
        // Slot 'openReportsFolder'
        QtMocHelpers::SlotData<void()>(30, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'updateSessionInfo'
        QtMocHelpers::SlotData<void()>(31, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onHelperMissing'
        QtMocHelpers::SlotData<void()>(32, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onChartDurationChanged'
        QtMocHelpers::SlotData<void(int)>(33, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 34 },
        }}),
        // Slot 'onEditGameRequested'
        QtMocHelpers::SlotData<void(const QString &)>(35, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 36 },
        }}),
        // Slot 'onRemoveGameRequested'
        QtMocHelpers::SlotData<void(const QString &)>(37, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 36 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<MainWindow, qt_meta_tag_ZN10MainWindowE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject MainWindow::staticMetaObject = { {
    QMetaObject::SuperData::link<QMainWindow::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN10MainWindowE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN10MainWindowE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN10MainWindowE_t>.metaTypes,
    nullptr
} };

void MainWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<MainWindow *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->onHardwareUpdated((*reinterpret_cast< std::add_pointer_t<QMap<QString,HardwareInfo>>>(_a[1]))); break;
        case 1: _t->onRtssStatusUpdated((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        case 2: _t->onDownloadRtssClicked(); break;
        case 3: _t->onNavigationButtonClicked(); break;
        case 4: _t->onTempNavigationButtonClicked(); break;
        case 5: _t->onSettingsButtonClicked(); break;
        case 6: _t->onParticlesEnabledChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 7: _t->onSaveReportsChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 8: _t->onGameSessionStarted((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<uint32_t>>(_a[3]))); break;
        case 9: _t->onGameSessionEnded((*reinterpret_cast< std::add_pointer_t<uint32_t>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<double>>(_a[3]))); break;
        case 10: _t->onActiveGameFpsUpdate((*reinterpret_cast< std::add_pointer_t<uint32_t>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 11: _t->onApiSearchFinished((*reinterpret_cast< std::add_pointer_t<ApiGameResult>>(_a[1]))); break;
        case 12: _t->onImageDownloaded((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QUrl>>(_a[2]))); break;
        case 13: _t->openReportsFolder(); break;
        case 14: _t->updateSessionInfo(); break;
        case 15: _t->onHelperMissing(); break;
        case 16: _t->onChartDurationChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 17: _t->onEditGameRequested((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 18: _t->onRemoveGameRequested((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObject *MainWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MainWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN10MainWindowE_t>.strings))
        return static_cast<void*>(this);
    return QMainWindow::qt_metacast(_clname);
}

int MainWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 19)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 19;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 19)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 19;
    }
    return _id;
}
QT_WARNING_POP
