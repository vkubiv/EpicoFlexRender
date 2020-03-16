#pragma once
#include "IFontFactory.h"
#include "NodeAttributes.h"
#include "include/core/SkImage.h"
#include "include/core/SkTextBlob.h"
#include "json.h"
#include <memory>
#include <vector>

class DrawAttributesReader
{
public:

  struct DrawParams
  {
    SkFont font;
    SkPaint paint;
  };

  DrawAttributesReader(std::unique_ptr<IFontFactory> fontFactory)
    : fontFactory_(std::move(fontFactory))
  {
    drawParamsStack_.push_back(DrawParams());  
  }

  sk_sp<SkTextBlob> readText(SkFont& font, json::Value& nodeWithText);

  void readFont(SkFont& font, json::Value& nodeWithFont);

  void readPaintAttributes(SkPaint& paint, json::Value& node);

  void readGradient(SkPaint& paint,
                    json::Value& node,
                    float nodeWidth,
                    float nodeHeight);

  sk_sp<SkImage> readImage(json::Value& nodeWithImage);

  DrawParams &currentDrawParams();
  void saveDrawParams();
  void restoreDrawParams();

private:
  std::unique_ptr<IFontFactory> fontFactory_;

  std::vector<DrawParams> drawParamsStack_;
};