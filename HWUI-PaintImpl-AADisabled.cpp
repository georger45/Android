/*
 * Copyright (C) 2013 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "Paint.h"

namespace android {

Paint::Paint()
        : SkPaint()
        , mLetterSpacing(0)
        , mWordSpacing(0)
        , mFontFeatureSettings()
        , mMinikinLocaleListId(0)
        , mFamilyVariant(minikin::FamilyVariant::DEFAULT) {
    // SkPaint::antialiasing defaults to false, but
    // SkFont::edging defaults to kAntiAlias. To keep them
    // insync, we manually set the font to kAilas.
    mFont.setEdging(SkFont::Edging::kAlias);
}

Paint::Paint(const Paint& paint)
        : SkPaint(paint)
        , mFont(paint.mFont)
        , mLetterSpacing(paint.mLetterSpacing)
        , mWordSpacing(paint.mWordSpacing)
        , mFontFeatureSettings(paint.mFontFeatureSettings)
        , mMinikinLocaleListId(paint.mMinikinLocaleListId)
        , mFamilyVariant(paint.mFamilyVariant)
        , mHyphenEdit(paint.mHyphenEdit)
        , mTypeface(paint.mTypeface)
        , mAlign(paint.mAlign)
        , mStrikeThru(paint.mStrikeThru)
        , mUnderline(paint.mUnderline)
        , mDevKern(paint.mDevKern) {}


Paint::~Paint() {}

Paint& Paint::operator=(const Paint& other) {
    SkPaint::operator=(other);
    mFont = other.mFont;
    mLetterSpacing = other.mLetterSpacing;
    mWordSpacing = other.mWordSpacing;
    mFontFeatureSettings = other.mFontFeatureSettings;
    mMinikinLocaleListId = other.mMinikinLocaleListId;
    mFamilyVariant = other.mFamilyVariant;
    mHyphenEdit = other.mHyphenEdit;
    mTypeface = other.mTypeface;
    mAlign = other.mAlign;
    mStrikeThru = other.mStrikeThru;
    mUnderline = other.mUnderline;
    mDevKern = other.mDevKern;
    return *this;
}

bool operator==(const Paint& a, const Paint& b) {
    return static_cast<const SkPaint&>(a) == static_cast<const SkPaint&>(b) &&
           a.mFont == b.mFont &&
           a.mLetterSpacing == b.mLetterSpacing && a.mWordSpacing == b.mWordSpacing &&
           a.mFontFeatureSettings == b.mFontFeatureSettings &&
           a.mMinikinLocaleListId == b.mMinikinLocaleListId &&
           a.mFamilyVariant == b.mFamilyVariant && a.mHyphenEdit == b.mHyphenEdit &&
           a.mTypeface == b.mTypeface && a.mAlign == b.mAlign &&
           a.mStrikeThru == b.mStrikeThru && a.mUnderline == b.mUnderline &&
           a.mDevKern == b.mDevKern;
}

void Paint::reset() {
    SkPaint::reset();

    mFont = SkFont();
    mFont.setEdging(SkFont::Edging::kAlias);

    mStrikeThru = false;
    mUnderline = false;
    mDevKern = false;
}

void Paint::setAntiAlias(bool aa) {
    // Java does not support/understand subpixel(lcd) antialiasing
    SkASSERT(mFont.getEdging() != SkFont::Edging::kSubpixelAntiAlias);
    // JavaPaint antialiasing affects both the SkPaint and SkFont settings.

    //SkPaint::setAntiAlias(aa);                                                    * ORIGINAL CODE COMMENTED OUT.
    //mFont.setEdging(aa ? SkFont::Edging::kAntiAlias : SkFont::Edging::kAlias);    *
    SkPaint::setAntiAlias(false);                                                   // NEW CODE TO TURN ANTIALIASING OFF.
    mFont.setEdging(SkFont::Edging::kAlias);                                        //
}

////////////////// Java flags compatibility //////////////////

/*  Flags are tricky. Java has its own idea of the "paint" flags, but they don't really
    match up with skia anymore, so we have to do some shuffling in get/set flags()

	3 flags apply to SkPaint (antialias, dither, filter -> enum)
    5 flags (merged with antialias) are for SkFont
    2 flags are for minikin::Paint (underline and strikethru)
*/

