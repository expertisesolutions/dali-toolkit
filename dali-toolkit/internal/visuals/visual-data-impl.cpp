/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd.
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
 *
 */

// CLASS HEADER
#include "visual-data-impl.h"

// EXTERNAL HEADER
#include <dali/public-api/common/dali-common.h>
#include <dali/integration-api/debug.h>
#include <dali/public-api/object/property-array.h>

// EXTERNAL HEADER

namespace Dali
{

namespace Toolkit
{

namespace Internal
{

namespace
{
//custom shader
const char * const CUSTOM_SHADER( "shader" );
const char * const CUSTOM_VERTEX_SHADER( "vertexShader" );
const char * const CUSTOM_FRAGMENT_SHADER( "fragmentShader" );
const char * const CUSTOM_SUBDIVIDE_GRID_X( "subdivideGridX" );
const char * const CUSTOM_SUBDIVIDE_GRID_Y( "subdivideGridY" );
const char * const CUSTOM_SHADER_HINTS( "hints" ); ///< type STRING for a hint from the below hint strings or an ARRAY of of hint strings

/**
 * where hints should be contain strings of the following shader hints:
 *   "none"                    | corresponds to Shader::Hint::NONE
 *   "outputIsTransparent"     | corresponds to Shader::Hint::OUTPUT_IS_TRANSPARENT
 *   "modifiesGeometry"        | corresponds to Shader::Hint::MODIFIES_GEOMETRY
 */

Shader::Hint::Value HintFromString( std::string hintString )
{
  if( hintString == "none" )
  {
    return Shader::Hint::NONE;
  }
  else if( hintString == "outputIsTransparent" )
  {
    return Shader::Hint::OUTPUT_IS_TRANSPARENT;
  }
  else if( hintString == "modifiesGeometry" )
  {
    return Shader::Hint::MODIFIES_GEOMETRY;
  }
  else
  {
    DALI_LOG_ERROR( "'%s' hint string is not recognised", hintString.c_str() );
  }

  return Shader::Hint::NONE;
}

}// unnamed namespace

Internal::Visual::Impl::Impl()
: mCustomShader(NULL),
  mDepthIndex( 0.0f ),
  mFlags( 0 )
{
}

Internal::Visual::Impl::~Impl()
{
  delete mCustomShader;
}

Internal::Visual::Impl::CustomShader::CustomShader( const Property::Map& map )
: mGridSize( 1, 1 ),
  mHints( Shader::Hint::NONE )
{
  SetPropertyMap( map );
}

void Internal::Visual::Impl::CustomShader::SetPropertyMap( const Property::Map& shaderMap )
{
  mVertexShader.clear();
  mFragmentShader.clear();
  mGridSize = ImageDimensions( 1, 1 );
  mHints = Shader::Hint::NONE;

  Property::Value* vertexShaderValue = shaderMap.Find( CUSTOM_VERTEX_SHADER );
  if( vertexShaderValue )
  {
    if( !vertexShaderValue->Get( mVertexShader ) )
    {
      DALI_LOG_ERROR( "'%s' parameter does not correctly specify a string", CUSTOM_VERTEX_SHADER );
    }
  }

  Property::Value* fragmentShaderValue = shaderMap.Find( CUSTOM_FRAGMENT_SHADER );
  if( fragmentShaderValue )
  {
    if( !fragmentShaderValue->Get( mFragmentShader ) )
    {
      DALI_LOG_ERROR( "'%s' parameter does not correctly specify a string", CUSTOM_FRAGMENT_SHADER );
    }
  }

  Property::Value* subdivideXValue = shaderMap.Find( CUSTOM_SUBDIVIDE_GRID_X );
  if( subdivideXValue )
  {
    int subdivideX;
    if( !subdivideXValue->Get( subdivideX ) || subdivideX < 1 )
    {
      DALI_LOG_ERROR( "'%s' parameter does not correctly specify a value greater than 1", CUSTOM_SUBDIVIDE_GRID_X );
    }
    else
    {
      mGridSize = ImageDimensions( subdivideX, mGridSize.GetY() );
    }
  }

  Property::Value* subdivideYValue = shaderMap.Find( CUSTOM_SUBDIVIDE_GRID_Y );
  if( subdivideYValue )
  {
    int subdivideY;
    if( !subdivideYValue->Get( subdivideY ) || subdivideY < 1 )
    {
      DALI_LOG_ERROR( "'%s' parameter does not correctly specify a value greater than 1", CUSTOM_SUBDIVIDE_GRID_Y );
    }
    else
    {
      mGridSize = ImageDimensions( mGridSize.GetX(), subdivideY );
    }
  }

  Property::Value* hintsValue = shaderMap.Find( CUSTOM_SHADER_HINTS );
  if( hintsValue )
  {
    std::string hintString;
    Property::Array hintsArray;

    if( hintsValue->Get( hintString ) )
    {
      mHints = HintFromString( hintString );
    }
    else if( hintsValue->Get( hintsArray ) )
    {
      int hints = Shader::Hint::NONE;
      for( Property::Array::SizeType i = 0; i < hintsArray.Count(); ++i)
      {
        Property::Value hintValue = hintsArray[ i ];
        if( hintValue.Get( hintString ) )
        {
          hints |= static_cast< int >( HintFromString( hintString ) );
        }
        else
        {
          DALI_LOG_ERROR( "'%s' parameter does not correctly specify an hint string at index %d", CUSTOM_SHADER_HINTS, i );
        }

        mHints = static_cast< Shader::Hint::Value >( hints );
      }
    }
    else
    {
      DALI_LOG_ERROR( "'%s' parameter does not correctly specify a hint string or an array of hint strings", CUSTOM_SHADER_HINTS );
    }
  }
}

void Internal::Visual::Impl::CustomShader::CreatePropertyMap( Property::Map& map ) const
{
  if( !mVertexShader.empty() || !mFragmentShader.empty() )
  {
    Property::Map customShader;
    if( !mVertexShader.empty() )
    {
      customShader.Insert(CUSTOM_VERTEX_SHADER, mVertexShader );
    }
    if( !mFragmentShader.empty() )
    {
      customShader.Insert(CUSTOM_FRAGMENT_SHADER, mFragmentShader );
    }

    if( mGridSize.GetWidth() != 1 )
    {
      customShader.Insert(CUSTOM_SUBDIVIDE_GRID_X, mGridSize.GetWidth() );
    }
    if( mGridSize.GetHeight() != 1 )
    {
      customShader.Insert(CUSTOM_SUBDIVIDE_GRID_Y, mGridSize.GetHeight() );
    }

    if( mHints != Dali::Shader::Hint::NONE )
    {
      customShader.Insert(CUSTOM_SHADER_HINTS, static_cast< int >(mHints) );
    }

    map.Insert( CUSTOM_SHADER, customShader );
  }
}

} // namespace Internal

} // namespace Toolkit

} // namespace Dali
