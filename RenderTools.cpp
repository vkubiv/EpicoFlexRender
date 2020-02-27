#include "RenderTools.h"
#include "Document.h"
#include "DocumentRender.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkSurface.h"

void
RasterToImage(int width,
              int height,
              const char* outputPath,
              std::function<void(SkCanvas&)> draw)
{
  sk_sp<SkSurface> rasterSurface =
    SkSurface::MakeRasterN32Premul(width, height);
  SkCanvas* rasterCanvas = rasterSurface->getCanvas();
  draw(*rasterCanvas);
  sk_sp<SkImage> img(rasterSurface->makeImageSnapshot());
  if (!img) {
    return;
  }
  sk_sp<SkData> png(img->encodeToData());
  if (!png) {
    return;
  }
  SkFILEWStream out(outputPath);
  (void)out.write(png->data(), png->size());
}

void
RenderJsonToCanvas(const std::string& stringWithJson,
                   SkCanvas& canvas,
                   std::unique_ptr<IFontFactory> fontFactory)
{
  DocumentRender render(std::move(fontFactory));

  json::Document inputJson;
  inputJson.Parse(stringWithJson.c_str());
  Document renderDocument(std::move(inputJson),
                          render.getDrawAttributesReader());

  renderDocument.buildYoga();
  auto rootNode = renderDocument.getRootNode();
  auto canvasSize = canvas.getBaseLayerSize();
  YGNodeCalculateLayout(
    rootNode, canvasSize.fWidth, canvasSize.fHeight, YGDirectionLTR);

  render.renderNode(canvas, rootNode);
}