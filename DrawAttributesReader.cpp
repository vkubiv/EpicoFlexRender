#include "DrawAttributesReader.h"
#include "include/core/SkImage.h"
#include "include/core/SkStream.h"
#include "include/effects/SkGradientShader.h"
#include "include/ports/SkFontMgr_android.h"
#include <string>
#include <vector>
#include <stdexcept>

namespace {
template<class Setter>
void
readFloat(const char* name, json::Value& node, Setter setter)
{
  if (node.HasMember(name) && node[name].IsNumber()) {
    setter(node[name].GetFloat());
  }
}

float
readFloat(json::Value& node, float defaultVal = 0)
{
  if (node.IsNumber()) {
    return node.GetFloat();
  }
  return defaultVal;
}

float
readFloatPercent(json::Value& node, float defaultVal = 0)
{
  return readFloat(node, defaultVal) / 100.0;
}

template<class Setter>
void
readString(const char* name, json::Value& node, Setter setter)
{
  if (node.HasMember(name) && node[name].IsString()) {
    setter(node[name]);
  }
}

SkColor
readColorHex(json::Value& node)
{
  if (node.IsString()) {
    if (node.GetStringLength() == 6) {
      SkColor color = std::stol(node.GetString(), 0, 16);
      return SkColorSetA(color, 0xFF);
    }

    if (node.GetStringLength() == 8) {
      SkColor color = std::stol(std::string(node.GetString(), 6), 0, 16);
      SkColor alpha = std::stol(std::string(node.GetString() + 6, 2), 0, 16);
      return SkColorSetA(color, alpha);
    }
  }

  return SkColorSetARGB(0xFF, 0x00, 0x00, 0x00);
}
}

sk_sp<SkTextBlob>
DrawAttributesReader::readText(SkFont& font, json::Value& nodeWithText)
{
  if (!nodeWithText.HasMember(NodeAttributes::text)) {
    return nullptr;
  }

  auto& text = nodeWithText[NodeAttributes::text];
  if (text.IsString()) {    
    return SkTextBlob::MakeFromString(text.GetString(), font);
  }

  throw std::runtime_error(
    "readText can read only nodes that have 'text' attribute with string type");
}
std::string
DrawAttributesReader::readStringText(json::Value& nodeWithText)
{
  if (!nodeWithText.HasMember(NodeAttributes::text)) {
    return nullptr;
  }

  auto& text = nodeWithText[NodeAttributes::text];
  if (text.IsString()) {
    return text.GetString();
  }

  throw std::runtime_error(
    "readText can read only nodes that have 'text' attribute with string type");
}

void
DrawAttributesReader::readFont(SkFont& font, json::Value& nodeWithFont)
{
  ;
  if (nodeWithFont.HasMember(NodeAttributes::font)) {
    font.setEdging(SkFont::Edging::kSubpixelAntiAlias);
    font.setSubpixel(true);
    font.setTypeface(
      fontFactory_->load(nodeWithFont[NodeAttributes::font].GetString()));
  }
  readFloat(NodeAttributes::fontSize, nodeWithFont, [&](float size) {
    font.setSize(size);
  });
}

void
DrawAttributesReader::readPaintAttributes(SkPaint& paint, json::Value& node)
{
  readString(NodeAttributes::color, node, [&](json::Value& string) {
    paint.setColor(readColorHex(string));
  });
}

void
DrawAttributesReader::readGradient(SkPaint& paint,
                                   json::Value& node,
                                   float nodeWidth,
                                   float nodeHeight)
{
  if (!node.HasMember(NodeAttributes::backgroundGradient))
    return;

  auto& gradient = node[NodeAttributes::backgroundGradient];
  if (gradient.IsArray()) {
    auto array = gradient.GetArray();

    SkPoint gradientDir =
      SkPoint::Make(readFloatPercent(array[0]) * nodeWidth,
                    readFloatPercent(array[1]) * nodeHeight);

    gradientDir.normalize();

    SkPoint points[2] = { SkPoint::Make(0, 0),
                          SkPoint::Make(gradientDir.fX * nodeWidth,
                                        gradientDir.fY * nodeHeight) };
    std::vector<SkColor> colors;
    std::vector<float> positions;

    for (int i = 2; i < array.Size(); i += 2) {
      colors.push_back(readColorHex(array[i]));
      positions.push_back(readFloatPercent(array[i + 1]));
    }
    if (!positions.empty() && !colors.empty() &&
        positions.size() == colors.size()) {
      paint.setShader(SkGradientShader::MakeLinear(points,
                                                   colors.data(),
                                                   positions.data(),
                                                   colors.size(),
                                                   SkTileMode::kClamp,
                                                   0,
                                                   nullptr));
    }
  }
}

sk_sp<SkImage>
DrawAttributesReader::readImage(json::Value& nodeWithImage)
{

  if (!nodeWithImage.HasMember(NodeAttributes::image)) {
    return nullptr;
  }
  auto& image = nodeWithImage[NodeAttributes::image];
  if (!image.IsString()) {
    return nullptr;
  }

  sk_sp<SkData> imageData = SkData::MakeFromFileName(image.GetString());
  if (imageData) {
    return SkImage::MakeFromEncoded(imageData);
  }

  return sk_sp<SkImage>();
}

DrawAttributesReader::DrawParams&
DrawAttributesReader::currentDrawParams()
{
  return drawParamsStack_.back();
}

void
DrawAttributesReader::saveDrawParams()
{
  drawParamsStack_.push_back(drawParamsStack_.back());
}
void
DrawAttributesReader::restoreDrawParams()
{
  drawParamsStack_.pop_back();
}
