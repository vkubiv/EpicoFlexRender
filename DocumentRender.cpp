#include "DocumentRender.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkData.h"
#include "include/core/SkImage.h"
#include "include/core/SkRRect.h"
#include "include/core/SkTextBlob.h"

DocumentRender::DocumentRender(std::unique_ptr<IFontFactory> fontFactory)
  : reader_(std::move(fontFactory))
{}

void
DocumentRender::renderNode(SkCanvas& canvas, YGNodeRef node)
{
  canvas.save();
  reader_.saveDrawParams();

  canvas.translate(YGNodeLayoutGetLeft(node), YGNodeLayoutGetTop(node));

  SkRect rect = SkRect::MakeXYWH(
    0, 0, YGNodeLayoutGetWidth(node), YGNodeLayoutGetHeight(node));

  json::Value* jsonNode = static_cast<json::Value*>(YGNodeGetContext(node));
  if (jsonNode) {

    if (jsonNode->HasMember(NodeAttributes::image)) {
      auto image = reader_.readImage(*jsonNode);
      canvas.drawImageRect(image, rect, nullptr);
    }

    SkPaint& paint = reader_.currentDrawParams().paint;
    SkFont& font = reader_.currentDrawParams().font;
    readStyle(paint, font, *jsonNode, node, canvas, rect);    

    if (jsonNode->HasMember(NodeAttributes::text)) {
      auto textBlob = reader_.readText(font , * jsonNode);

      paint.setBlendMode(SkBlendMode::kSrcATop);

      canvas.drawTextBlob(
        textBlob, -textBlob->bounds().fLeft, -textBlob->bounds().fTop, paint);
    }
  }

  for (int i = 0; i < YGNodeGetChildCount(node); ++i)
    renderNode(canvas, YGNodeGetChild(node, i));

  reader_.restoreDrawParams();
  canvas.restore();
}

void
DocumentRender::readStyle(SkPaint& paint,
                          SkFont& font,
                          rapidjson::Value& jsonNode,
                          const YGNodeRef& node,
                          SkCanvas& canvas,
                          const SkRect& rect)
{
  if (!jsonNode.HasMember(NodeAttributes::style))
    return;

  auto& style = jsonNode[NodeAttributes::style];

  reader_.readPaintAttributes(paint, style);
  reader_.readFont(font, style);

  if (style.HasMember(NodeAttributes::backgroundGradient)) {
    SkPaint gradient;
    reader_.readGradient(gradient, style,
                         YGNodeLayoutGetWidth(node),
                         YGNodeLayoutGetHeight(node));
    canvas.drawRect(rect, gradient);
  }
}
