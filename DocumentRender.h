#pragma once

#include "DrawAttributesReader.h"
#include "yoga.h"
#include <memory>

class IFontFactory;

class DocumentRender
{
public:
  DocumentRender(std::unique_ptr<IFontFactory> fontFactory);

  void renderNode(SkCanvas& canvas, YGNodeRef node);

  void readStyle(SkPaint& paint,
                 SkFont& font,
                 rapidjson::Value& jsonNode,
                 const YGNodeRef& node,
                 SkCanvas& canvas,
                 const SkRect& rect);

  DrawAttributesReader& getDrawAttributesReader() { return reader_; }

private:
  DrawAttributesReader reader_;
};