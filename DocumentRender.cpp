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

  canvas.translate(YGNodeLayoutGetLeft(node), YGNodeLayoutGetTop(node));

  SkRect rect = SkRect::MakeXYWH(
    0, 0, YGNodeLayoutGetWidth(node), YGNodeLayoutGetHeight(node));

  json::Value* jsonNode = static_cast<json::Value*>(YGNodeGetContext(node));
  if (jsonNode) {

    if (jsonNode->HasMember(NodeAttributes::image)) {
      auto image = reader_.readImage(*jsonNode);
      canvas.drawImageRect(image, rect, nullptr);
    }

    SkPaint paint;
    reader_.readPaintAttributes(paint, *jsonNode);

    if (jsonNode->HasMember(NodeAttributes::gradient)) {
      SkPaint gradient;
      reader_.readGradient(gradient,
                           *jsonNode,
                           YGNodeLayoutGetWidth(node),
                           YGNodeLayoutGetHeight(node));
      canvas.drawRect(rect, gradient);
    }

    if (jsonNode->HasMember(NodeAttributes::text)) {
      auto textBlob = reader_.readText(*jsonNode);

      paint.setBlendMode(SkBlendMode::kSrcATop);

      canvas.drawTextBlob(
        textBlob, -textBlob->bounds().fLeft, -textBlob->bounds().fTop, paint);
    }
  }

  for (int i = 0; i < YGNodeGetChildCount(node); ++i)
    renderNode(canvas, YGNodeGetChild(node, i));

  canvas.restore();
}
