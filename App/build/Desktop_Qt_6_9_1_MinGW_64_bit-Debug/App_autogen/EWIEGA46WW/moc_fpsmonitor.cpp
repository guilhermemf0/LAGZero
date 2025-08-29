/****************************************************************************
** Meta object code from reading C++ file 'fpsmonitor.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.9.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../fpsmonitor.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'fpsmonitor.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN9FpsWorkerE_t {};
} // unnamed namespace

template <> constexpr inline auto FpsWorker::qt_create_metaobjectdata<qt_meta_tag_ZN9FpsWorkerE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "FpsWorker",
        "rtssStatusUpdated",
        "",
        "found",
        "installPath",
        "gameSessionStarted",
        "exeName",
        "windowTitle",
        "uint32_t",
        "processId",
        "gameSessionEnded",
        "averageFps",
        "activeGameFpsUpdate",
        "currentFps",
        "process",
        "readFps"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'rtssStatusUpdated'
        QtMocHelpers::SignalData<void(bool, const QString &)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 3 }, { QMetaType::QString, 4 },
        }}),
        // Signal 'gameSessionStarted'
        QtMocHelpers::SignalData<void(const QString &, const QString &, uint32_t)>(5, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 6 }, { QMetaType::QString, 7 }, { 0x80000000 | 8, 9 },
        }}),
        // Signal 'gameSessionEnded'
        QtMocHelpers::SignalData<void(uint32_t, const QString &, double)>(10, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 8, 9 }, { QMetaType::QString, 6 }, { QMetaType::Double, 11 },
        }}),
        // Signal 'activeGameFpsUpdate'
        QtMocHelpers::SignalData<void(uint32_t, int)>(12, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 8, 9 }, { QMetaType::Int, 13 },
        }}),
        // Slot 'process'
        QtMocHelpers::SlotData<void()>(14, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'readFps'
        QtMocHelpers::SlotData<void()>(15, 2, QMC::AccessPrivate, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<FpsWorker, qt_meta_tag_ZN9FpsWorkerE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject FpsWorker::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN9FpsWorkerE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN9FpsWorkerE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN9FpsWorkerE_t>.metaTypes,
    nullptr
} };

void FpsWorker::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<FpsWorker *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->rtssStatusUpdated((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        case 1: _t->gameSessionStarted((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<uint32_t>>(_a[3]))); break;
        case 2: _t->gameSessionEnded((*reinterpret_cast< std::add_pointer_t<uint32_t>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<double>>(_a[3]))); break;
        case 3: _t->activeGameFpsUpdate((*reinterpret_cast< std::add_pointer_t<uint32_t>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 4: _t->process(); break;
        case 5: _t->readFps(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (FpsWorker::*)(bool , const QString & )>(_a, &FpsWorker::rtssStatusUpdated, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (FpsWorker::*)(const QString & , const QString & , uint32_t )>(_a, &FpsWorker::gameSessionStarted, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (FpsWorker::*)(uint32_t , const QString & , double )>(_a, &FpsWorker::gameSessionEnded, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (FpsWorker::*)(uint32_t , int )>(_a, &FpsWorker::activeGameFpsUpdate, 3))
            return;
    }
}

const QMetaObject *FpsWorker::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *FpsWorker::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN9FpsWorkerE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int FpsWorker::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 6)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 6;
    }
    return _id;
}

// SIGNAL 0
void FpsWorker::rtssStatusUpdated(bool _t1, const QString & _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1, _t2);
}

// SIGNAL 1
void FpsWorker::gameSessionStarted(const QString & _t1, const QString & _t2, uint32_t _t3)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1, _t2, _t3);
}

// SIGNAL 2
void FpsWorker::gameSessionEnded(uint32_t _t1, const QString & _t2, double _t3)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1, _t2, _t3);
}

// SIGNAL 3
void FpsWorker::activeGameFpsUpdate(uint32_t _t1, int _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 3, nullptr, _t1, _t2);
}
namespace {
struct qt_meta_tag_ZN10FpsMonitorE_t {};
} // unnamed namespace

template <> constexpr inline auto FpsMonitor::qt_create_metaobjectdata<qt_meta_tag_ZN10FpsMonitorE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "FpsMonitor",
        "rtssStatusUpdated",
        "",
        "found",
        "installPath",
        "gameSessionStarted",
        "exeName",
        "windowTitle",
        "uint32_t",
        "processId",
        "gameSessionEnded",
        "averageFps",
        "activeGameFpsUpdate",
        "currentFps"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'rtssStatusUpdated'
        QtMocHelpers::SignalData<void(bool, const QString &)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 3 }, { QMetaType::QString, 4 },
        }}),
        // Signal 'gameSessionStarted'
        QtMocHelpers::SignalData<void(const QString &, const QString &, uint32_t)>(5, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 6 }, { QMetaType::QString, 7 }, { 0x80000000 | 8, 9 },
        }}),
        // Signal 'gameSessionEnded'
        QtMocHelpers::SignalData<void(uint32_t, const QString &, double)>(10, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 8, 9 }, { QMetaType::QString, 6 }, { QMetaType::Double, 11 },
        }}),
        // Signal 'activeGameFpsUpdate'
        QtMocHelpers::SignalData<void(uint32_t, int)>(12, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 8, 9 }, { QMetaType::Int, 13 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<FpsMonitor, qt_meta_tag_ZN10FpsMonitorE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject FpsMonitor::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN10FpsMonitorE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN10FpsMonitorE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN10FpsMonitorE_t>.metaTypes,
    nullptr
} };

void FpsMonitor::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<FpsMonitor *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->rtssStatusUpdated((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        case 1: _t->gameSessionStarted((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<uint32_t>>(_a[3]))); break;
        case 2: _t->gameSessionEnded((*reinterpret_cast< std::add_pointer_t<uint32_t>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<double>>(_a[3]))); break;
        case 3: _t->activeGameFpsUpdate((*reinterpret_cast< std::add_pointer_t<uint32_t>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (FpsMonitor::*)(bool , const QString & )>(_a, &FpsMonitor::rtssStatusUpdated, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (FpsMonitor::*)(const QString & , const QString & , uint32_t )>(_a, &FpsMonitor::gameSessionStarted, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (FpsMonitor::*)(uint32_t , const QString & , double )>(_a, &FpsMonitor::gameSessionEnded, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (FpsMonitor::*)(uint32_t , int )>(_a, &FpsMonitor::activeGameFpsUpdate, 3))
            return;
    }
}

const QMetaObject *FpsMonitor::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *FpsMonitor::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN10FpsMonitorE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int FpsMonitor::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 4)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 4;
    }
    return _id;
}

// SIGNAL 0
void FpsMonitor::rtssStatusUpdated(bool _t1, const QString & _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1, _t2);
}

// SIGNAL 1
void FpsMonitor::gameSessionStarted(const QString & _t1, const QString & _t2, uint32_t _t3)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1, _t2, _t3);
}

// SIGNAL 2
void FpsMonitor::gameSessionEnded(uint32_t _t1, const QString & _t2, double _t3)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1, _t2, _t3);
}

// SIGNAL 3
void FpsMonitor::activeGameFpsUpdate(uint32_t _t1, int _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 3, nullptr, _t1, _t2);
}
QT_WARNING_POP
