#ifndef DALI_TOOLKIT_INTERNAL_ANIMATED_IMAGE_VISUAL_H
#define DALI_TOOLKIT_INTERNAL_ANIMATED_IMAGE_VISUAL_H

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
 *
 */

// EXTERNAL INCLUDES
#include <dali/public-api/common/dali-vector.h>
#include <dali/public-api/common/intrusive-ptr.h>
#include <dali/public-api/math/vector4.h>
#include <dali/public-api/object/weak-handle.h>
#include <dali/public-api/adaptor-framework/timer.h>

// INTERNAL INCLUDES
#include <dali-toolkit/internal/visuals/visual-base-impl.h>
#include <dali-toolkit/internal/visuals/visual-url.h>
#include <dali-toolkit/internal/visuals/animated-image/image-cache.h>

namespace Dali
{

namespace Toolkit
{

namespace Internal
{

class AnimatedImageVisual;
typedef IntrusivePtr< AnimatedImageVisual > AnimatedImageVisualPtr;

/**
 * The visual which renders an animated image
 *
 * One of the following properties is mandatory
 *
 * | %Property Name     | Type              |
 * |------------------- |-------------------|
 * | url                | STRING            |
 * | urls               | ARRAY of STRING   |
 *
 * The remaining properties are optional
 * | pixelArea          | VECTOR4           |
 * | wrapModeU          | INTEGER OR STRING |
 * | wrapModeV          | INTEGER OR STRING |
 * | cacheSize          | INTEGER           |
 * | batchSize          | INTEGER           |
 * | frameDelay         | INTEGER           |
 *
 * pixelArea is a rectangular area.
 * In its Vector4 value, the first two elements indicate the top-left position of the area,
 * and the last two elements are the area width and height respectively.
 * If not specified, the default value is [0.0, 0.0, 1.0, 1.0], i.e. the entire area of the image.
 *
 * wrapModeU and wrapModeV separately decide how the texture should be sampled when the u and v coordinate exceeds the range of 0.0 to 1.0.
 * Its value should be one of the following wrap mode:
 *   "DEFAULT"
 *   "CLAMP_TO_EDGE"
 *   "REPEAT"
 *   "MIRRORED_REPEAT"
 *
 * cacheSize is used with multiple images - it determines how many images are kept pre-loaded
 * batchSize is used with multiple images - it determines how many images to load on each frame
 * frameDelay is used with multiple images - it is the number of milliseconds between each frame
 */

class AnimatedImageVisual : public Visual::Base,
                            public ConnectionTracker,
                            public ImageCache::FrameReadyObserver
{

public:

  /**
   * @brief Create the animated image Visual using the image URL.
   *
   * @param[in] factoryCache A pointer pointing to the VisualFactoryCache object
   * @param[in] imageUrl The URL to gif resource to use
   * @param[in] properties A Property::Map containing settings for this visual
   * @return A smart-pointer to the newly allocated visual.
   */
  static AnimatedImageVisualPtr New( VisualFactoryCache& factoryCache, const VisualUrl& imageUrl, const Property::Map& properties );

  /**
   * @brief Create the animated image Visual using image URLs.
   *
   * @param[in] factoryCache A pointer pointing to the VisualFactoryCache object
   * @param[in] imageUrls A Property::Array containing the URLs to the image resources
   * @param[in] properties A Property::Map containing settings for this visual
   * @return A smart-pointer to the newly allocated visual.
   */
  static AnimatedImageVisualPtr New( VisualFactoryCache& factoryCache, const Property::Array& imageUrls, const Property::Map& properties );

  /**
   * @brief Create the animated image visual using the image URL.
   *
   * @param[in] factoryCache A pointer pointing to the VisualFactoryCache object
   * @param[in] imageUrl The URL to animated image resource to use
   */
  static AnimatedImageVisualPtr New( VisualFactoryCache& factoryCache, const VisualUrl& imageUrl );

public:  // from Visual

  /**
   * @copydoc Visual::Base::GetNaturalSize
   */
  virtual void GetNaturalSize( Vector2& naturalSize );

  /**
   * @copydoc Visual::Base::CreatePropertyMap
   */
  virtual void DoCreatePropertyMap( Property::Map& map ) const;

  /**
   * @copydoc Visual::Base::CreateInstancePropertyMap
   */
  virtual void DoCreateInstancePropertyMap( Property::Map& map ) const;

protected:

  /**
   * @brief Constructor.
   *
   * @param[in] factoryCache A pointer pointing to the VisualFactoryCache object
   */
  AnimatedImageVisual( VisualFactoryCache& factoryCache );

  /**
   * @brief A reference counted object may only be deleted by calling Unreference().
   */
  virtual ~AnimatedImageVisual();

  /**
   * @copydoc Visual::Base::DoSetProperties
   */
  virtual void DoSetProperties( const Property::Map& propertyMap );

  /**
   * Helper method to set individual values by index key.
   * @param[in] index The index key of the value
   * @param[in] value The value
   */
  void DoSetProperty( Property::Index index, const Property::Value& value );

  /**
   * @copydoc Visual::Base::DoSetOnStage
   */
  virtual void DoSetOnStage( Actor& actor );

  /**
   * @copydoc Visual::Base::DoSetOffStage
   */
  virtual void DoSetOffStage( Actor& actor );

  /**
   * @copydoc Visual::Base::OnSetTransform
   */
  virtual void OnSetTransform();

private:
  /**
   * Creates the renderer for the animated image
   */
  void CreateRenderer();

  /**
   * Starts the Load of the first batch of URLs
   */
  void LoadFirstBatch();

  /**
   * Adds the texture set to the renderer, and the renderer to the
   * placement actor, and starts the frame timer
   */
  void StartFirstFrame( TextureSet& textureSet );

  /**
   * Prepares the texture set for displaying
   */
  TextureSet PrepareTextureSet();

  /**
   * Load the gif image and pack the frames into atlas.
   * @return The atlas texture.
   */
  TextureSet PrepareAnimatedGifImage();

  /**
   * Set the image size from the texture set
   */
  void SetImageSize( TextureSet& textureSet );

  /**
   * Called when the next frame is ready.
   */
  void FrameReady( TextureSet textureSet );

  /**
   * Display the next frame. It is called when the mFrameDelayTimer ticks.
   * Returns true to ensure the timer continues running.
   */
  bool DisplayNextFrame();


  // Undefined
  AnimatedImageVisual( const AnimatedImageVisual& animatedImageVisual );

  // Undefined
  AnimatedImageVisual& operator=( const AnimatedImageVisual& animatedImageVisual );

private:

  Timer mFrameDelayTimer;
  WeakHandle<Actor> mPlacementActor;

  // Variables for GIF player
  Dali::Vector<Vector4> mTextureRectContainer;
  Dali::Vector<uint32_t> mFrameDelayContainer;
  Vector4 mPixelArea;
  VisualUrl mImageUrl;
  uint32_t mCurrentFrameIndex; // Frame index into textureRects

  // Variables for Multi-Image player
  ImageCache::UrlList* mImageUrls;
  ImageCache* mImageCache;
  uint16_t mCacheSize;
  uint16_t mBatchSize;
  uint16_t mFrameDelay;
  uint16_t mUrlIndex;

  // Shared variables
  ImageDimensions mImageSize;

  Dali::WrapMode::Type mWrapModeU:3;
  Dali::WrapMode::Type mWrapModeV:3;
  bool mStartFirstFrame:1;
};

} // namespace Internal

} // namespace Toolkit

} // namespace Dali
#endif /* DALI_TOOLKIT_INTERNAL_ANIMATED_IMAGE_VISUAL_H */