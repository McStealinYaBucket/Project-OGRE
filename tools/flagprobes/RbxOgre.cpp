// probe: RbxOgre (PLAYER 0x480700-0x51f6b0), istreamDataStream.cpp (0x51f270-0x51f690). Minimal local base
// standing in for Ogre::DataStream (vftable + name + size + access = 0x28 bytes, as the stores show).
#include <istream>
#include <string>
namespace Ogre {
class DataStream {
public:
  virtual ~DataStream() {}
  virtual size_t read(void* buf, size_t count) = 0;
  virtual void skip(long count) = 0;
  virtual void seek(size_t pos) = 0;
  virtual size_t tell() const = 0;
  virtual bool eof() const = 0;
  virtual void close() = 0;
protected:
  std::string mName; size_t mSize; unsigned short mAccess;
};
class istreamDataStream : public DataStream {
public:
  size_t read(void* buf, size_t count);
  void skip(long count);
  void seek(size_t pos);
  size_t tell() const;
  bool eof() const;
  void close();
protected:
  std::istream* mStream;
  bool mFreeOnClose;
};
size_t istreamDataStream::read(void* buf, size_t count)   // 0x51f270
{
  mStream->read(static_cast<char*>(buf), static_cast<std::streamsize>(count));
  return mStream->gcount();
}
void istreamDataStream::seek(size_t pos)                   // 0x51f4b0
{
  mStream->clear();
  mStream->seekg(static_cast<std::streamoff>(pos), std::ios::beg);
}
bool istreamDataStream::eof() const                        // 0x51f4e0
{
  return mStream->eof();
}
void istreamDataStream::close()                            // 0x51f4f0
{
  if (mStream && mFreeOnClose)
  {
    delete mStream;
    mStream = 0;
  }
}
void istreamDataStream::skip(long count)                   // 0x51f620
{
  mStream->clear();
  mStream->seekg(static_cast<std::streamoff>(count), std::ios::cur);
}
size_t istreamDataStream::tell() const                     // 0x51f650
{
  mStream->clear();
  return (size_t)mStream->tellg();
}
}

// RBX::Name::doDeclare<sDecal> (0x480bf0), instantiated in an RbxOgre TU: EH frame for the function-local
// static, no cookie (/GS-). Declarations after Client/App/include/Util/Name.h (prior tree).
namespace RBX {
class Name { public:
  static const Name& declare(const char* const& sName);
  template<const char* const& sName> static const Name& doDeclare() { static const Name* n = &declare(sName); return *n; }
};
extern const char* const sDecal;
template const Name& Name::doDeclare<sDecal>();
}
// RBX::AdornRbxGfx::getViewport() const (0x491700): int->float through cvtsi2ss (/arch:SSE, x87 fild without).
#include <G3D/Rect2D.h>
namespace Ogre { class __declspec(dllimport) Viewport { public:
  int getActualLeft() const; int getActualTop() const; int getActualWidth() const; int getActualHeight() const; }; }
namespace RBX {
struct OgreViewHolder { char pad[0x44]; Ogre::Viewport* viewport; };
class AdornRbxGfx { public: char pad[0x1c]; OgreViewHolder* view; G3D::Rect2D getViewport() const; };
G3D::Rect2D AdornRbxGfx::getViewport() const
{
  Ogre::Viewport* vp = view->viewport;
  return G3D::Rect2D::xywh((float)vp->getActualLeft(), (float)vp->getActualTop(), (float)vp->getActualWidth(), (float)vp->getActualHeight());
}
}

// RBX::FrameRateManagerStatsItem::update() (0x48e530): a double member copy (x87 fld/fstp; movsd under SSE2?).
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
