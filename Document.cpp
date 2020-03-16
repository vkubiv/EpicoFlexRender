#include "Document.h"
#include "AttributesReader.h"
#include "DrawAttributesReader.h"
#include <memory>

namespace {

using YGSafeNodePtr = std::unique_ptr<YGNode, decltype(&YGNodeFreeRecursive)>;

void
readLayoutAttributes(YGNodeRef ygNode, json::Value& jsonNode)
{
  if (!jsonNode.HasMember(NodeAttributes::style))
    return;

  auto& styleNode = jsonNode[NodeAttributes::style];

  ReadEnum(AttributesReader::FlexDirection, ygNode, styleNode);
  ReadEnum(AttributesReader::JustifyConten, ygNode, styleNode);
  ReadEnum(AttributesReader::AlignContent, ygNode, styleNode);
  ReadEnum(AttributesReader::AlignItems, ygNode, styleNode);
  ReadEnum(AttributesReader::AlignSelf, ygNode, styleNode);
  ReadEnum(AttributesReader::PositionType, ygNode, styleNode);
  ReadEnum(AttributesReader::FlexWrap, ygNode, styleNode);
  ReadSimpleFloat(AttributesReader::Flex, ygNode, styleNode);
  ReadSimpleFloat(AttributesReader::FlexGrow, ygNode, styleNode);
  ReadFloat(AttributesReader::Width, ygNode, styleNode);
  ReadFloat(AttributesReader::Height, ygNode, styleNode);
  ReadSimpleFloat(AttributesReader::AspectRatio, ygNode, styleNode);
  ReadEdgeProperty(AttributesReader::Padding, ygNode, styleNode);
  ReadEdgeProperty(AttributesReader::Margin, ygNode, styleNode);
}

void
readStyleAttributes(DrawAttributesReader& drawAttributesReader,
                    json::Value& jsonNode)
{
  if (!jsonNode.HasMember(NodeAttributes::style))
    return;

  auto& styleNode = jsonNode[NodeAttributes::style];
  auto& currentDrawParams = drawAttributesReader.currentDrawParams();

  drawAttributesReader.readFont(currentDrawParams.font, styleNode);
}

YGNodeRef
readNode(json::Value& jsonNode, DrawAttributesReader& drawAttributesReader)
{

  drawAttributesReader.saveDrawParams();

  YGSafeNodePtr ygNode(YGNodeNew(), YGNodeFreeRecursive);
  readLayoutAttributes(ygNode.get(), jsonNode);
  YGNodeSetContext(ygNode.get(), &jsonNode);
  
  readStyleAttributes(drawAttributesReader, jsonNode);
  

  if (jsonNode.HasMember(NodeAttributes::children)) {
    auto& children = jsonNode[NodeAttributes::children];
    if (children.IsArray()) {
      int index = 0;
      for (auto& child : children.GetArray()) {

        YGNodeInsertChild(
          ygNode.get(), readNode(child, drawAttributesReader), index);
        index++;
      }
    }
  }

  if (jsonNode.HasMember(NodeAttributes::text)) {
    auto textBlob = drawAttributesReader.readText(
      drawAttributesReader.currentDrawParams().font, jsonNode);
    YGNodeStyleSetMinWidth(ygNode.get(), textBlob->bounds().width());
    YGNodeStyleSetMinHeight(ygNode.get(), textBlob->bounds().height());
  }

  drawAttributesReader.restoreDrawParams();

  return ygNode.release();
}
}

void
Document::buildYoga()
{
  rootNode_ = readNode(jsonDoc_, drawAttributesReader_);
}