#include "DocumentRender.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkData.h"
#include "include/core/SkImage.h"
#include "include/core/SkRRect.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkTextBlob.h"
#include <codecvt>
#include <vector>
#include <locale>

#if _MSC_VER >= 1900

std::u32string
convert_to_utf32(std::string text)
{
  std::wstring_convert<std::codecvt_utf8<int32_t>, int32_t> utf32conv;
  auto intstr = utf32conv.from_bytes(text);
  
  return std::u32string(intstr.begin(), intstr.end());
}

#else

std::u32string
convert_to_utf32(std::string text)
{
  std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> utf32conv;
  return utf32conv.from_bytes(text);
}


#endif 

struct GlyphDesc
{
  SkGlyphID id;
  SkFont font;  
  SkScalar width;
  SkRect bounds;
};

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
      SkPaint paint;
      paint.setFilterQuality(kHigh_SkFilterQuality);
      paint.setAntiAlias(true);
      auto image = reader_.readImage(*jsonNode);
      canvas.drawImageRect(image, rect, &paint);
    }


    SkPaint& paint = reader_.currentDrawParams().paint;
    SkFont& font = reader_.currentDrawParams().font;
    readStyle(paint, font, *jsonNode, node, canvas, rect);    

    if (jsonNode->HasMember(NodeAttributes::text)) {
      auto text= reader_.readStringText(* jsonNode);            
      auto textBlob = reader_.readText(font, *jsonNode);            

      paint.setBlendMode(SkBlendMode::kSrc);
      
      std::u32string utf32 = convert_to_utf32(text);

      std::vector<GlyphDesc> glyphs(utf32.size());

      sk_sp<SkFontMgr> mgr(SkFontMgr::RefDefault());
           
      for (size_t i = 0; i < glyphs.size(); ++i) {
        glyphs[i].id = font.unicharToGlyph(utf32[i]);
        
        glyphs[i].font = font;

        if (glyphs[i].id == 0) {
        
          sk_sp<SkTypeface> fallback(mgr->matchFamilyStyleCharacter(
            nullptr, font.getTypeface()->fontStyle(), nullptr, 0, utf32[i]));

          glyphs[i].font.setTypeface(fallback);
          glyphs[i].id = glyphs[i].font.unicharToGlyph(utf32[i]);
        }

        glyphs[i].font.getWidthsBounds(&glyphs[i].id, 1, &glyphs[i].width, &glyphs[i].bounds, &paint);

      }   
      
      SkScalar offset = 0;
      for (size_t i = 0; i < utf32.size(); ++i) {        

        canvas.drawSimpleText(&utf32[i],
                              sizeof(utf32[i]),
                              SkTextEncoding::kUTF32,
                              offset - textBlob->bounds().fLeft,
                              -textBlob->bounds().fTop,
                              glyphs[i].font,
                              paint);

        offset += glyphs[i].width;
      }

      /*canvas.drawTextBlob(
        textBlob, -textBlob->bounds().fLeft, -textBlob->bounds().fTop, paint);*/
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
