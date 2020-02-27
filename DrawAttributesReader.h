#pragma once
#include "IFontFactory.h"
#include "NodeAttributes.h"
#include "include/core/SkImage.h"
#include "include/core/SkTextBlob.h"
#include "json.h"
#include <memory>

class DrawAttributesReader
{
public:
  DrawAttributesReader(std::unique_ptr<IFontFactory> fontFactory)
    : fontFactory_(std::move(fontFactory))
  {}

  sk_sp<SkTextBlob> readText(json::Value& nodeWithText);

  SkFont readFont(json::Value& nodeWithFont);

  void readPaintAttributes(SkPaint& paint, json::Value& node);

  void readGradient(SkPaint& paint,
                    json::Value& node,
                    float nodeWidth,
                    float nodeHeight);

  sk_sp<SkImage> readImage(json::Value& nodeWithImage);

private:
  std::unique_ptr<IFontFactory> fontFactory_;
};