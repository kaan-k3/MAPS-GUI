/****************************************************************************
** Meta object code from reading C++ file 'mainwindow.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.10.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../mainwindow.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'mainwindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.10.1. It"
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
        "onLoadIGES",
        "",
        "onLoadGauss",
        "onLoadAllBodies",
        "onExportScreenshot",
        "onSave",
        "onResetNormals",
        "onSurfaceControl",
        "onPrincipalData",
        "onWaveSettings",
        "onRunConfig",
        "onGenerateProject",
        "onRunMAPS",
        "onStopMAPS",
        "onMapsOutput",
        "line",
        "onMapsFinished",
        "exitCode",
        "summary",
        "onToggleSurface",
        "on",
        "onToggleWireframe",
        "onToggleControlNet",
        "onToggleControlPts",
        "onToggleNormals",
        "onToggleGaussPts",
        "onToggleGaussNormals",
        "onTogglePerspective",
        "onFitView",
        "onFlipAll",
        "onFlipSelected",
        "onPatchListClicked",
        "QTreeWidgetItem*",
        "item",
        "column",
        "onPatchColorChangeRequested",
        "patchIndex",
        "onPatchClicked",
        "onWorkerProgress",
        "msg",
        "onWorkerFinished",
        "std::vector<std::vector<QVector3D>>",
        "patchPoints",
        "patchNormals",
        "std::vector<std::pair<int,int>>",
        "patchDims",
        "std::vector<QVector3D>",
        "allPts"
    };

    QtMocHelpers::UintData qt_methods {
        // Slot 'onLoadIGES'
        QtMocHelpers::SlotData<void()>(1, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onLoadGauss'
        QtMocHelpers::SlotData<void()>(3, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onLoadAllBodies'
        QtMocHelpers::SlotData<void()>(4, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onExportScreenshot'
        QtMocHelpers::SlotData<void()>(5, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onSave'
        QtMocHelpers::SlotData<void()>(6, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onResetNormals'
        QtMocHelpers::SlotData<void()>(7, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onSurfaceControl'
        QtMocHelpers::SlotData<void()>(8, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onPrincipalData'
        QtMocHelpers::SlotData<void()>(9, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onWaveSettings'
        QtMocHelpers::SlotData<void()>(10, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onRunConfig'
        QtMocHelpers::SlotData<void()>(11, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onGenerateProject'
        QtMocHelpers::SlotData<void()>(12, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onRunMAPS'
        QtMocHelpers::SlotData<void()>(13, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onStopMAPS'
        QtMocHelpers::SlotData<void()>(14, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onMapsOutput'
        QtMocHelpers::SlotData<void(const QString &)>(15, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 16 },
        }}),
        // Slot 'onMapsFinished'
        QtMocHelpers::SlotData<void(int, const QString &)>(17, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 18 }, { QMetaType::QString, 19 },
        }}),
        // Slot 'onToggleSurface'
        QtMocHelpers::SlotData<void(bool)>(20, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Bool, 21 },
        }}),
        // Slot 'onToggleWireframe'
        QtMocHelpers::SlotData<void(bool)>(22, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Bool, 21 },
        }}),
        // Slot 'onToggleControlNet'
        QtMocHelpers::SlotData<void(bool)>(23, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Bool, 21 },
        }}),
        // Slot 'onToggleControlPts'
        QtMocHelpers::SlotData<void(bool)>(24, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Bool, 21 },
        }}),
        // Slot 'onToggleNormals'
        QtMocHelpers::SlotData<void(bool)>(25, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Bool, 21 },
        }}),
        // Slot 'onToggleGaussPts'
        QtMocHelpers::SlotData<void(bool)>(26, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Bool, 21 },
        }}),
        // Slot 'onToggleGaussNormals'
        QtMocHelpers::SlotData<void(bool)>(27, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Bool, 21 },
        }}),
        // Slot 'onTogglePerspective'
        QtMocHelpers::SlotData<void(bool)>(28, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Bool, 21 },
        }}),
        // Slot 'onFitView'
        QtMocHelpers::SlotData<void()>(29, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onFlipAll'
        QtMocHelpers::SlotData<void()>(30, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onFlipSelected'
        QtMocHelpers::SlotData<void()>(31, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onPatchListClicked'
        QtMocHelpers::SlotData<void(QTreeWidgetItem *, int)>(32, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 33, 34 }, { QMetaType::Int, 35 },
        }}),
        // Slot 'onPatchColorChangeRequested'
        QtMocHelpers::SlotData<void(int)>(36, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 37 },
        }}),
        // Slot 'onPatchClicked'
        QtMocHelpers::SlotData<void(int)>(38, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 37 },
        }}),
        // Slot 'onWorkerProgress'
        QtMocHelpers::SlotData<void(const QString &)>(39, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 40 },
        }}),
        // Slot 'onWorkerFinished'
        QtMocHelpers::SlotData<void(const std::vector<std::vector<QVector3D>> &, const std::vector<std::vector<QVector3D>> &, const std::vector<std::pair<int,int>> &, const std::vector<QVector3D> &)>(41, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 42, 43 }, { 0x80000000 | 42, 44 }, { 0x80000000 | 45, 46 }, { 0x80000000 | 47, 48 },
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
        case 0: _t->onLoadIGES(); break;
        case 1: _t->onLoadGauss(); break;
        case 2: _t->onLoadAllBodies(); break;
        case 3: _t->onExportScreenshot(); break;
        case 4: _t->onSave(); break;
        case 5: _t->onResetNormals(); break;
        case 6: _t->onSurfaceControl(); break;
        case 7: _t->onPrincipalData(); break;
        case 8: _t->onWaveSettings(); break;
        case 9: _t->onRunConfig(); break;
        case 10: _t->onGenerateProject(); break;
        case 11: _t->onRunMAPS(); break;
        case 12: _t->onStopMAPS(); break;
        case 13: _t->onMapsOutput((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 14: _t->onMapsFinished((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2]))); break;
        case 15: _t->onToggleSurface((*reinterpret_cast<std::add_pointer_t<bool>>(_a[1]))); break;
        case 16: _t->onToggleWireframe((*reinterpret_cast<std::add_pointer_t<bool>>(_a[1]))); break;
        case 17: _t->onToggleControlNet((*reinterpret_cast<std::add_pointer_t<bool>>(_a[1]))); break;
        case 18: _t->onToggleControlPts((*reinterpret_cast<std::add_pointer_t<bool>>(_a[1]))); break;
        case 19: _t->onToggleNormals((*reinterpret_cast<std::add_pointer_t<bool>>(_a[1]))); break;
        case 20: _t->onToggleGaussPts((*reinterpret_cast<std::add_pointer_t<bool>>(_a[1]))); break;
        case 21: _t->onToggleGaussNormals((*reinterpret_cast<std::add_pointer_t<bool>>(_a[1]))); break;
        case 22: _t->onTogglePerspective((*reinterpret_cast<std::add_pointer_t<bool>>(_a[1]))); break;
        case 23: _t->onFitView(); break;
        case 24: _t->onFlipAll(); break;
        case 25: _t->onFlipSelected(); break;
        case 26: _t->onPatchListClicked((*reinterpret_cast<std::add_pointer_t<QTreeWidgetItem*>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<int>>(_a[2]))); break;
        case 27: _t->onPatchColorChangeRequested((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 28: _t->onPatchClicked((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 29: _t->onWorkerProgress((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 30: _t->onWorkerFinished((*reinterpret_cast<std::add_pointer_t<std::vector<std::vector<QVector3D>>>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<std::vector<std::vector<QVector3D>>>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<std::vector<std::pair<int,int>>>>(_a[3])),(*reinterpret_cast<std::add_pointer_t<std::vector<QVector3D>>>(_a[4]))); break;
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
        if (_id < 31)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 31;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 31)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 31;
    }
    return _id;
}
QT_WARNING_POP
