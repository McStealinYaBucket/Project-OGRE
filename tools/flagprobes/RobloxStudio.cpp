// probe: RobloxStudio (STUDIO Qt shell 0x401000-0x514000): moc output for UpdateUIManager (moc_UpdateUIManager.cpp,
// vftable 0x00d1b844 slots 0-2: 0x50ec50 metaObject, 0x50eca0 qt_metacast, 0x50ed00 qt_metacall). moc's output is
// fixed by Qt 4.8.5's moc, so the body is certain; the Qt declarations are minimal local stand-ins. metaObject
// keeps QScopedPointer's Q_ASSERT (qt_assert import), so QT_NO_DEBUG was not defined; qt_metacast inlines strcmp.
#include <string.h>
struct QMetaObject { enum Call { InvokeMetaMethod = 0 }; const void* d[4]; };
__declspec(dllimport) void qt_assert(const char* assertion, const char* file, int line);
inline void qt_noop() {}
#define Q_ASSERT(cond) ((!(cond)) ? qt_assert(#cond, __FILE__, __LINE__) : qt_noop())
#line 90 "qscopedpointer.h"
template <typename T> class QScopedPointer { public: T* operator->() const { Q_ASSERT(d); return d; } T* d; };
class QObjectData { public: virtual ~QObjectData() = 0; void* q_ptr; void* parent; void* children; unsigned int bits; int postedEvents; QMetaObject* metaObject; };
class __declspec(dllimport) QObject { public:
  virtual const QMetaObject* metaObject() const; virtual void* qt_metacast(const char*); virtual int qt_metacall(QMetaObject::Call, int, void**);
  virtual ~QObject();
protected: QScopedPointer<QObjectData> d_ptr; };
class UpdateUIManager : public QObject { public:
  static const QMetaObject staticMetaObject;
  virtual const QMetaObject* metaObject() const; virtual void* qt_metacast(const char*); virtual int qt_metacall(QMetaObject::Call, int, void**);
private:
  static void qt_static_metacall(QObject* _o, QMetaObject::Call _c, int _id, void** _a); };
extern const char qt_meta_stringdata_UpdateUIManager[];
const QMetaObject* UpdateUIManager::metaObject() const
{
  return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}
void* UpdateUIManager::qt_metacast(const char* _clname)
{
  if (!_clname) return 0;
  if (!strcmp(_clname, qt_meta_stringdata_UpdateUIManager))
    return static_cast<void*>(const_cast<UpdateUIManager*>(this));
  return QObject::qt_metacast(_clname);
}
int UpdateUIManager::qt_metacall(QMetaObject::Call _c, int _id, void** _a)
{
  _id = QObject::qt_metacall(_c, _id, _a);
  if (_id < 0)
    return _id;
  if (_c == QMetaObject::InvokeMetaMethod) {
    if (_id < 4)
      qt_static_metacall(this, _c, _id, _a);
    _id -= 4;
  }
  return _id;
}

// RBX::FrameRateManagerStatsItem::update() (STUDIO 0x0043e8a0, a COMDAT that a Studio TU contributed; PLAYER's copy
// at 0x0048e530 sits in RbxOgre): same body as the RbxOgre probe, here under the Studio shell's flags.
namespace RBX {
class FrameRateManager { public: struct Metrics { bool AutoQuality; int QualityLevel; int NumberOfSettles; double AverageSwitchesPerSettle; };
  Metrics GetMetrics(); };
class FrameRateManagerStatsItem { public:
  char pad[0x88]; int qualityLevel; bool autoQuality; int numberOfSettles; double averageSwitches; FrameRateManager* frm;
  void update(); };
void FrameRateManagerStatsItem::update()
{
  FrameRateManager::Metrics metrics = frm->GetMetrics();
  qualityLevel = metrics.QualityLevel;
  autoQuality = metrics.AutoQuality;
  numberOfSettles = metrics.NumberOfSettles;
  averageSwitches = metrics.AverageSwitchesPerSettle;
}
}
