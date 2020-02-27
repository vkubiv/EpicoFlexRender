#pragma once

#include "DrawAttributesReader.h"
#include "json.h"
#include "yoga.h"

class Document
{
public:
  explicit Document(json::Document&& jsonDoc,
                    DrawAttributesReader& drawAttributesReader)
    : jsonDoc_(std::move(jsonDoc))
    , drawAttributesReader_(drawAttributesReader)
    , rootNode_(nullptr)
  {}

  ~Document()
  {
    if (rootNode_) {
      YGNodeFreeRecursive(rootNode_);
    }
  }

  void buildYoga();

  YGNodeRef getRootNode() { return rootNode_; }

private:
  json::Document&& jsonDoc_;
  DrawAttributesReader& drawAttributesReader_;
  YGNodeRef rootNode_;
};