#include <cstdint>
#include <vector>
#include <string>
#include <map>

#include "basic-shader-frag.h"
#include "basic-shader-vert.h"
#include "blur-two-pass-shader-frag.h"
#include "border-visual-anti-aliasing-shader-frag.h"
#include "border-visual-anti-aliasing-shader-vert.h"
#include "border-visual-shader-frag.h"
#include "border-visual-shader-vert.h"
#include "color-visual-shader-frag.h"
#include "color-visual-shader-vert.h"
#include "experimental-shader-vert.h"
#include "gradient-visual-shader-0-frag.h"
#include "gradient-visual-shader-0-vert.h"
#include "gradient-visual-shader-1-frag.h"
#include "gradient-visual-shader-1-vert.h"
#include "image-visual-atlas-clamp-shader-frag.h"
#include "image-visual-atlas-various-wrap-shader-frag.h"
#include "image-visual-no-atlas-shader-frag.h"
#include "image-visual-shader-frag.h"
#include "image-visual-shader-vert.h"
#include "mesh-visual-normal-map-shader-frag.h"
#include "mesh-visual-normal-map-shader-vert.h"
#include "mesh-visual-shader-frag.h"
#include "mesh-visual-shader-vert.h"
#include "mesh-visual-simple-shader-frag.h"
#include "mesh-visual-simple-shader-vert.h"
#include "npatch-visual-3x3-shader-vert.h"
#include "npatch-visual-mask-shader-frag.h"
#include "npatch-visual-shader-frag.h"
#include "npatch-visual-shader-vert.h"
#include "primitive-visual-shader-frag.h"
#include "primitive-visual-shader-vert.h"
#include "shadow-view-render-shader-frag.h"
#include "shadow-view-render-shader-vert.h"
#include "text-atlas-l8-shader-frag.h"
#include "text-atlas-rgba-shader-frag.h"
#include "text-atlas-shader-vert.h"
#include "text-decorator-shader-frag.h"
#include "text-decorator-shader-vert.h"
#include "text-scroller-shader-frag.h"
#include "text-scroller-shader-vert.h"
#include "text-visual-multi-color-text-shader-frag.h"
#include "text-visual-multi-color-text-with-style-shader-frag.h"
#include "text-visual-shader-vert.h"
#include "text-visual-single-color-text-shader-frag.h"
#include "text-visual-single-color-text-with-emoji-shader-frag.h"
#include "text-visual-single-color-text-with-style-and-emoji-shader-frag.h"
#include "text-visual-single-color-text-with-style-shader-frag.h"
#include "wireframe-visual-shader-frag.h"
#include "wireframe-visual-shader-vert.h"
static std::map<std::string, std::vector<uint32_t>> gGraphicsBuiltinShader = {
  { "SHADER_BASIC_SHADER_FRAG", SHADER_BASIC_SHADER_FRAG },
  { "SHADER_BASIC_SHADER_VERT", SHADER_BASIC_SHADER_VERT },
  { "SHADER_BLUR_TWO_PASS_SHADER_FRAG", SHADER_BLUR_TWO_PASS_SHADER_FRAG },
  { "SHADER_BORDER_VISUAL_ANTI_ALIASING_SHADER_FRAG", SHADER_BORDER_VISUAL_ANTI_ALIASING_SHADER_FRAG },
  { "SHADER_BORDER_VISUAL_ANTI_ALIASING_SHADER_VERT", SHADER_BORDER_VISUAL_ANTI_ALIASING_SHADER_VERT },
  { "SHADER_BORDER_VISUAL_SHADER_FRAG", SHADER_BORDER_VISUAL_SHADER_FRAG },
  { "SHADER_BORDER_VISUAL_SHADER_VERT", SHADER_BORDER_VISUAL_SHADER_VERT },
  { "SHADER_COLOR_VISUAL_SHADER_FRAG", SHADER_COLOR_VISUAL_SHADER_FRAG },
  { "SHADER_COLOR_VISUAL_SHADER_VERT", SHADER_COLOR_VISUAL_SHADER_VERT },
  { "SHADER_EXPERIMENTAL_SHADER_VERT", SHADER_EXPERIMENTAL_SHADER_VERT },
  { "SHADER_GRADIENT_VISUAL_SHADER_0_FRAG", SHADER_GRADIENT_VISUAL_SHADER_0_FRAG },
  { "SHADER_GRADIENT_VISUAL_SHADER_0_VERT", SHADER_GRADIENT_VISUAL_SHADER_0_VERT },
  { "SHADER_GRADIENT_VISUAL_SHADER_1_FRAG", SHADER_GRADIENT_VISUAL_SHADER_1_FRAG },
  { "SHADER_GRADIENT_VISUAL_SHADER_1_VERT", SHADER_GRADIENT_VISUAL_SHADER_1_VERT },
  { "SHADER_IMAGE_VISUAL_ATLAS_CLAMP_SHADER_FRAG", SHADER_IMAGE_VISUAL_ATLAS_CLAMP_SHADER_FRAG },
  { "SHADER_IMAGE_VISUAL_ATLAS_VARIOUS_WRAP_SHADER_FRAG", SHADER_IMAGE_VISUAL_ATLAS_VARIOUS_WRAP_SHADER_FRAG },
  { "SHADER_IMAGE_VISUAL_NO_ATLAS_SHADER_FRAG", SHADER_IMAGE_VISUAL_NO_ATLAS_SHADER_FRAG },
  { "SHADER_IMAGE_VISUAL_SHADER_FRAG", SHADER_IMAGE_VISUAL_SHADER_FRAG },
  { "SHADER_IMAGE_VISUAL_SHADER_VERT", SHADER_IMAGE_VISUAL_SHADER_VERT },
  { "SHADER_MESH_VISUAL_NORMAL_MAP_SHADER_FRAG", SHADER_MESH_VISUAL_NORMAL_MAP_SHADER_FRAG },
  { "SHADER_MESH_VISUAL_NORMAL_MAP_SHADER_VERT", SHADER_MESH_VISUAL_NORMAL_MAP_SHADER_VERT },
  { "SHADER_MESH_VISUAL_SHADER_FRAG", SHADER_MESH_VISUAL_SHADER_FRAG },
  { "SHADER_MESH_VISUAL_SHADER_VERT", SHADER_MESH_VISUAL_SHADER_VERT },
  { "SHADER_MESH_VISUAL_SIMPLE_SHADER_FRAG", SHADER_MESH_VISUAL_SIMPLE_SHADER_FRAG },
  { "SHADER_MESH_VISUAL_SIMPLE_SHADER_VERT", SHADER_MESH_VISUAL_SIMPLE_SHADER_VERT },
  { "SHADER_NPATCH_VISUAL_3X3_SHADER_VERT", SHADER_NPATCH_VISUAL_3X3_SHADER_VERT },
  { "SHADER_NPATCH_VISUAL_MASK_SHADER_FRAG", SHADER_NPATCH_VISUAL_MASK_SHADER_FRAG },
  { "SHADER_NPATCH_VISUAL_SHADER_FRAG", SHADER_NPATCH_VISUAL_SHADER_FRAG },
  { "SHADER_NPATCH_VISUAL_SHADER_VERT", SHADER_NPATCH_VISUAL_SHADER_VERT },
  { "SHADER_PRIMITIVE_VISUAL_SHADER_FRAG", SHADER_PRIMITIVE_VISUAL_SHADER_FRAG },
  { "SHADER_PRIMITIVE_VISUAL_SHADER_VERT", SHADER_PRIMITIVE_VISUAL_SHADER_VERT },
  { "SHADER_SHADOW_VIEW_RENDER_SHADER_FRAG", SHADER_SHADOW_VIEW_RENDER_SHADER_FRAG },
  { "SHADER_SHADOW_VIEW_RENDER_SHADER_VERT", SHADER_SHADOW_VIEW_RENDER_SHADER_VERT },
  { "SHADER_TEXT_ATLAS_L8_SHADER_FRAG", SHADER_TEXT_ATLAS_L8_SHADER_FRAG },
  { "SHADER_TEXT_ATLAS_RGBA_SHADER_FRAG", SHADER_TEXT_ATLAS_RGBA_SHADER_FRAG },
  { "SHADER_TEXT_ATLAS_SHADER_VERT", SHADER_TEXT_ATLAS_SHADER_VERT },
  { "SHADER_TEXT_DECORATOR_SHADER_FRAG", SHADER_TEXT_DECORATOR_SHADER_FRAG },
  { "SHADER_TEXT_DECORATOR_SHADER_VERT", SHADER_TEXT_DECORATOR_SHADER_VERT },
  { "SHADER_TEXT_SCROLLER_SHADER_FRAG", SHADER_TEXT_SCROLLER_SHADER_FRAG },
  { "SHADER_TEXT_SCROLLER_SHADER_VERT", SHADER_TEXT_SCROLLER_SHADER_VERT },
  { "SHADER_TEXT_VISUAL_MULTI_COLOR_TEXT_SHADER_FRAG", SHADER_TEXT_VISUAL_MULTI_COLOR_TEXT_SHADER_FRAG },
  { "SHADER_TEXT_VISUAL_MULTI_COLOR_TEXT_WITH_STYLE_SHADER_FRAG", SHADER_TEXT_VISUAL_MULTI_COLOR_TEXT_WITH_STYLE_SHADER_FRAG },
  { "SHADER_TEXT_VISUAL_SHADER_VERT", SHADER_TEXT_VISUAL_SHADER_VERT },
  { "SHADER_TEXT_VISUAL_SINGLE_COLOR_TEXT_SHADER_FRAG", SHADER_TEXT_VISUAL_SINGLE_COLOR_TEXT_SHADER_FRAG },
  { "SHADER_TEXT_VISUAL_SINGLE_COLOR_TEXT_WITH_EMOJI_SHADER_FRAG", SHADER_TEXT_VISUAL_SINGLE_COLOR_TEXT_WITH_EMOJI_SHADER_FRAG },
  { "SHADER_TEXT_VISUAL_SINGLE_COLOR_TEXT_WITH_STYLE_AND_EMOJI_SHADER_FRAG", SHADER_TEXT_VISUAL_SINGLE_COLOR_TEXT_WITH_STYLE_AND_EMOJI_SHADER_FRAG },
  { "SHADER_TEXT_VISUAL_SINGLE_COLOR_TEXT_WITH_STYLE_SHADER_FRAG", SHADER_TEXT_VISUAL_SINGLE_COLOR_TEXT_WITH_STYLE_SHADER_FRAG },
  { "SHADER_WIREFRAME_VISUAL_SHADER_FRAG", SHADER_WIREFRAME_VISUAL_SHADER_FRAG },
  { "SHADER_WIREFRAME_VISUAL_SHADER_VERT", SHADER_WIREFRAME_VISUAL_SHADER_VERT },
};

extern "C" {

#define IMPORT_API __attribute__ ((visibility ("default")))

IMPORT_API std::vector<uint32_t> GraphicsGetBuiltinShader( const std::string& tag );

std::vector<uint32_t> GraphicsGetBuiltinShader( const std::string& tag )
{
    auto iter = gGraphicsBuiltinShader.find( tag );
    if( iter != gGraphicsBuiltinShader.end() )
    {
        return iter->second;
    }
    return {};
}

}
