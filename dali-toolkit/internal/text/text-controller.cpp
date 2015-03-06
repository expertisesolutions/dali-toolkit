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
#include <dali-toolkit/internal/text/text-controller.h>

// INTERNAL INCLUDES
#include <dali-toolkit/internal/text/character-set-conversion.h>
#include <dali-toolkit/internal/text/layouts/layout-engine.h>
#include <dali-toolkit/internal/text/layouts/layout-parameters.h>
#include <dali-toolkit/internal/text/logical-model.h>
#include <dali-toolkit/internal/text/multi-language-support.h>
#include <dali-toolkit/internal/text/script-run.h>
#include <dali-toolkit/internal/text/segmentation.h>
#include <dali-toolkit/internal/text/shaper.h>
#include <dali-toolkit/internal/text/text-view.h>
#include <dali-toolkit/internal/text/visual-model.h>

// EXTERNAL INCLUDES
#include <limits>
#include <vector>
#include <dali/public-api/text-abstraction/font-client.h>

using std::vector;

namespace Dali
{

namespace Toolkit
{

namespace Text
{

struct Controller::TextInput
{
  // Used to queue input events until DoRelayout()
  enum EventType
  {
    KEYBOARD_FOCUS_GAIN_EVENT,
    KEYBOARD_FOCUS_LOST_EVENT,
    TAP_EVENT,
    GRAB_HANDLE_EVENT
  };

  union Param
  {
    int mInt;
    unsigned int mUint;
    float mFloat;
  };

  struct Event
  {
    Event( EventType eventType )
    : type( eventType )
    {
      p1.mInt = 0;
      p2.mInt = 0;
    }

    EventType type;
    Param p1;
    Param p2;
    Param p3;
  };

  enum State
  {
    INACTIVE,
    SELECTING,
    EDITING
  };

  TextInput( LogicalModelPtr logicalModel,
             VisualModelPtr visualModel,
             DecoratorPtr decorator )
  : mLogicalModel( logicalModel ),
    mVisualModel( visualModel ),
    mDecorator( decorator ),
    mState( INACTIVE )
  {
  }

  /**
   * @brief Helper to move the cursor, grab handle etc.
   */
  bool ProcessTouchEvents()
  {
    mDecoratorUpdated = false;

    if( mDecorator )
    {
      for( vector<TextInput::Event>::iterator iter = mEventQueue.begin(); iter != mEventQueue.end(); ++iter )
      {
        switch( iter->type )
        {
          case KEYBOARD_FOCUS_GAIN_EVENT:
          {
            OnKeyboardFocus( true );
            break;
          }
          case KEYBOARD_FOCUS_LOST_EVENT:
          {
            OnKeyboardFocus( false );
            break;
          }
          case TAP_EVENT:
          {
            OnTapEvent( *iter );
            break;
          }
          case GRAB_HANDLE_EVENT:
          {
            OnGrabHandleEvent( *iter );
            break;
          }
        }
      }
    }

    mEventQueue.clear();

    return mDecoratorUpdated;
  }

  void OnKeyboardFocus( bool hasFocus )
  {
    // TODO
  }

  void OnTapEvent( const Event& event )
  {
    if( 1u == event.p1.mUint )
    {
      mState = TextInput::EDITING;
      mDecorator->SetActiveCursor( ACTIVE_CURSOR_PRIMARY );
      mDecorator->StartCursorBlink();
      mDecorator->SetGrabHandleActive( true );

      float xPosition = event.p2.mFloat;
      float yPosition = event.p3.mFloat;
      float height(0.0f);
      GetClosestCursorPosition( xPosition, yPosition, height );
      mDecorator->SetPosition( PRIMARY_CURSOR, xPosition, yPosition, height );

      mDecoratorUpdated = true;
    }
    else if( 2u == event.p1.mUint )
    {
      mState = TextInput::SELECTING;
      mDecorator->SetGrabHandleActive( false );
      mDecorator->SetSelectionActive( true );
      mDecoratorUpdated = true;
    }
  }

