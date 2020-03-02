#include "Document.h"
#include "AttributesReader.h"
#include "DrawAttributesReader.h"
#include <memory>

namespace {

using YGSafeNodePtr = std::unique_ptr<YGNode, decltype(&YGNodeFreeRecursive)>;

void
readLayoutAttributes(YGNodeRef ygNode, json::Value& jsonNode)
{
  ReadEnum(AttributesReader::FlexDirection, ygNode, jsonNode);
  ReadEnum(AttributesReader::JustifyConten, ygNode, jsonNode);
  ReadEnum(AttributesReader::AlignContent, ygNode, jsonNode);
  ReadEnum(AttributesReader::AlignItems, ygNode, jsonNode);
  ReadEnum(AttributesReader::AlignSelf, ygNode, jsonNode);
  ReadEnum(AttributesReader::PositionType, ygNode, jsonNode);
  ReadEnum(AttributesReader::FlexWrap, ygNode, jsonNode);
  ReadSimpleFloat(AttributesReader::Flex, ygNode, jsonNode);
  ReadSimpleFloat(AttributesReader::FlexGrow, ygNode, jsonNode);
  ReadFloat(AttributesReader::Width, ygNode, jsonNode);
  ReadFloat(AttributesReader::Height, ygNode, jsonNode);
  ReadSimpleFloat(AttributesReader::AspectRatio, ygNode, jsonNode);
  ReadEdgeProperty(AttributesReader::Padding, ygNode, jsonNode);
  ReadEdgeProperty(AttributesReader::Margin, ygNode, jsonNode);
}

YGNodeRef
readNode(json::Value& jsonNode, DrawAttributesReader& drawAttributesReader)
{
  YGSafeNodePtr ygNode(YGNodeNew(), YGNodeFreeRecursive);
  readLayoutAttributes(ygNode.get(), jsonNode);
  YGNodeSetContext(ygNode.get(), &jsonNode);

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
    auto textBlob = drawAttributesReader.readText(jsonNode);
    YGNodeStyleSetMinWidth(ygNode.get(), textBlob->bounds().width());
    YGNodeStyleSetMinHeight(ygNode.get(), textBlob->bounds().height());
  }

  return ygNode.release();
}
}

void
Document::buildYoga()
{
  rootNode_ = readNode(jsonDoc_, drawAttributesReader_);
}