// probe: GfxBase (PLAYER 0x9c5ed0, 0x9d55f0, 0x9d5650, 0x9ca750, 0x9d5b10)
namespace RBX {
struct RenderCaps;
class CRenderSettings { public: void setAutoQualityLevel(int level); };
void CRenderSettings::setAutoQualityLevel(int level) {}
class Adorn { public: void preSubmitPass(); };
void Adorn::preSubmitPass() {}
struct Camera;
class IAdornableCollector;
class IAdornable { public: void* vt; int pad[3]; IAdornableCollector* collector; void shouldRenderSetDirty(); };
class IAdornableCollector { public: void onRenderNeedsUpdate(IAdornable* a); };
void IAdornable::shouldRenderSetDirty() { if (collector) collector->onRenderNeedsUpdate(this); }
class AdornInner { public: virtual void v00(); virtual void v04(); virtual void v08(); virtual void v0c(); virtual void v10(); virtual void v14(); virtual void v18(); virtual void v1c();
  virtual void v20(); virtual void v24(); virtual void v28(); virtual void v2c(); virtual void v30(); virtual void v34(); virtual void v38(); virtual void v3c();
  virtual void v40(); virtual void v44(); virtual void v48(); virtual void v4c(); virtual void v50(); virtual void v54(); virtual void v58(); virtual void v5c();
  virtual void v60(); virtual void v64(); virtual void v68(); virtual void v6c(); virtual void v70(); virtual void v74(); virtual void v78(); virtual void v7c();
  virtual void v80(); virtual const RenderCaps* getRenderCaps() const; };
class AdornBillboarder { public: void* vt; int pad[6]; AdornInner* adorn; const Camera* getCamera() const; const RenderCaps* getRenderCaps() const; };
const Camera* AdornBillboarder::getCamera() const { return 0; }
const RenderCaps* AdornBillboarder::getRenderCaps() const { return adorn->getRenderCaps(); }
}

// ViewBase::render (0x9c7c10): the double == 0 compare decides /arch (fucomip = /arch:SSE).
namespace RBX {
class IMetric;
namespace Time { double nowFastSec(); }
class ViewBase { public:
  virtual void v00(); virtual void v04(); virtual void v08(); virtual void v0c(); virtual void v10(); virtual void v14(); virtual void v18(); virtual void v1c();
  virtual void renderPrepare(IMetric* metric); virtual void renderPerform(double timeJobStart);
  void render(IMetric* metric, double timeJobStart); };
void ViewBase::render(IMetric* metric, double timeJobStart)
{
  if (timeJobStart == 0.0)
    timeJobStart = Time::nowFastSec();
  renderPrepare(metric);
  renderPerform(timeJobStart);
}
}
// AdornBillboarder::drawFont2DImpl (0x9d5720): EH frame without a cookie (/GS-), runtime_error ctor inlined (/Ob).
#include <string>
#include <stdexcept>
namespace G3D { class Vector2 { public: float x, y; }; class Color4; class Rect2D; }
namespace RBX {
namespace Text { enum Font { F0 }; enum XAlign { X0 }; enum YAlign { Y0 }; }
class AdornBillboarder2 { public:
  G3D::Vector2 drawFont2DImpl(Adorn* adorn, const std::string& s, const G3D::Vector2& pos, float size, const G3D::Color4& color,
    const G3D::Color4& outline, Text::Font font, Text::XAlign xalign, Text::YAlign yalign, const G3D::Vector2& availableSpace, const G3D::Rect2D& clip); };
G3D::Vector2 AdornBillboarder2::drawFont2DImpl(Adorn*, const std::string&, const G3D::Vector2&, float, const G3D::Color4&,
    const G3D::Color4&, Text::Font, Text::XAlign, Text::YAlign, const G3D::Vector2&, const G3D::Rect2D&)
{
  throw std::runtime_error("Invalid operation");
}
}