  void OnGrabHandleEvent( const Event& event )
  {
    unsigned int state = event.p1.mUint;

    if( GRAB_HANDLE_PRESSED == state )
    {
      float xPosition = event.p2.mFloat;
      float yPosition = event.p3.mFloat;
      float height(0.0f);

      GetClosestCursorPosition( xPosition, yPosition, height );

      mDecorator->SetPosition( PRIMARY_CURSOR, xPosition, yPosition, height );
      mDecoratorUpdated = true;
    }
  }

  void GetClosestCursorPosition( float& x, float& y, float& height )
  {
    // TODO - Look at LineRuns first

    Text::Length numberOfGlyphs = mVisualModel->GetNumberOfGlyphs();
    if( 0 == numberOfGlyphs )
    {
      return;
    }

    Vector<GlyphInfo> glyphs;
    glyphs.Resize( numberOfGlyphs );
    mVisualModel->GetGlyphs( &glyphs[0], 0, numberOfGlyphs );

    std::vector<Vector2> positions;
    positions.resize( numberOfGlyphs );
    mVisualModel->GetGlyphPositions( &positions[0], 0, numberOfGlyphs );

    unsigned int closestGlyph = 0;
    float closestDistance = std::numeric_limits<float>::max();

    for( unsigned int i=0; i<glyphs.Count(); ++i )
    {
      float glyphX = positions[i].x + glyphs[i].width*0.5f;
      float glyphY = positions[i].y + glyphs[i].height*0.5f;

      float distanceToGlyph = fabsf( glyphX - x ) + fabsf( glyphY - y );

      if( distanceToGlyph < closestDistance )
      {
        closestDistance = distanceToGlyph;
        closestGlyph = i;
      }
    }

    // TODO - Consider RTL languages
    x = positions[closestGlyph].x + glyphs[closestGlyph].width;
    y = 0.0f;

    FontMetrics metrics;
    TextAbstraction::FontClient::Get().GetFontMetrics( glyphs[closestGlyph].fontId, metrics );
    height = metrics.height; // TODO - Fix for multi-line
  }

  LogicalModelPtr mLogicalModel;
  VisualModelPtr  mVisualModel;
  DecoratorPtr    mDecorator;

  State mState;

  /**
   * This is used to delay handling events until after the model has been updated.
   * The number of updates to the model is minimized to improve performance.
   */
  vector<Event> mEventQueue; ///< The queue of touch events etc.

  bool mDecoratorUpdated;
};

struct Controller::Impl
{
  Impl( ControlInterface& controlInterface )
  : mControlInterface( controlInterface ),
    mNewText(),
    mOperations( NO_OPERATION ),
    mControlSize(),
    mTextInput( NULL )
  {
    mLogicalModel = LogicalModel::New();
    mVisualModel  = VisualModel::New();

    mView.SetVisualModel( mVisualModel );

    mFontClient = TextAbstraction::FontClient::Get();
  }

  ~Impl()
  {
    delete mTextInput;
  }

  ControlInterface& mControlInterface;

  std::string mNewText;

  LogicalModelPtr mLogicalModel;
  VisualModelPtr  mVisualModel;

  View mView;

  LayoutEngine mLayoutEngine;

  TextAbstraction::FontClient mFontClient;

  OperationsMask mOperations;

  Size mControlSize;

