#pragma once

#include "include/core/SkTypeface.h"

class IFontFactory
{
public:
  virtual ~IFontFactory() {}
  virtual sk_sp<SkTypeface> load(const char* familyName) = 0;
};