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

  DrawAttributesReader& getDrawAttributesReader() { return reader_; }

private:
  DrawAttributesReader reader_;
};