  // Avoid allocating everything for text input until EnableTextInput()
  Controller::TextInput* mTextInput;
};

ControllerPtr Controller::New( ControlInterface& controlInterface )
{
  return ControllerPtr( new Controller( controlInterface ) );
}

void Controller::SetText( const std::string& text )
{
  // Keep until size negotiation
  mImpl->mNewText = text;
  mImpl->mOperations = ALL_OPERATIONS;

  if( mImpl->mTextInput )
  {
    // Cancel previously queued events
    mImpl->mTextInput->mEventQueue.clear();

    // TODO - Hide selection decorations
  }
}

void Controller::GetText( std::string& text )
{
  if( !mImpl->mNewText.empty() )
  {
    text = mImpl->mNewText;
  }
  else
  {
    // TODO - Convert from UTF-32
  }
}

void Controller::EnableTextInput( DecoratorPtr decorator )
{
  if( !mImpl->mTextInput )
  {
    mImpl->mTextInput = new TextInput( mImpl->mLogicalModel, mImpl->mVisualModel, decorator );
  }
}

bool Controller::Relayout( const Vector2& size )
{
  if( ( size.width < Math::MACHINE_EPSILON_1000 ) || ( size.height < Math::MACHINE_EPSILON_1000 ) )
  {
    // Not worth to relayout if width or height is equal to zero.
    return false;
  }

  bool updated = false;

  if( size != mImpl->mControlSize )
  {
    updated = DoRelayout( size, mImpl->mOperations );

    // Do not re-do any operation until something changes.
    mImpl->mOperations = NO_OPERATION;

    mImpl->mControlSize = size;
  }

  if( mImpl->mTextInput )
  {
    // Move the cursor, grab handle etc.
    updated = mImpl->mTextInput->ProcessTouchEvents() || updated;
  }

  return updated;
}

bool Controller::DoRelayout( const Vector2& size, OperationsMask operations )
{
  bool viewUpdated( false );

  Vector<Character> utf32Characters;
  Length characterCount = 0u;
  if( CONVERT_TO_UTF32 & operations )
  {
    std::string& text = mImpl->mNewText;

    //  Convert text into UTF-32
    utf32Characters.Resize( text.size() );

    // This is a bit horrible but std::string returns a (signed) char*
    const uint8_t* utf8 = reinterpret_cast<const uint8_t*>( text.c_str() );

    // Transform a text array encoded in utf8 into an array encoded in utf32.
    // It returns the actual number of characters.
    characterCount = Utf8ToUtf32( utf8, text.size(), utf32Characters.Begin() );
    utf32Characters.Resize( characterCount );

    // Sets the text into the model.
    mImpl->mLogicalModel->SetText( utf32Characters.Begin(), characterCount );

    // Discard temporary text
    //text.clear(); temporary keep the text. will be fixed in the next patch.
  }

  Vector<LineBreakInfo> lineBreakInfo;
  if( GET_LINE_BREAKS & operations )
  {
    // Retrieves the line break info. The line break info is used to split the text in 'paragraphs' to
    // calculate the bidirectional info for each 'paragraph'.
    // It's also used to layout the text (where it should be a new line) or to shape the text (text in different lines
    // is not shaped together).
    lineBreakInfo.Resize( characterCount, TextAbstraction::LINE_NO_BREAK );

    SetLineBreakInfo( utf32Characters,
                      lineBreakInfo );

    mImpl->mLogicalModel->SetLineBreakInfo( lineBreakInfo.Begin(), characterCount );
  }

  Vector<WordBreakInfo> wordBreakInfo;
  if( GET_WORD_BREAKS & operations )
  {
    // Retrieves the word break info. The word break info is used to layout the text (where to wrap the text in lines).
    wordBreakInfo.Resize( characterCount, TextAbstraction::WORD_NO_BREAK );

    SetWordBreakInfo( utf32Characters,
                      wordBreakInfo );

    mImpl->mLogicalModel->SetWordBreakInfo( wordBreakInfo.Begin(), characterCount );
  }

  const bool getScripts = GET_SCRIPTS & operations;
  const bool validateFonts = VALIDATE_FONTS & operations;

  Vector<ScriptRun> scripts;
  Vector<FontRun> fonts;
  if( getScripts || validateFonts )
  {
    // Validates the fonts assigned by the application or assigns default ones.
    // It makes sure all the characters are going to be rendered by the correct font.
    MultilanguageSupport multilanguageSupport = MultilanguageSupport::Get();

    if( getScripts )
    {
      // Retrieves the scripts used in the text.
      multilanguageSupport.SetScripts( utf32Characters,
                                       lineBreakInfo,
                                       scripts );

      // Sets the scripts into the model.
      mImpl->mLogicalModel->SetScripts( scripts.Begin(), scripts.Count() );
    }

    if( validateFonts )
    {
      // Validates the fonts. If there is a character with no assigned font it sets a default one.
      // After this call, fonts are validated.
      multilanguageSupport.ValidateFonts( utf32Characters,
                                          scripts,
                                          fonts );

      // Sets the fonts into the model.
      mImpl->mLogicalModel->SetFonts( fonts.Begin(), fonts.Count() );
    }
  }

  Vector<GlyphInfo> glyphs;
  Vector<CharacterIndex> glyphsToCharactersMap;
  Vector<Length> charactersPerGlyph;
  if( SHAPE_TEXT & operations )
  {
    // Shapes the text.
    ShapeText( utf32Characters,
               lineBreakInfo,
               scripts,
               fonts,
               glyphs,
               glyphsToCharactersMap,
               charactersPerGlyph );
  }

  if( GET_GLYPH_METRICS & operations )
  {
    mImpl->mFontClient.GetGlyphMetrics( glyphs.Begin(), glyphs.Count() );
  }

  Length numberOfGlyphs = glyphs.Count();
  if( 0u != numberOfGlyphs )
  {
    // Sets the glyphs into the model.
    mImpl->mVisualModel->SetGlyphs( glyphs.Begin(),
                                    glyphsToCharactersMap.Begin(),
                                    charactersPerGlyph.Begin(),
                                    numberOfGlyphs );
  }

  if( LAYOUT & operations )
  {
    if( 0u == numberOfGlyphs )
    {
      const Length numberOfCharacters = mImpl->mLogicalModel->GetNumberOfCharacters();
      numberOfGlyphs = mImpl->mVisualModel->GetNumberOfGlyphs();

      lineBreakInfo.Resize( numberOfCharacters );
      wordBreakInfo.Resize( numberOfCharacters );
      glyphs.Resize( numberOfGlyphs );
      glyphsToCharactersMap.Resize( numberOfGlyphs );
      charactersPerGlyph.Resize( numberOfGlyphs );

      mImpl->mLogicalModel->GetLineBreakInfo( lineBreakInfo.Begin(),
                                              0u,
                                              numberOfCharacters );

      mImpl->mLogicalModel->GetWordBreakInfo( wordBreakInfo.Begin(),
                                              0u,
                                              numberOfCharacters );

      mImpl->mVisualModel->GetGlyphs( glyphs.Begin(),
                                      0u,
                                      numberOfGlyphs );

      mImpl->mVisualModel->GetGlyphToCharacterMap( glyphsToCharactersMap.Begin(),
                                                   0u,
                                                   numberOfGlyphs );

      mImpl->mVisualModel->GetCharactersPerGlyphMap( charactersPerGlyph.Begin(),
                                                     0u,
                                                     numberOfGlyphs );
    }

    // Set the layout parameters.
    LayoutParameters layoutParameters( size,
                                       lineBreakInfo.Begin(),
                                       wordBreakInfo.Begin(),
                                       numberOfGlyphs,
                                       glyphs.Begin(),
                                       glyphsToCharactersMap.Begin(),
                                       charactersPerGlyph.Begin() );

    // Reserve space to set the positions of the glyphs.
    Vector<Vector2> glyphPositions;
    glyphPositions.Resize( numberOfGlyphs );

    Size layoutSize;

    // Update the visual model
    viewUpdated = mImpl->mLayoutEngine.LayoutText( layoutParameters,
                                                   glyphPositions,
                                                   layoutSize );

    // Sets the positions into the model.
    mImpl->mVisualModel->SetGlyphPositions( glyphPositions.Begin(),
                                            numberOfGlyphs );

    // Sets the actual size.
    mImpl->mVisualModel->SetActualSize( layoutSize );
  }

  return viewUpdated;
}

Vector3 Controller::GetNaturalSize()
{
  // TODO - Finish implementing
  return Vector3::ZERO;

  // Operations that can be done only once until the text changes.
  const OperationsMask onlyOnceOperations = static_cast<OperationsMask>( CONVERT_TO_UTF32 |
                                                                         GET_SCRIPTS      |
                                                                         VALIDATE_FONTS   |
                                                                         GET_LINE_BREAKS  |
                                                                         GET_WORD_BREAKS  |
                                                                         SHAPE_TEXT       |
                                                                         GET_GLYPH_METRICS );

  // Operations that need to be done if the size or the text changes.
  const OperationsMask sizeOperations =  static_cast<OperationsMask>( LAYOUT |
                                                                      REORDER );

  const float maxFloat = std::numeric_limits<float>::max();
  DoRelayout( Vector2( maxFloat, maxFloat ),
              static_cast<OperationsMask>( onlyOnceOperations |
                                           sizeOperations ) );

  // Do not do again the only once operations.
  mImpl->mOperations = static_cast<OperationsMask>( mImpl->mOperations & ~onlyOnceOperations );

  // Do the size related operations again.
  mImpl->mOperations = static_cast<OperationsMask>( mImpl->mOperations | sizeOperations );

  return Vector3( mImpl->mVisualModel->GetNaturalSize() );
}

float Controller::GetHeightForWidth( float width )
{
  // Operations that can be done only once until the text changes.
  const OperationsMask onlyOnceOperations = static_cast<OperationsMask>( CONVERT_TO_UTF32 |
                                                                         GET_SCRIPTS      |
                                                                         VALIDATE_FONTS   |
                                                                         GET_LINE_BREAKS  |
                                                                         GET_WORD_BREAKS  |
                                                                         SHAPE_TEXT       |
                                                                         GET_GLYPH_METRICS );

  // Operations that need to be done if the size or the text changes.
  const OperationsMask sizeOperations =  static_cast<OperationsMask>( LAYOUT |
                                                                      REORDER );

  DoRelayout( Size( width, 0.f ),
              static_cast<OperationsMask>( onlyOnceOperations |
                                           sizeOperations ) );

  // Do not do again the only once operations.
  mImpl->mOperations = static_cast<OperationsMask>( mImpl->mOperations & ~onlyOnceOperations );

  // Do the size related operations again.
  mImpl->mOperations = static_cast<OperationsMask>( mImpl->mOperations | sizeOperations );

  return mImpl->mVisualModel->GetActualSize().height;
}

View& Controller::GetView()
{
  return mImpl->mView;
}

LayoutEngine& Controller::GetLayoutEngine()
{
  return mImpl->mLayoutEngine;
}

void Controller::RequestRelayout()
{
  mImpl->mControlInterface.RequestTextRelayout();
}

void Controller::KeyboardFocusGainEvent()
{
  DALI_ASSERT_DEBUG( mImpl->mTextInput && "Unexpected KeyboardFocusGainEvent" );

  if( mImpl->mTextInput )
  {
    TextInput::Event event( TextInput::KEYBOARD_FOCUS_GAIN_EVENT );
    mImpl->mTextInput->mEventQueue.push_back( event );

    RequestRelayout();
  }
}

void Controller::KeyboardFocusLostEvent()
{
  DALI_ASSERT_DEBUG( mImpl->mTextInput && "Unexpected KeyboardFocusLostEvent" );

  if( mImpl->mTextInput )
  {
    TextInput::Event event( TextInput::KEYBOARD_FOCUS_LOST_EVENT );
    mImpl->mTextInput->mEventQueue.push_back( event );

    RequestRelayout();
  }
}

void Controller::TapEvent( unsigned int tapCount, float x, float y )
{
  DALI_ASSERT_DEBUG( mImpl->mTextInput && "Unexpected TapEvent" );

  if( mImpl->mTextInput )
  {
    TextInput::Event event( TextInput::TAP_EVENT );
    event.p1.mUint = tapCount;
    event.p2.mFloat = x;
    event.p3.mFloat = y;
    mImpl->mTextInput->mEventQueue.push_back( event );

    RequestRelayout();
  }
}

void Controller::GrabHandleEvent( GrabHandleState state, float x, float y )
{
  DALI_ASSERT_DEBUG( mImpl->mTextInput && "Unexpected GrabHandleEvent" );

  if( mImpl->mTextInput )
  {
    TextInput::Event event( TextInput::GRAB_HANDLE_EVENT );
    event.p1.mUint  = state;
    event.p2.mFloat = x;
    event.p3.mFloat = y;
    mImpl->mTextInput->mEventQueue.push_back( event );

    RequestRelayout();
  }
}

Controller::~Controller()
{
  delete mImpl;
}

Controller::Controller( ControlInterface& controlInterface )
: mImpl( NULL )
{
  mImpl = new Controller::Impl( controlInterface );
}

} // namespace Text

} // namespace Toolkit

} // namespace Dali
