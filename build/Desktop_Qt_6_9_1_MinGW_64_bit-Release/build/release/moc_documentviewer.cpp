/****************************************************************************
** Meta object code from reading C++ file 'documentviewer.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.9.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../documentviewer.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'documentviewer.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.9.1. It"
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
struct qt_meta_tag_ZN14DocumentViewerE_t {};
} // unnamed namespace

template <> constexpr inline auto DocumentViewer::qt_create_metaobjectdata<qt_meta_tag_ZN14DocumentViewerE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "DocumentViewer",
        "documentLoaded",
        "",
        "Document",
        "document",
        "sectionHighlighted",
        "section",
        "searchResultFound",
        "keyword",
        "position",
        "loadDocument",
        "loadDocumentById",
        "documentId",
        "searchInDocument",
        "highlightSection",
        "exportDocument",
        "printDocument",
        "onSearchTextChanged",
        "text",
        "onSearchButtonClicked",
        "onNextButtonClicked",
        "onPreviousButtonClicked",
        "onZoomInClicked",
        "onZoomOutClicked",
        "onZoomResetClicked",
        "onPrintClicked",
        "onExportClicked",
        "onOutlineItemClicked",
        "QTreeWidgetItem*",
        "item",
        "column",
        "onOutlineContextMenu",
        "pos",
        "onDocumentLinkClicked",
        "url",
        "onTextSelectionChanged"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'documentLoaded'
        QtMocHelpers::SignalData<void(const Document &)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 3, 4 },
        }}),
        // Signal 'sectionHighlighted'
        QtMocHelpers::SignalData<void(int)>(5, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 6 },
        }}),
        // Signal 'searchResultFound'
        QtMocHelpers::SignalData<void(const QString &, int)>(7, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 8 }, { QMetaType::Int, 9 },
        }}),
        // Slot 'loadDocument'
        QtMocHelpers::SlotData<void(const Document &)>(10, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 3, 4 },
        }}),
        // Slot 'loadDocumentById'
        QtMocHelpers::SlotData<void(int)>(11, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 12 },
        }}),
        // Slot 'searchInDocument'
        QtMocHelpers::SlotData<void(const QString &)>(13, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 8 },
        }}),
        // Slot 'highlightSection'
        QtMocHelpers::SlotData<void(int)>(14, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 6 },
        }}),
        // Slot 'exportDocument'
        QtMocHelpers::SlotData<void()>(15, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'printDocument'
        QtMocHelpers::SlotData<void()>(16, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'onSearchTextChanged'
        QtMocHelpers::SlotData<void(const QString &)>(17, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 18 },
        }}),
        // Slot 'onSearchButtonClicked'
        QtMocHelpers::SlotData<void()>(19, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onNextButtonClicked'
        QtMocHelpers::SlotData<void()>(20, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onPreviousButtonClicked'
        QtMocHelpers::SlotData<void()>(21, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onZoomInClicked'
        QtMocHelpers::SlotData<void()>(22, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onZoomOutClicked'
        QtMocHelpers::SlotData<void()>(23, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onZoomResetClicked'
        QtMocHelpers::SlotData<void()>(24, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onPrintClicked'
        QtMocHelpers::SlotData<void()>(25, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onExportClicked'
        QtMocHelpers::SlotData<void()>(26, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onOutlineItemClicked'
        QtMocHelpers::SlotData<void(QTreeWidgetItem *, int)>(27, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 28, 29 }, { QMetaType::Int, 30 },
        }}),
        // Slot 'onOutlineContextMenu'
        QtMocHelpers::SlotData<void(const QPoint &)>(31, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QPoint, 32 },
        }}),
        // Slot 'onDocumentLinkClicked'
        QtMocHelpers::SlotData<void(const QUrl &)>(33, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QUrl, 34 },
        }}),
        // Slot 'onTextSelectionChanged'
        QtMocHelpers::SlotData<void()>(35, 2, QMC::AccessPrivate, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<DocumentViewer, qt_meta_tag_ZN14DocumentViewerE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject DocumentViewer::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN14DocumentViewerE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN14DocumentViewerE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN14DocumentViewerE_t>.metaTypes,
    nullptr
} };

void DocumentViewer::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<DocumentViewer *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->documentLoaded((*reinterpret_cast< std::add_pointer_t<Document>>(_a[1]))); break;
        case 1: _t->sectionHighlighted((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 2: _t->searchResultFound((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 3: _t->loadDocument((*reinterpret_cast< std::add_pointer_t<Document>>(_a[1]))); break;
        case 4: _t->loadDocumentById((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 5: _t->searchInDocument((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 6: _t->highlightSection((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 7: _t->exportDocument(); break;
        case 8: _t->printDocument(); break;
        case 9: _t->onSearchTextChanged((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 10: _t->onSearchButtonClicked(); break;
        case 11: _t->onNextButtonClicked(); break;
        case 12: _t->onPreviousButtonClicked(); break;
        case 13: _t->onZoomInClicked(); break;
        case 14: _t->onZoomOutClicked(); break;
        case 15: _t->onZoomResetClicked(); break;
        case 16: _t->onPrintClicked(); break;
        case 17: _t->onExportClicked(); break;
        case 18: _t->onOutlineItemClicked((*reinterpret_cast< std::add_pointer_t<QTreeWidgetItem*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 19: _t->onOutlineContextMenu((*reinterpret_cast< std::add_pointer_t<QPoint>>(_a[1]))); break;
        case 20: _t->onDocumentLinkClicked((*reinterpret_cast< std::add_pointer_t<QUrl>>(_a[1]))); break;
        case 21: _t->onTextSelectionChanged(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (DocumentViewer::*)(const Document & )>(_a, &DocumentViewer::documentLoaded, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (DocumentViewer::*)(int )>(_a, &DocumentViewer::sectionHighlighted, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (DocumentViewer::*)(const QString & , int )>(_a, &DocumentViewer::searchResultFound, 2))
            return;
    }
}

const QMetaObject *DocumentViewer::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *DocumentViewer::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN14DocumentViewerE_t>.strings))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int DocumentViewer::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 22)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 22;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 22)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 22;
    }
    return _id;
}

// SIGNAL 0
void DocumentViewer::documentLoaded(const Document & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1);
}

// SIGNAL 1
void DocumentViewer::sectionHighlighted(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1);
}

// SIGNAL 2
void DocumentViewer::searchResultFound(const QString & _t1, int _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1, _t2);
}
QT_WARNING_POP
