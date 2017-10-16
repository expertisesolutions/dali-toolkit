 /*
 * Copyright (c) 2017 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// CLASS HEADER
#include <dali-toolkit/internal/visuals/visual-factory-impl.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>
#include <dali/public-api/images/image.h>
#include <dali/public-api/object/property-array.h>
#include <dali/public-api/object/type-registry.h>
#include <dali/public-api/object/type-registry-helper.h>
#include <dali/devel-api/scripting/scripting.h>

// INTERNAL INCLUDES
#include <dali-toolkit/public-api/visuals/image-visual-properties.h>
#include <dali-toolkit/public-api/visuals/text-visual-properties.h>
#include <dali-toolkit/public-api/visuals/visual-properties.h>
#include <dali-toolkit/internal/visuals/border/border-visual.h>
#include <dali-toolkit/internal/visuals/color/color-visual.h>
#include <dali-toolkit/internal/visuals/gradient/gradient-visual.h>
#include <dali-toolkit/internal/visuals/image/image-visual.h>
#include <dali-toolkit/internal/visuals/mesh/mesh-visual.h>
#include <dali-toolkit/internal/visuals/npatch/npatch-visual.h>
#include <dali-toolkit/internal/visuals/primitive/primitive-visual.h>
#include <dali-toolkit/internal/visuals/svg/svg-visual.h>
#include <dali-toolkit/internal/visuals/text/text-visual.h>
#include <dali-toolkit/internal/visuals/animated-image/animated-image-visual.h>
#include <dali-toolkit/internal/visuals/wireframe/wireframe-visual.h>
#include <dali-toolkit/internal/visuals/visual-factory-cache.h>
#include <dali-toolkit/internal/visuals/visual-url.h>
#include <dali-toolkit/internal/visuals/visual-string-constants.h>

namespace Dali
{

namespace Toolkit
{

namespace Internal
{

namespace
{

BaseHandle Create()
{
  BaseHandle handle = Toolkit::VisualFactory::Get();

  return handle;
}

DALI_TYPE_REGISTRATION_BEGIN_CREATE( Toolkit::VisualFactory, Dali::BaseHandle, Create, true )
DALI_TYPE_REGISTRATION_END()

} // namespace

VisualFactory::VisualFactory( bool debugEnabled )
:mDebugEnabled( debugEnabled )
{
}

VisualFactory::~VisualFactory()
{
}

Toolkit::Visual::Base VisualFactory::CreateVisual( const Property::Map& propertyMap )
{
  // Create factory cache if it hasn't already been
  if( !mFactoryCache )
  {
    mFactoryCache = new VisualFactoryCache();
  }

  Visual::BasePtr visualPtr;

  Property::Value* typeValue = propertyMap.Find( Toolkit::Visual::Property::TYPE, VISUAL_TYPE );
  Toolkit::Visual::Type visualType = Toolkit::Visual::IMAGE; // Default to IMAGE type.
  if( typeValue )
  {
    Scripting::GetEnumerationProperty( *typeValue, VISUAL_TYPE_TABLE, VISUAL_TYPE_TABLE_COUNT, visualType );
  }

  switch( visualType )
  {
    case Toolkit::Visual::BORDER:
    {
      visualPtr = BorderVisual::New( *( mFactoryCache.Get() ), propertyMap );
      break;
    }

    case Toolkit::Visual::COLOR:
    {
      visualPtr = ColorVisual::New( *( mFactoryCache.Get() ), propertyMap );
      break;
    }

    case Toolkit::Visual::GRADIENT:
    {
      visualPtr = GradientVisual::New( *( mFactoryCache.Get() ), propertyMap );
      break;
    }

    case Toolkit::Visual::IMAGE:
    {
      Property::Value* imageURLValue = propertyMap.Find( Toolkit::ImageVisual::Property::URL, IMAGE_URL_NAME );
      std::string imageUrl;
      if( imageURLValue )
      {
        if( imageURLValue->Get( imageUrl ) )
        {
          VisualUrl visualUrl( imageUrl );

          switch( visualUrl.GetType() )
          {
            case VisualUrl::N_PATCH:
            {
              visualPtr = NPatchVisual::New( *( mFactoryCache.Get() ), visualUrl, propertyMap );
              break;
            }
            case VisualUrl::SVG:
            {
              visualPtr = SvgVisual::New( *( mFactoryCache.Get() ), visualUrl, propertyMap );
              break;
            }
            case VisualUrl::GIF:
            {
              visualPtr = AnimatedImageVisual::New( *( mFactoryCache.Get() ), visualUrl, propertyMap );
              break;
            }
            case VisualUrl::REGULAR_IMAGE:
            {
              visualPtr = ImageVisual::New( *( mFactoryCache.Get() ), visualUrl, propertyMap );
              break;
            }
          }
        }
        else
        {
          Property::Array* array = imageURLValue->GetArray();
          if( array )
          {
            visualPtr = AnimatedImageVisual::New( *( mFactoryCache.Get() ), *array, propertyMap );
          }
        }
      }
      break;
    }

    case Toolkit::Visual::MESH:
    {
      visualPtr = MeshVisual::New( *( mFactoryCache.Get() ), propertyMap );
      break;
    }

    case Toolkit::Visual::PRIMITIVE:
    {
      visualPtr = PrimitiveVisual::New( *( mFactoryCache.Get() ), propertyMap );
      break;
    }

    case Toolkit::Visual::WIREFRAME:
    {
      visualPtr = WireframeVisual::New( *( mFactoryCache.Get() ), propertyMap );
      break;
    }

    case Toolkit::Visual::TEXT:
    {
      visualPtr = TextVisual::New( *( mFactoryCache.Get() ), propertyMap );
      break;
    }

    case Toolkit::Visual::N_PATCH:
    {
      Property::Value* imageURLValue = propertyMap.Find( Toolkit::ImageVisual::Property::URL, IMAGE_URL_NAME );
      std::string imageUrl;
      if( imageURLValue && imageURLValue->Get( imageUrl ) )
      {
        visualPtr = NPatchVisual::New( *( mFactoryCache.Get() ), imageUrl, propertyMap );
      }
      break;
    }

    case Toolkit::Visual::SVG:
    {
      Property::Value* imageURLValue = propertyMap.Find( Toolkit::ImageVisual::Property::URL, IMAGE_URL_NAME );
      std::string imageUrl;
      if( imageURLValue && imageURLValue->Get( imageUrl ) )
      {
        visualPtr = SvgVisual::New( *( mFactoryCache.Get() ), imageUrl, propertyMap );
      }
      break;
    }

    case Toolkit::Visual::ANIMATED_IMAGE:
    {
      Property::Value* imageURLValue = propertyMap.Find( Toolkit::ImageVisual::Property::URL, IMAGE_URL_NAME );
      std::string imageUrl;
      if( imageURLValue )
      {
        if( imageURLValue->Get( imageUrl ) )
        {
          visualPtr = AnimatedImageVisual::New( *( mFactoryCache.Get() ), imageUrl, propertyMap );
        }
        else
        {
          Property::Array* array = imageURLValue->GetArray();
          if( array )
          {
            visualPtr = AnimatedImageVisual::New( *( mFactoryCache.Get() ), *array, propertyMap );
          }
        }
      }
      break;
    }
  }

  if( !visualPtr )
  {
    DALI_LOG_ERROR( "Renderer type unknown\n" );
  }

  if( mDebugEnabled && visualType !=  Toolkit::Visual::WIREFRAME )
  {
    //Create a WireframeVisual if we have debug enabled
    visualPtr = WireframeVisual::New( *( mFactoryCache.Get() ), visualPtr, propertyMap );
  }

  return Toolkit::Visual::Base( visualPtr.Get() );
}

Toolkit::Visual::Base VisualFactory::CreateVisual( const Image& image )
{
  if( !mFactoryCache )
  {
    mFactoryCache = new VisualFactoryCache();
  }

  Visual::BasePtr visualPtr;

  NinePatchImage npatchImage = NinePatchImage::DownCast( image );
  if( npatchImage )
  {
    visualPtr = NPatchVisual::New( *( mFactoryCache.Get() ), npatchImage );
  }
  else
  {
    visualPtr = ImageVisual::New( *( mFactoryCache.Get() ), image );
  }

  if( mDebugEnabled )
  {
    //Create a WireframeVisual if we have debug enabled
    visualPtr = WireframeVisual::New( *( mFactoryCache.Get() ), visualPtr );
  }

  return Toolkit::Visual::Base( visualPtr.Get() );
}

Toolkit::Visual::Base VisualFactory::CreateVisual( const std::string& url, ImageDimensions size )
{
  if( !mFactoryCache )
  {
    mFactoryCache = new VisualFactoryCache();
  }

  Visual::BasePtr visualPtr;

  // first resolve url type to know which visual to create
  VisualUrl visualUrl( url );
  switch( visualUrl.GetType() )
  {
    case VisualUrl::N_PATCH:
    {
      visualPtr = NPatchVisual::New( *( mFactoryCache.Get() ), visualUrl );
      break;
    }
    case VisualUrl::SVG:
    {
      visualPtr = SvgVisual::New( *( mFactoryCache.Get() ), visualUrl );
      break;
    }
    case VisualUrl::GIF:
    {
      visualPtr = AnimatedImageVisual::New( *( mFactoryCache.Get() ), visualUrl );
      break;
    }
    case VisualUrl::REGULAR_IMAGE:
    {
      visualPtr = ImageVisual::New( *( mFactoryCache.Get() ), visualUrl, size );
      break;
    }
  }

  if( mDebugEnabled )
  {
    //Create a WireframeVisual if we have debug enabled
    visualPtr = WireframeVisual::New( *( mFactoryCache.Get() ), visualPtr );
  }

  return Toolkit::Visual::Base( visualPtr.Get() );
}

Internal::TextureManager& VisualFactory::GetTextureManager()
{
  if( !mFactoryCache )
  {
    mFactoryCache = new VisualFactoryCache();
  }
  return mFactoryCache->GetTextureManager();
}

} // namespace Internal

} // namespace Toolkit

} // namespace Dali