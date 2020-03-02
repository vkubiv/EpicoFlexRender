#pragma once

#include "NodeAttributes.h"
#include "json.h"
#include "yoga.h"
#include <array>
#include <string>

namespace AttributesReader {
template<class Value>
struct ValueMap
{
  Value value;
  const char* strValue;
};

constexpr ValueMap<YGAlign> AlignValueMap[] = {
  { YGAlignAuto, "auto" },
  { YGAlignFlexStart, "flex-start" },
  { YGAlignCenter, "center" },
  { YGAlignFlexEnd, "flex-end" },
  { YGAlignStretch, "stretch" },
  { YGAlignBaseline, "baseline" },
  { YGAlignSpaceBetween, "space-between" },
  { YGAlignSpaceAround, "space-around" }
};

constexpr ValueMap<YGEdge> EdgeValueMap[] = {
  { YGEdgeAll, "" },
  { YGEdgeVertical, "Vertical" },
  { YGEdgeHorizontal, "Horizontal" },
  { YGEdgeLeft, "Left" },
  { YGEdgeTop, "Top" },
  { YGEdgeRight, "Right" },
  { YGEdgeBottom, "Bottom" },
  { YGEdgeStart, "Start" },
  { YGEdgeEnd, "End" },

};

constexpr ValueMap<YGFlexDirection> FlexDirectionValueMap[] = {
  { YGFlexDirectionColumn, "column" },
  { YGFlexDirectionColumnReverse, "column-reverse" },
  { YGFlexDirectionRow, "row" },
  { YGFlexDirectionRowReverse, "row-reverse" },
};

constexpr ValueMap<YGJustify> JustifyValueMap[] = {
  { YGJustifyFlexStart, "flex-start" },
  { YGJustifyCenter, "center" },
  { YGJustifyFlexEnd, "flex-end" },
  { YGJustifySpaceBetween, "space-between" },
  { YGJustifySpaceAround, "space-around" },
  { YGJustifySpaceEvenly, "space-evenly" },
};

constexpr ValueMap<YGPositionType> PositionTypeValueMap[] = {
  { YGPositionTypeRelative, "relative" },
  { YGPositionTypeAbsolute, "absolute" },
};

constexpr ValueMap<YGWrap> WrapValueMap[] = {
  { YGWrapNoWrap, "nowrap" },
  { YGWrapWrap, "wrap" },
  { YGWrapWrapReverse, "wrap-reverse" },
};

template<class ValueMap, class Setter>
struct EnumDesc
{
  const char* name;
  ValueMap valueMap;
  Setter setter;
};

template<class ValueMapItem, int N, class Setter>
EnumDesc<std::array<ValueMapItem, N>, Setter>
MakeEnumDesc(const char* name, const ValueMapItem (&valueMap)[N], Setter setter)
{
  std::array<ValueMapItem, N> valueMapArray;
  std::copy(valueMap, valueMap + N, valueMapArray.begin());

  return EnumDesc<std::array<ValueMapItem, N>, Setter>{ name,
                                                        valueMapArray,
                                                        setter };
}

template<class Setter>
struct SimpleFloatDesc
{
  const char* name;
  Setter setter;
};

template<class Setter>
SimpleFloatDesc<Setter>
MakeSimpleFloatDesc(const char* name, Setter setter)
{
  return { name, setter };
}

template<class Setter, class PercentsSetter>
struct FloatDesc
{
  const char* name;
  Setter setter;
  PercentsSetter percentsSetter;
};

template<class Setter, class PercentsSetter>
FloatDesc<Setter, PercentsSetter>
MakeFloatDesc(const char* name, Setter setter, PercentsSetter percentsSetter)
{
  return { name, setter, percentsSetter };
}

const auto FlexDirection = MakeEnumDesc(NodeAttributes::flexDirection,
                                        FlexDirectionValueMap,
                                        YGNodeStyleSetFlexDirection);
const auto JustifyConten = MakeEnumDesc(NodeAttributes::justifyContent,
                                        JustifyValueMap,
                                        YGNodeStyleSetJustifyContent);
const auto AlignContent = MakeEnumDesc(NodeAttributes::alignContent,
                                       AlignValueMap,
                                       YGNodeStyleSetAlignContent);
const auto AlignItems = MakeEnumDesc(NodeAttributes::alignItems,
                                     AlignValueMap,
                                     YGNodeStyleSetAlignItems);
const auto AlignSelf = MakeEnumDesc(NodeAttributes::alignSelf,
                                    AlignValueMap,
                                    YGNodeStyleSetAlignSelf);
const auto PositionType = MakeEnumDesc(NodeAttributes::position,
                                       PositionTypeValueMap,
                                       YGNodeStyleSetPositionType);
const auto FlexWrap =
  MakeEnumDesc(NodeAttributes::flexWrap, WrapValueMap, YGNodeStyleSetFlexWrap);

const auto Flex = MakeSimpleFloatDesc(NodeAttributes::flex, YGNodeStyleSetFlex);
const auto FlexGrow =
  MakeSimpleFloatDesc(NodeAttributes::flexGrow, YGNodeStyleSetFlexGrow);

const auto Width = MakeFloatDesc(NodeAttributes::width,
                                 YGNodeStyleSetWidth,
                                 YGNodeStyleSetWidthPercent);

const auto Height = MakeFloatDesc(NodeAttributes::height,
                                  YGNodeStyleSetHeight,
                                  YGNodeStyleSetHeightPercent);

const auto AspectRatio = MakeSimpleFloatDesc(NodeAttributes::aspectRatio,
                                  YGNodeStyleSetAspectRatio);


const auto Margin = MakeFloatDesc(NodeAttributes::margin,
                                  YGNodeStyleSetMargin,
                                  YGNodeStyleSetMarginPercent);
const auto Padding = MakeFloatDesc(NodeAttributes::padding,
                                   YGNodeStyleSetPadding,
                                   YGNodeStyleSetPaddingPercent);

template<class PropertyDesc>
void
ReadEnum(const PropertyDesc& propertyDesc,
         YGNodeRef node,
         const json::Value& nodeJson)
{

  if (!nodeJson.HasMember(propertyDesc.name))
    return;

  auto& property = nodeJson[propertyDesc.name];
  if (property.IsString()) {

    const char* propVal = property.GetString();

    for (auto& mapItem : propertyDesc.valueMap) {
      if (strcmp(propVal, mapItem.strValue) == 0) {
        propertyDesc.setter(node, mapItem.value);
        return;
      }
    }
  }

  throw std::runtime_error(std::string("Invalid value for property '") +
                           propertyDesc.name + "'");
}

template<class PropertyDesc>
void
ReadSimpleFloat(const PropertyDesc& propertyDesc,
                YGNodeRef node,
                const json::Value& nodeJson)
{
  if (!nodeJson.HasMember(propertyDesc.name))
    return;

  auto& property = nodeJson[propertyDesc.name];
  if (property.IsNumber()) {
    propertyDesc.setter(node, property.GetFloat());
    return;
  }

  throw std::runtime_error(std::string("Invalid value for property '") +
                           propertyDesc.name + "'");
}

template<class PropertyDesc>
void
ReadFloat(const PropertyDesc& propertyDesc,
          YGNodeRef node,
          const json::Value& nodeJson)
{
  if (!nodeJson.HasMember(propertyDesc.name))
    return;

  auto& property = nodeJson[propertyDesc.name];
  if (property.IsNumber()) {
    propertyDesc.setter(node, property.GetFloat());
    return;
  }

  if (property.IsString()) {
    const char* propVal = property.GetString();
    auto len = property.GetStringLength();
    if (len > 1 && propVal[len - 1] == '%') {
      propertyDesc.percentsSetter(node,
                                  std::stof(std::string(propVal, len - 1)));
      return;
    }
  }

  throw std::runtime_error(std::string("Invalid value for property '") +
                           propertyDesc.name + "'");
}

template<class PropertyDesc>
void
ReadEdgeProperty(const PropertyDesc& propertyDesc,
                 YGNodeRef node,
                 const json::Value& nodeJson)
{
  for (auto& edge : EdgeValueMap) {
    auto propName = std::string(propertyDesc.name) + edge.strValue;
    if (nodeJson.HasMember(propName.c_str())) {
      const auto MarginEdge =
        MakeFloatDesc(propName.c_str(),
                      [&](YGNodeRef node, float points) {
                        propertyDesc.setter(node, edge.value, points);
                      },
                      [&](YGNodeRef node, float percent) {
                        propertyDesc.percentsSetter(node, edge.value, percent);
                      });

      ReadFloat(MarginEdge, node, nodeJson);
    }
  }
}

}