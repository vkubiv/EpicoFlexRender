#pragma once

#include <functional>
#include <memory>
#include <string>

class SkCanvas;
class IFontFactory;

void
RasterToImage(int width,
              int height,
              const char* outputPath,
              std::function<void(SkCanvas&)> draw);

void
RenderJsonToCanvas(const std::string& stringWithJson,
                   SkCanvas& canvas,
                   std::unique_ptr<IFontFactory> fontFactory);