// flags relating to SkPaint
static const uint32_t sAntiAliasFlag    = 0x01;   // affects paint and font-edging
static const uint32_t sFilterBitmapFlag = 0x02;   // maps to enum
static const uint32_t sDitherFlag       = 0x04;
// flags relating to SkFont
static const uint32_t sFakeBoldFlag     = 0x020;
static const uint32_t sLinearMetrics    = 0x040;
static const uint32_t sSubpixelMetrics  = 0x080;
static const uint32_t sEmbeddedBitmaps  = 0x400;
static const uint32_t sForceAutoHinting = 0x800;
// flags related to minikin::Paint
static const uint32_t sUnderlineFlag    = 0x08;
static const uint32_t sStrikeThruFlag   = 0x10;
// flags no longer supported on native side (but mirrored for compatibility)
static const uint32_t sDevKernFlag      = 0x100;

static uint32_t paintToLegacyFlags(const SkPaint& paint) {
    uint32_t flags = 0;
    flags |= -(int)paint.isAntiAlias() & sAntiAliasFlag;
    flags |= -(int)paint.isDither()    & sDitherFlag;
    if (paint.getFilterQuality() != kNone_SkFilterQuality) {
        flags |= sFilterBitmapFlag;
    }
    return flags;
}

static uint32_t fontToLegacyFlags(const SkFont& font) {
    uint32_t flags = 0;
    flags |= -(int)font.isEmbolden()         & sFakeBoldFlag;
    flags |= -(int)font.isLinearMetrics()    & sLinearMetrics;
    flags |= -(int)font.isSubpixel()         & sSubpixelMetrics;
    flags |= -(int)font.isEmbeddedBitmaps()  & sEmbeddedBitmaps;
    flags |= -(int)font.isForceAutoHinting() & sForceAutoHinting;
    return flags;
}

static void applyLegacyFlagsToPaint(uint32_t flags, SkPaint* paint) {
    //paint->setAntiAlias((flags & sAntiAliasFlag) != 0);
    paint->setAntiAlias(false);
    paint->setDither   ((flags & sDitherFlag) != 0);

    if (flags & sFilterBitmapFlag) {
        paint->setFilterQuality(kLow_SkFilterQuality);
    } else {
        paint->setFilterQuality(kNone_SkFilterQuality);
    }
}

static void applyLegacyFlagsToFont(uint32_t flags, SkFont* font) {
    font->setEmbolden        ((flags & sFakeBoldFlag) != 0);
    font->setLinearMetrics   ((flags & sLinearMetrics) != 0);
    font->setSubpixel        ((flags & sSubpixelMetrics) != 0);
    font->setEmbeddedBitmaps ((flags & sEmbeddedBitmaps) != 0);
    font->setForceAutoHinting((flags & sForceAutoHinting) != 0);

    //if (flags & sAntiAliasFlag) {                          *
    //    font->setEdging(SkFont::Edging::kAntiAlias);       *
    //} else {                                               * ORIGINAL CODE COMMENTED OUT.
    //    font->setEdging(SkFont::Edging::kAlias);           *
    //}                                                      *			
    font->setEdging(SkFont::Edging::kAlias);                 // NEW CODE TO TURN FONT SMOOTHING OFF.
}

uint32_t Paint::GetSkPaintJavaFlags(const SkPaint& paint) {
    return paintToLegacyFlags(paint);
}

void Paint::SetSkPaintJavaFlags(SkPaint* paint, uint32_t flags) {
    applyLegacyFlagsToPaint(flags, paint);
}

uint32_t Paint::getJavaFlags() const {
    uint32_t flags = paintToLegacyFlags(*this) | fontToLegacyFlags(mFont);
    flags |= -(int)mStrikeThru & sStrikeThruFlag;
    flags |= -(int)mUnderline  & sUnderlineFlag;
    flags |= -(int)mDevKern    & sDevKernFlag;
    return flags;
}

void Paint::setJavaFlags(uint32_t flags) {
    applyLegacyFlagsToPaint(flags, this);
    applyLegacyFlagsToFont(flags, &mFont);
    mStrikeThru = (flags & sStrikeThruFlag) != 0;
    mUnderline  = (flags & sUnderlineFlag) != 0;
    mDevKern    = (flags & sDevKernFlag) != 0;
}

}  // namespace android
