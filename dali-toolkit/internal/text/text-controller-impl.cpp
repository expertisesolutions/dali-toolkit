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
#include <dali-toolkit/internal/text/text-controller-impl.h>

// EXTERNAL INCLUDES
#include <dali/public-api/adaptor-framework/key.h>
#include <dali/integration-api/debug.h>
#include <limits>

// INTERNAL INCLUDES
#include <dali-toolkit/internal/text/bidirectional-support.h>
#include <dali-toolkit/internal/text/character-set-conversion.h>
#include <dali-toolkit/internal/text/color-segmentation.h>
#include <dali-toolkit/internal/text/cursor-helper-functions.h>
#include <dali-toolkit/internal/text/multi-language-support.h>
#include <dali-toolkit/internal/text/segmentation.h>
#include <dali-toolkit/internal/text/shaper.h>
#include <dali-toolkit/internal/text/text-run-container.h>

namespace
{

/**
 * @brief Struct used to calculate the selection box.
 */
struct SelectionBoxInfo
{
  float lineOffset;
  float lineHeight;
  float minX;
  float maxX;
};

#if defined(DEBUG_ENABLED)
  Debug::Filter* gLogFilter = Debug::Filter::New(Debug::NoLogging, true, "LOG_TEXT_CONTROLS");
#endif

const float MAX_FLOAT = std::numeric_limits<float>::max();
const float MIN_FLOAT = std::numeric_limits<float>::min();
const Dali::Toolkit::Text::CharacterDirection LTR = false; ///< Left To Right direction

} // namespace

namespace Dali
{

namespace Toolkit
{

namespace Text
{

EventData::EventData( DecoratorPtr decorator )
: mDecorator( decorator ),
  mImfManager(),
  mPlaceholderTextActive(),
  mPlaceholderTextInactive(),
  mPlaceholderTextColor( 0.8f, 0.8f, 0.8f, 0.8f ),
  mEventQueue(),
  mState( INACTIVE ),
  mPrimaryCursorPosition( 0u ),
  mLeftSelectionPosition( 0u ),
  mRightSelectionPosition( 0u ),
  mPreEditStartPosition( 0u ),
  mPreEditLength( 0u ),
  mCursorHookPositionX( 0.f ),
  mIsShowingPlaceholderText( false ),
  mPreEditFlag( false ),
  mDecoratorUpdated( false ),
  mCursorBlinkEnabled( true ),
  mGrabHandleEnabled( true ),
  mGrabHandlePopupEnabled( true ),
  mSelectionEnabled( true ),
  mUpdateCursorPosition( false ),
  mUpdateGrabHandlePosition( false ),
  mUpdateLeftSelectionPosition( false ),
  mUpdateRightSelectionPosition( false ),
  mUpdateHighlightBox( false ),
  mScrollAfterUpdatePosition( false ),
  mScrollAfterDelete( false ),
  mAllTextSelected( false ),
  mUpdateInputStyle( false )
{
  mImfManager = ImfManager::Get();
}

EventData::~EventData()
{}

bool Controller::Impl::ProcessInputEvents()
{
  DALI_LOG_INFO( gLogFilter, Debug::Verbose, "-->Controller::ProcessInputEvents\n" );
  if( NULL == mEventData )
  {
    // Nothing to do if there is no text input.
    DALI_LOG_INFO( gLogFilter, Debug::Verbose, "<--Controller::ProcessInputEvents no event data\n" );
    return false;
  }

  if( mEventData->mDecorator )
  {
    for( std::vector<Event>::iterator iter = mEventData->mEventQueue.begin();
         iter != mEventData->mEventQueue.end();
         ++iter )
    {
      switch( iter->type )
      {
        case Event::CURSOR_KEY_EVENT:
        {
          OnCursorKeyEvent( *iter );
          break;
        }
        case Event::TAP_EVENT:
        {
          OnTapEvent( *iter );
          break;
        }
        case Event::LONG_PRESS_EVENT:
        {
          OnLongPressEvent( *iter );
          break;
        }
        case Event::PAN_EVENT:
        {
          OnPanEvent( *iter );
          break;
        }
        case Event::GRAB_HANDLE_EVENT:
        case Event::LEFT_SELECTION_HANDLE_EVENT:
        case Event::RIGHT_SELECTION_HANDLE_EVENT: // Fall through
        {
          OnHandleEvent( *iter );
          break;
        }
        case Event::SELECT:
        {
          OnSelectEvent( *iter );
          break;
        }
        case Event::SELECT_ALL:
        {
          OnSelectAllEvent();
          break;
        }
      }
    }
  }

  if( mEventData->mUpdateCursorPosition ||
      mEventData->mUpdateHighlightBox )
  {
    NotifyImfManager();
  }

  // The cursor must also be repositioned after inserts into the model
  if( mEventData->mUpdateCursorPosition )
  {
    // Updates the cursor position and scrolls the text to make it visible.
    CursorInfo cursorInfo;
    // Calculate the cursor position from the new cursor index.
    GetCursorPosition( mEventData->mPrimaryCursorPosition,
                       cursorInfo );

    if( mEventData->mUpdateCursorHookPosition )
    {
      // Update the cursor hook position. Used to move the cursor with the keys 'up' and 'down'.
      mEventData->mCursorHookPositionX = cursorInfo.primaryPosition.x;
      mEventData->mUpdateCursorHookPosition = false;
    }

    // Scroll first the text after delete ...
    if( mEventData->mScrollAfterDelete )
    {
      ScrollTextToMatchCursor( cursorInfo );
    }

    // ... then, text can be scrolled to make the cursor visible.
    if( mEventData->mScrollAfterUpdatePosition )
    {
      const Vector2 currentCursorPosition( cursorInfo.primaryPosition.x, cursorInfo.lineOffset );
      ScrollToMakePositionVisible( currentCursorPosition, cursorInfo.lineHeight );
    }
    mEventData->mScrollAfterUpdatePosition = false;
    mEventData->mScrollAfterDelete = false;

    UpdateCursorPosition( cursorInfo );

    mEventData->mDecoratorUpdated = true;
    mEventData->mUpdateCursorPosition = false;
    mEventData->mUpdateGrabHandlePosition = false;
  }
  else
  {
    CursorInfo leftHandleInfo;
    CursorInfo rightHandleInfo;

    if( mEventData->mUpdateHighlightBox )
    {
      GetCursorPosition( mEventData->mLeftSelectionPosition,
                         leftHandleInfo );

      GetCursorPosition( mEventData->mRightSelectionPosition,
                         rightHandleInfo );

      if( mEventData->mScrollAfterUpdatePosition && mEventData->mUpdateLeftSelectionPosition )
      {
        const Vector2 currentCursorPosition( leftHandleInfo.primaryPosition.x, leftHandleInfo.lineOffset );
        ScrollToMakePositionVisible( currentCursorPosition, leftHandleInfo.lineHeight );
       }

      if( mEventData->mScrollAfterUpdatePosition && mEventData->mUpdateRightSelectionPosition )
      {
        const Vector2 currentCursorPosition( rightHandleInfo.primaryPosition.x, rightHandleInfo.lineOffset );
        ScrollToMakePositionVisible( currentCursorPosition, rightHandleInfo.lineHeight );
      }
    }

    if( mEventData->mUpdateLeftSelectionPosition )
    {
      UpdateSelectionHandle( LEFT_SELECTION_HANDLE,
                             leftHandleInfo );

      SetPopupButtons();
      mEventData->mDecoratorUpdated = true;
      mEventData->mUpdateLeftSelectionPosition = false;
    }

    if( mEventData->mUpdateRightSelectionPosition )
    {
      UpdateSelectionHandle( RIGHT_SELECTION_HANDLE,
                             rightHandleInfo );

      SetPopupButtons();
      mEventData->mDecoratorUpdated = true;
      mEventData->mUpdateRightSelectionPosition = false;
    }

    if( mEventData->mUpdateHighlightBox )
    {
      RepositionSelectionHandles();

      mEventData->mUpdateLeftSelectionPosition = false;
      mEventData->mUpdateRightSelectionPosition = false;
      mEventData->mUpdateHighlightBox = false;
    }

    mEventData->mScrollAfterUpdatePosition = false;
  }

  if( mEventData->mUpdateInputStyle )
  {
    // Set the default style first.
    RetrieveDefaultInputStyle( mEventData->mInputStyle );

    // Get the character index from the cursor index.
    const CharacterIndex styleIndex = ( mEventData->mPrimaryCursorPosition > 0u ) ? mEventData->mPrimaryCursorPosition - 1u : 0u;

    // Retrieve the style from the style runs stored in the logical model.
    mLogicalModel->RetrieveStyle( styleIndex, mEventData->mInputStyle );

    mEventData->mUpdateInputStyle = false;
  }

  mEventData->mEventQueue.clear();

  DALI_LOG_INFO( gLogFilter, Debug::Verbose, "<--Controller::ProcessInputEvents\n" );

  const bool decoratorUpdated = mEventData->mDecoratorUpdated;
  mEventData->mDecoratorUpdated = false;

  return decoratorUpdated;
}

void Controller::Impl::NotifyImfManager()
{
  if( mEventData && mEventData->mImfManager )
  {
    CharacterIndex cursorPosition = GetLogicalCursorPosition();

    const Length numberOfWhiteSpaces = GetNumberOfWhiteSpaces( 0u );

    // Update the cursor position by removing the initial white spaces.
    if( cursorPosition < numberOfWhiteSpaces )
    {
      cursorPosition = 0u;
    }
    else
    {
      cursorPosition -= numberOfWhiteSpaces;
    }

    mEventData->mImfManager.SetCursorPosition( cursorPosition );
    mEventData->mImfManager.NotifyCursorPosition();
  }
}

CharacterIndex Controller::Impl::GetLogicalCursorPosition() const
{
  CharacterIndex cursorPosition = 0u;

  if( mEventData )
  {
    if( ( EventData::SELECTING == mEventData->mState ) ||
        ( EventData::SELECTION_HANDLE_PANNING == mEventData->mState ) )
    {
      cursorPosition = std::min( mEventData->mRightSelectionPosition, mEventData->mLeftSelectionPosition );
    }
    else
    {
      cursorPosition = mEventData->mPrimaryCursorPosition;
    }
  }

  return cursorPosition;
}

Length Controller::Impl::GetNumberOfWhiteSpaces( CharacterIndex index ) const
{
  Length numberOfWhiteSpaces = 0u;

  // Get the buffer to the text.
  Character* utf32CharacterBuffer = mLogicalModel->mText.Begin();

  const Length totalNumberOfCharacters = mLogicalModel->mText.Count();
  for( ; index < totalNumberOfCharacters; ++index, ++numberOfWhiteSpaces )
  {
    if( !TextAbstraction::IsWhiteSpace( *( utf32CharacterBuffer + index ) ) )
    {
      break;
    }
  }

  return numberOfWhiteSpaces;
}

void Controller::Impl::GetText( CharacterIndex index, std::string& text ) const
{
  // Get the total number of characters.
  Length numberOfCharacters = mLogicalModel->mText.Count();

  // Retrieve the text.
  if( 0u != numberOfCharacters )
  {
    Utf32ToUtf8( mLogicalModel->mText.Begin() + index, numberOfCharacters - index, text );
  }
}

void Controller::Impl::CalculateTextUpdateIndices( Length& numberOfCharacters )
{
  mTextUpdateInfo.mParagraphCharacterIndex = 0u;
  mTextUpdateInfo.mStartGlyphIndex = 0u;
  mTextUpdateInfo.mStartLineIndex = 0u;
  numberOfCharacters = 0u;

  const Length numberOfParagraphs = mLogicalModel->mParagraphInfo.Count();
  if( 0u == numberOfParagraphs )
  {
    mTextUpdateInfo.mParagraphCharacterIndex = 0u;
    numberOfCharacters = 0u;

    mTextUpdateInfo.mRequestedNumberOfCharacters = mTextUpdateInfo.mNumberOfCharactersToAdd - mTextUpdateInfo.mNumberOfCharactersToRemove;

    // Nothing else to do if there are no paragraphs.
    return;
  }

  // Find the paragraphs to be updated.
  Vector<ParagraphRunIndex> paragraphsToBeUpdated;
  if( mTextUpdateInfo.mCharacterIndex >= mTextUpdateInfo.mPreviousNumberOfCharacters )
  {
    // Text is being added at the end of the current text.
    if( mTextUpdateInfo.mIsLastCharacterNewParagraph )
    {
      // Text is being added in a new paragraph after the last character of the text.
      mTextUpdateInfo.mParagraphCharacterIndex = mTextUpdateInfo.mPreviousNumberOfCharacters;
      numberOfCharacters = 0u;
      mTextUpdateInfo.mRequestedNumberOfCharacters = mTextUpdateInfo.mNumberOfCharactersToAdd - mTextUpdateInfo.mNumberOfCharactersToRemove;

      mTextUpdateInfo.mStartGlyphIndex = mVisualModel->mGlyphs.Count();
      mTextUpdateInfo.mStartLineIndex = mVisualModel->mLines.Count() - 1u;

      // Nothing else to do;
      return;
    }

    paragraphsToBeUpdated.PushBack( numberOfParagraphs - 1u );
  }
  else
  {
    Length numberOfCharactersToUpdate = 0u;
    if( mTextUpdateInfo.mFullRelayoutNeeded )
    {
      numberOfCharactersToUpdate = mTextUpdateInfo.mPreviousNumberOfCharacters;
    }
    else
    {
      numberOfCharactersToUpdate = ( mTextUpdateInfo.mNumberOfCharactersToRemove > 0u ) ? mTextUpdateInfo.mNumberOfCharactersToRemove : 1u;
    }
    mLogicalModel->FindParagraphs( mTextUpdateInfo.mCharacterIndex,
                                   numberOfCharactersToUpdate,
                                   paragraphsToBeUpdated );
  }

  if( 0u != paragraphsToBeUpdated.Count() )
  {
    const ParagraphRunIndex firstParagraphIndex = *( paragraphsToBeUpdated.Begin() );
    const ParagraphRun& firstParagraph = *( mLogicalModel->mParagraphInfo.Begin() + firstParagraphIndex );
    mTextUpdateInfo.mParagraphCharacterIndex = firstParagraph.characterRun.characterIndex;

    ParagraphRunIndex lastParagraphIndex = *( paragraphsToBeUpdated.End() - 1u );
    const ParagraphRun& lastParagraph = *( mLogicalModel->mParagraphInfo.Begin() + lastParagraphIndex );

    if( ( mTextUpdateInfo.mNumberOfCharactersToRemove > 0u ) &&                                            // Some character are removed.
        ( lastParagraphIndex < numberOfParagraphs - 1u ) &&                                                // There is a next paragraph.
        ( ( lastParagraph.characterRun.characterIndex + lastParagraph.characterRun.numberOfCharacters ) == // The last removed character is the new paragraph character.
          ( mTextUpdateInfo.mCharacterIndex + mTextUpdateInfo.mNumberOfCharactersToRemove ) ) )
    {
      // The new paragraph character of the last updated paragraph has been removed so is going to be merged with the next one.
      const ParagraphRun& lastParagraph = *( mLogicalModel->mParagraphInfo.Begin() + lastParagraphIndex + 1u );

      numberOfCharacters = lastParagraph.characterRun.characterIndex + lastParagraph.characterRun.numberOfCharacters - mTextUpdateInfo.mParagraphCharacterIndex;
    }
    else
    {
      numberOfCharacters = lastParagraph.characterRun.characterIndex + lastParagraph.characterRun.numberOfCharacters - mTextUpdateInfo.mParagraphCharacterIndex;
    }
  }

  mTextUpdateInfo.mRequestedNumberOfCharacters = numberOfCharacters + mTextUpdateInfo.mNumberOfCharactersToAdd - mTextUpdateInfo.mNumberOfCharactersToRemove;
  mTextUpdateInfo.mStartGlyphIndex = *( mVisualModel->mCharactersToGlyph.Begin() + mTextUpdateInfo.mParagraphCharacterIndex );
}

void Controller::Impl::ClearFullModelData( OperationsMask operations )
{
  if( NO_OPERATION != ( GET_LINE_BREAKS & operations ) )
  {
    mLogicalModel->mLineBreakInfo.Clear();
    mLogicalModel->mParagraphInfo.Clear();
  }

  if( NO_OPERATION != ( GET_WORD_BREAKS & operations ) )
  {
    mLogicalModel->mLineBreakInfo.Clear();
  }

  if( NO_OPERATION != ( GET_SCRIPTS & operations ) )
  {
    mLogicalModel->mScriptRuns.Clear();
  }

  if( NO_OPERATION != ( VALIDATE_FONTS & operations ) )
  {
    mLogicalModel->mFontRuns.Clear();
  }

  if( 0u != mLogicalModel->mBidirectionalParagraphInfo.Count() )
  {
    if( NO_OPERATION != ( BIDI_INFO & operations ) )
    {
      mLogicalModel->mBidirectionalParagraphInfo.Clear();
      mLogicalModel->mCharacterDirections.Clear();
    }

    if( NO_OPERATION != ( REORDER & operations ) )
    {
      // Free the allocated memory used to store the conversion table in the bidirectional line info run.
      for( Vector<BidirectionalLineInfoRun>::Iterator it = mLogicalModel->mBidirectionalLineInfo.Begin(),
             endIt = mLogicalModel->mBidirectionalLineInfo.End();
           it != endIt;
           ++it )
      {
        BidirectionalLineInfoRun& bidiLineInfo = *it;

        free( bidiLineInfo.visualToLogicalMap );
        bidiLineInfo.visualToLogicalMap = NULL;
      }
      mLogicalModel->mBidirectionalLineInfo.Clear();
    }
  }

  if( NO_OPERATION != ( SHAPE_TEXT & operations ) )
  {
    mVisualModel->mGlyphs.Clear();
    mVisualModel->mGlyphsToCharacters.Clear();
    mVisualModel->mCharactersToGlyph.Clear();
    mVisualModel->mCharactersPerGlyph.Clear();
    mVisualModel->mGlyphsPerCharacter.Clear();
    mVisualModel->mGlyphPositions.Clear();
  }

  if( NO_OPERATION != ( LAYOUT & operations ) )
  {
    mVisualModel->mLines.Clear();
  }

  if( NO_OPERATION != ( COLOR & operations ) )
  {
    mVisualModel->mColorIndices.Clear();
  }
}

void Controller::Impl::ClearCharacterModelData( CharacterIndex startIndex, CharacterIndex endIndex, OperationsMask operations )
{
  const CharacterIndex endIndexPlusOne = endIndex + 1u;

  if( NO_OPERATION != ( GET_LINE_BREAKS & operations ) )
  {
    // Clear the line break info.
    LineBreakInfo* lineBreakInfoBuffer = mLogicalModel->mLineBreakInfo.Begin();

    mLogicalModel->mLineBreakInfo.Erase( lineBreakInfoBuffer + startIndex,
                                         lineBreakInfoBuffer + endIndexPlusOne );

    // Clear the paragraphs.
    ClearCharacterRuns( startIndex,
                        endIndex,
                        mLogicalModel->mParagraphInfo );
  }

  if( NO_OPERATION != ( GET_WORD_BREAKS & operations ) )
  {
    // Clear the word break info.
    WordBreakInfo* wordBreakInfoBuffer = mLogicalModel->mWordBreakInfo.Begin();

    mLogicalModel->mWordBreakInfo.Erase( wordBreakInfoBuffer + startIndex,
                                         wordBreakInfoBuffer + endIndexPlusOne );
  }

  if( NO_OPERATION != ( GET_SCRIPTS & operations ) )
  {
    // Clear the scripts.
    ClearCharacterRuns( startIndex,
                        endIndex,
                        mLogicalModel->mScriptRuns );
  }

  if( NO_OPERATION != ( VALIDATE_FONTS & operations ) )
  {
    // Clear the fonts.
    ClearCharacterRuns( startIndex,
                        endIndex,
                        mLogicalModel->mFontRuns );
  }

  if( 0u != mLogicalModel->mBidirectionalParagraphInfo.Count() )
  {
    if( NO_OPERATION != ( BIDI_INFO & operations ) )
    {
      // Clear the bidirectional paragraph info.
      ClearCharacterRuns( startIndex,
                          endIndex,
                          mLogicalModel->mBidirectionalParagraphInfo );

      // Clear the character's directions.
      CharacterDirection* characterDirectionsBuffer = mLogicalModel->mCharacterDirections.Begin();

      mLogicalModel->mCharacterDirections.Erase( characterDirectionsBuffer + startIndex,
                                                 characterDirectionsBuffer + endIndexPlusOne );
    }

    if( NO_OPERATION != ( REORDER & operations ) )
    {
      uint32_t startRemoveIndex = mLogicalModel->mBidirectionalLineInfo.Count();
      uint32_t endRemoveIndex = startRemoveIndex;
      ClearCharacterRuns( startIndex,
                          endIndex,
                          mLogicalModel->mBidirectionalLineInfo,
                          startRemoveIndex,
                          endRemoveIndex );

      BidirectionalLineInfoRun* bidirectionalLineInfoBuffer = mLogicalModel->mBidirectionalLineInfo.Begin();

      // Free the allocated memory used to store the conversion table in the bidirectional line info run.
      for( Vector<BidirectionalLineInfoRun>::Iterator it = bidirectionalLineInfoBuffer + startRemoveIndex,
             endIt = bidirectionalLineInfoBuffer + endRemoveIndex;
           it != endIt;
           ++it )
      {
        BidirectionalLineInfoRun& bidiLineInfo = *it;

        free( bidiLineInfo.visualToLogicalMap );
        bidiLineInfo.visualToLogicalMap = NULL;
      }

      mLogicalModel->mBidirectionalLineInfo.Erase( bidirectionalLineInfoBuffer + startRemoveIndex,
                                                   bidirectionalLineInfoBuffer + endRemoveIndex );
    }
  }
}

void Controller::Impl::ClearGlyphModelData( CharacterIndex startIndex, CharacterIndex endIndex, OperationsMask operations )
{
  const CharacterIndex endIndexPlusOne = endIndex + 1u;
  const Length numberOfCharactersRemoved = endIndexPlusOne - startIndex;

  // Convert the character index to glyph index before deleting the character to glyph and the glyphs per character buffers.
  GlyphIndex* charactersToGlyphBuffer = mVisualModel->mCharactersToGlyph.Begin();
  Length* glyphsPerCharacterBuffer = mVisualModel->mGlyphsPerCharacter.Begin();

  const GlyphIndex endGlyphIndexPlusOne = *( charactersToGlyphBuffer + endIndex ) + *( glyphsPerCharacterBuffer + endIndex );
  const Length numberOfGlyphsRemoved = endGlyphIndexPlusOne - mTextUpdateInfo.mStartGlyphIndex;

  if( NO_OPERATION != ( SHAPE_TEXT & operations ) )
  {
    // Update the character to glyph indices.
    for( Vector<GlyphIndex>::Iterator it =  charactersToGlyphBuffer + endIndexPlusOne,
           endIt =  charactersToGlyphBuffer + mVisualModel->mCharactersToGlyph.Count();
         it != endIt;
         ++it )
    {
      CharacterIndex& index = *it;
      index -= numberOfGlyphsRemoved;
    }

    // Clear the character to glyph conversion table.
    mVisualModel->mCharactersToGlyph.Erase( charactersToGlyphBuffer + startIndex,
                                            charactersToGlyphBuffer + endIndexPlusOne );

    // Clear the glyphs per character table.
    mVisualModel->mGlyphsPerCharacter.Erase( glyphsPerCharacterBuffer + startIndex,
                                             glyphsPerCharacterBuffer + endIndexPlusOne );

    // Clear the glyphs buffer.
    GlyphInfo* glyphsBuffer = mVisualModel->mGlyphs.Begin();
    mVisualModel->mGlyphs.Erase( glyphsBuffer + mTextUpdateInfo.mStartGlyphIndex,
                                 glyphsBuffer + endGlyphIndexPlusOne );

    CharacterIndex* glyphsToCharactersBuffer = mVisualModel->mGlyphsToCharacters.Begin();

    // Update the glyph to character indices.
    for( Vector<CharacterIndex>::Iterator it = glyphsToCharactersBuffer + endGlyphIndexPlusOne,
           endIt = glyphsToCharactersBuffer + mVisualModel->mGlyphsToCharacters.Count();
         it != endIt;
         ++it )
    {
      CharacterIndex& index = *it;
      index -= numberOfCharactersRemoved;
    }

    // Clear the glyphs to characters buffer.
    mVisualModel->mGlyphsToCharacters.Erase( glyphsToCharactersBuffer + mTextUpdateInfo.mStartGlyphIndex,
                                             glyphsToCharactersBuffer  + endGlyphIndexPlusOne );

    // Clear the characters per glyph buffer.
    Length* charactersPerGlyphBuffer = mVisualModel->mCharactersPerGlyph.Begin();
    mVisualModel->mCharactersPerGlyph.Erase( charactersPerGlyphBuffer + mTextUpdateInfo.mStartGlyphIndex,
                                             charactersPerGlyphBuffer + endGlyphIndexPlusOne );

    // Clear the positions buffer.
    Vector2* positionsBuffer = mVisualModel->mGlyphPositions.Begin();
    mVisualModel->mGlyphPositions.Erase( positionsBuffer + mTextUpdateInfo.mStartGlyphIndex,
                                         positionsBuffer + endGlyphIndexPlusOne );
  }

  if( NO_OPERATION != ( LAYOUT & operations ) )
  {
    // Clear the lines.
    uint32_t startRemoveIndex = mVisualModel->mLines.Count();
    uint32_t endRemoveIndex = startRemoveIndex;
    ClearCharacterRuns( startIndex,
                        endIndex,
                        mVisualModel->mLines,
                        startRemoveIndex,
                        endRemoveIndex );

    // Will update the glyph runs.
    startRemoveIndex = mVisualModel->mLines.Count();
    endRemoveIndex = startRemoveIndex;
    ClearGlyphRuns( mTextUpdateInfo.mStartGlyphIndex,
                    endGlyphIndexPlusOne - 1u,
                    mVisualModel->mLines,
                    startRemoveIndex,
                    endRemoveIndex );

    // Set the line index from where to insert the new laid-out lines.
    mTextUpdateInfo.mStartLineIndex = startRemoveIndex;

    LineRun* linesBuffer = mVisualModel->mLines.Begin();
    mVisualModel->mLines.Erase( linesBuffer + startRemoveIndex,
                                linesBuffer + endRemoveIndex );
  }

  if( NO_OPERATION != ( COLOR & operations ) )
  {
    if( 0u != mVisualModel->mColorIndices.Count() )
    {
      ColorIndex* colorIndexBuffer = mVisualModel->mColorIndices.Begin();
      mVisualModel->mColorIndices.Erase( colorIndexBuffer + mTextUpdateInfo.mStartGlyphIndex,
                                         colorIndexBuffer + endGlyphIndexPlusOne );
    }
  }
}

void Controller::Impl::ClearModelData( CharacterIndex startIndex, CharacterIndex endIndex, OperationsMask operations )
{
  if( mTextUpdateInfo.mClearAll ||
      ( ( 0u == startIndex ) &&
        ( mTextUpdateInfo.mPreviousNumberOfCharacters == endIndex + 1u ) ) )
  {
    ClearFullModelData( operations );
  }
  else
  {
    // Clear the model data related with characters.
    ClearCharacterModelData( startIndex, endIndex, operations );

    // Clear the model data related with glyphs.
    ClearGlyphModelData( startIndex, endIndex, operations );
  }

  // The estimated number of lines. Used to avoid reallocations when layouting.
  mTextUpdateInfo.mEstimatedNumberOfLines = std::max( mVisualModel->mLines.Count(), mLogicalModel->mParagraphInfo.Count() );

  mVisualModel->ClearCaches();
}

bool Controller::Impl::UpdateModel( OperationsMask operationsRequired )
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "Controller::UpdateModel\n" );

  // Calculate the operations to be done.
  const OperationsMask operations = static_cast<OperationsMask>( mOperationsPending & operationsRequired );

  if( NO_OPERATION == operations )
  {
    // Nothing to do if no operations are pending and required.
    return false;
  }

  Vector<Character>& utf32Characters = mLogicalModel->mText;

  const Length numberOfCharacters = utf32Characters.Count();

  // Index to the first character of the first paragraph to be updated.
  CharacterIndex startIndex = 0u;
  // Number of characters of the paragraphs to be removed.
  Length paragraphCharacters = 0u;

  CalculateTextUpdateIndices( paragraphCharacters );
  startIndex = mTextUpdateInfo.mParagraphCharacterIndex;

  if( mTextUpdateInfo.mClearAll ||
      ( 0u != paragraphCharacters ) )
  {
    ClearModelData( startIndex, startIndex + ( ( paragraphCharacters > 0u ) ? paragraphCharacters - 1u : 0u ), operations );
  }

  mTextUpdateInfo.mClearAll = false;

  // Whether the model is updated.
  bool updated = false;

  Vector<LineBreakInfo>& lineBreakInfo = mLogicalModel->mLineBreakInfo;
  const Length requestedNumberOfCharacters = mTextUpdateInfo.mRequestedNumberOfCharacters;

  if( NO_OPERATION != ( GET_LINE_BREAKS & operations ) )
  {
    // Retrieves the line break info. The line break info is used to split the text in 'paragraphs' to
    // calculate the bidirectional info for each 'paragraph'.
    // It's also used to layout the text (where it should be a new line) or to shape the text (text in different lines
    // is not shaped together).
    lineBreakInfo.Resize( numberOfCharacters, TextAbstraction::LINE_NO_BREAK );

    SetLineBreakInfo( utf32Characters,
                      startIndex,
                      requestedNumberOfCharacters,
                      lineBreakInfo );

    // Create the paragraph info.
    mLogicalModel->CreateParagraphInfo( startIndex,
                                        requestedNumberOfCharacters );
    updated = true;
  }

  Vector<WordBreakInfo>& wordBreakInfo = mLogicalModel->mWordBreakInfo;
  if( NO_OPERATION != ( GET_WORD_BREAKS & operations ) )
  {
    // Retrieves the word break info. The word break info is used to layout the text (where to wrap the text in lines).
    wordBreakInfo.Resize( numberOfCharacters, TextAbstraction::WORD_NO_BREAK );

    SetWordBreakInfo( utf32Characters,
                      startIndex,
                      requestedNumberOfCharacters,
                      wordBreakInfo );
    updated = true;
  }

  const bool getScripts = NO_OPERATION != ( GET_SCRIPTS & operations );
  const bool validateFonts = NO_OPERATION != ( VALIDATE_FONTS & operations );

  Vector<ScriptRun>& scripts = mLogicalModel->mScriptRuns;
  Vector<FontRun>& validFonts = mLogicalModel->mFontRuns;

  if( getScripts || validateFonts )
  {
    // Validates the fonts assigned by the application or assigns default ones.
    // It makes sure all the characters are going to be rendered by the correct font.
    MultilanguageSupport multilanguageSupport = MultilanguageSupport::Get();

    if( getScripts )
    {
      // Retrieves the scripts used in the text.
      multilanguageSupport.SetScripts( utf32Characters,
                                       startIndex,
                                       requestedNumberOfCharacters,
                                       scripts );
    }

    if( validateFonts )
    {
      // Validate the fonts set through the mark-up string.
      Vector<FontDescriptionRun>& fontDescriptionRuns = mLogicalModel->mFontDescriptionRuns;

      // Get the default font id.
      const FontId defaultFontId = ( NULL == mFontDefaults ) ? 0u : mFontDefaults->GetFontId( mFontClient );

      // Validates the fonts. If there is a character with no assigned font it sets a default one.
      // After this call, fonts are validated.
      multilanguageSupport.ValidateFonts( utf32Characters,
                                          scripts,
                                          fontDescriptionRuns,
                                          defaultFontId,
                                          startIndex,
                                          requestedNumberOfCharacters,
                                          validFonts );
    }
    updated = true;
  }

  Vector<Character> mirroredUtf32Characters;
  bool textMirrored = false;
  const Length numberOfParagraphs = mLogicalModel->mParagraphInfo.Count();
  if( NO_OPERATION != ( BIDI_INFO & operations ) )
  {
    Vector<BidirectionalParagraphInfoRun>& bidirectionalInfo = mLogicalModel->mBidirectionalParagraphInfo;
    bidirectionalInfo.Reserve( numberOfParagraphs );

    // Calculates the bidirectional info for the whole paragraph if it contains right to left scripts.
    SetBidirectionalInfo( utf32Characters,
                          scripts,
                          lineBreakInfo,
                          startIndex,
                          requestedNumberOfCharacters,
                          bidirectionalInfo );

    if( 0u != bidirectionalInfo.Count() )
    {
      // Only set the character directions if there is right to left characters.
      Vector<CharacterDirection>& directions = mLogicalModel->mCharacterDirections;
      GetCharactersDirection( bidirectionalInfo,
                              numberOfCharacters,
                              startIndex,
                              requestedNumberOfCharacters,
                              directions );

      // This paragraph has right to left text. Some characters may need to be mirrored.
      // TODO: consider if the mirrored string can be stored as well.

      textMirrored = GetMirroredText( utf32Characters,
                                      directions,
                                      bidirectionalInfo,
                                      startIndex,
                                      requestedNumberOfCharacters,
                                      mirroredUtf32Characters );
    }
    else
    {
      // There is no right to left characters. Clear the directions vector.
      mLogicalModel->mCharacterDirections.Clear();
    }
    updated = true;
  }

  Vector<GlyphInfo>& glyphs = mVisualModel->mGlyphs;
  Vector<CharacterIndex>& glyphsToCharactersMap = mVisualModel->mGlyphsToCharacters;
  Vector<Length>& charactersPerGlyph = mVisualModel->mCharactersPerGlyph;
  Vector<GlyphIndex> newParagraphGlyphs;
  newParagraphGlyphs.Reserve( numberOfParagraphs );

  const Length currentNumberOfGlyphs = glyphs.Count();
  if( NO_OPERATION != ( SHAPE_TEXT & operations ) )
  {
    const Vector<Character>& textToShape = textMirrored ? mirroredUtf32Characters : utf32Characters;
    // Shapes the text.
    ShapeText( textToShape,
               lineBreakInfo,
               scripts,
               validFonts,
               startIndex,
               mTextUpdateInfo.mStartGlyphIndex,
               requestedNumberOfCharacters,
               glyphs,
               glyphsToCharactersMap,
               charactersPerGlyph,
               newParagraphGlyphs );

    // Create the 'number of glyphs' per character and the glyph to character conversion tables.
    mVisualModel->CreateGlyphsPerCharacterTable( startIndex, mTextUpdateInfo.mStartGlyphIndex, requestedNumberOfCharacters );
    mVisualModel->CreateCharacterToGlyphTable( startIndex, mTextUpdateInfo.mStartGlyphIndex, requestedNumberOfCharacters );
    updated = true;
  }

  const Length numberOfGlyphs = glyphs.Count() - currentNumberOfGlyphs;

  if( NO_OPERATION != ( GET_GLYPH_METRICS & operations ) )
  {
    GlyphInfo* glyphsBuffer = glyphs.Begin();
    mMetrics->GetGlyphMetrics( glyphsBuffer + mTextUpdateInfo.mStartGlyphIndex, numberOfGlyphs );

    // Update the width and advance of all new paragraph characters.
    for( Vector<GlyphIndex>::ConstIterator it = newParagraphGlyphs.Begin(), endIt = newParagraphGlyphs.End(); it != endIt; ++it )
    {
      const GlyphIndex index = *it;
      GlyphInfo& glyph = *( glyphsBuffer + index );

      glyph.xBearing = 0.f;
      glyph.width = 0.f;
      glyph.advance = 0.f;
    }
    updated = true;
  }

  if( NO_OPERATION != ( COLOR & operations ) )
  {
    // Set the color runs in glyphs.
    SetColorSegmentationInfo( mLogicalModel->mColorRuns,
                              mVisualModel->mCharactersToGlyph,
                              mVisualModel->mGlyphsPerCharacter,
                              startIndex,
                              mTextUpdateInfo.mStartGlyphIndex,
                              requestedNumberOfCharacters,
                              mVisualModel->mColors,
                              mVisualModel->mColorIndices );

    updated = true;
  }

  if( ( NULL != mEventData ) &&
      mEventData->mPreEditFlag &&
      ( 0u != mVisualModel->mCharactersToGlyph.Count() ) )
  {
    // Add the underline for the pre-edit text.
    const GlyphIndex* const charactersToGlyphBuffer = mVisualModel->mCharactersToGlyph.Begin();
    const Length* const glyphsPerCharacterBuffer = mVisualModel->mGlyphsPerCharacter.Begin();

    const GlyphIndex glyphStart = *( charactersToGlyphBuffer + mEventData->mPreEditStartPosition );
    const CharacterIndex lastPreEditCharacter = mEventData->mPreEditStartPosition + ( ( mEventData->mPreEditLength > 0u ) ? mEventData->mPreEditLength - 1u : 0u );
    const Length numberOfGlyphsLastCharacter = *( glyphsPerCharacterBuffer + lastPreEditCharacter );
    const GlyphIndex glyphEnd = *( charactersToGlyphBuffer + lastPreEditCharacter ) + ( numberOfGlyphsLastCharacter > 1u ? numberOfGlyphsLastCharacter - 1u : 0u );

    GlyphRun underlineRun;
    underlineRun.glyphIndex = glyphStart;
    underlineRun.numberOfGlyphs = 1u + glyphEnd - glyphStart;

    // TODO: At the moment the underline runs are only for pre-edit.
    mVisualModel->mUnderlineRuns.PushBack( underlineRun );
  }

  // The estimated number of lines. Used to avoid reallocations when layouting.
  mTextUpdateInfo.mEstimatedNumberOfLines = std::max( mVisualModel->mLines.Count(), mLogicalModel->mParagraphInfo.Count() );

  // Set the previous number of characters for the next time the text is updated.
  mTextUpdateInfo.mPreviousNumberOfCharacters = numberOfCharacters;

  return updated;
}

void Controller::Impl::RetrieveDefaultInputStyle( InputStyle& inputStyle )
{
  // Sets the default text's color.
  inputStyle.textColor = mTextColor;
  inputStyle.isDefaultColor = true;

  inputStyle.familyName.clear();
  inputStyle.weight = TextAbstraction::FontWeight::NORMAL;
  inputStyle.width = TextAbstraction::FontWidth::NORMAL;
  inputStyle.slant = TextAbstraction::FontSlant::NORMAL;
  inputStyle.size = 0.f;

  inputStyle.familyDefined = false;
  inputStyle.weightDefined = false;
  inputStyle.widthDefined = false;
  inputStyle.slantDefined = false;
  inputStyle.sizeDefined = false;

  // Sets the default font's family name, weight, width, slant and size.
  if( mFontDefaults )
  {
    if( mFontDefaults->familyDefined )
    {
      inputStyle.familyName = mFontDefaults->mFontDescription.family;
      inputStyle.familyDefined = true;
    }

    if( mFontDefaults->weightDefined )
    {
      inputStyle.weight = mFontDefaults->mFontDescription.weight;
      inputStyle.weightDefined = true;
    }

    if( mFontDefaults->widthDefined )
    {
      inputStyle.width = mFontDefaults->mFontDescription.width;
      inputStyle.widthDefined = true;
    }

    if( mFontDefaults->slantDefined )
    {
      inputStyle.slant = mFontDefaults->mFontDescription.slant;
      inputStyle.slantDefined = true;
    }

    if( mFontDefaults->sizeDefined )
    {
      inputStyle.size = mFontDefaults->mDefaultPointSize;
      inputStyle.sizeDefined = true;
    }
  }
}

float Controller::Impl::GetDefaultFontLineHeight()
{
  FontId defaultFontId = 0u;
  if( NULL == mFontDefaults )
  {
    TextAbstraction::FontDescription fontDescription;
    defaultFontId = mFontClient.GetFontId( fontDescription );
  }
  else
  {
    defaultFontId = mFontDefaults->GetFontId( mFontClient );
  }

  Text::FontMetrics fontMetrics;
  mMetrics->GetFontMetrics( defaultFontId, fontMetrics );

  return( fontMetrics.ascender - fontMetrics.descender );
}

void Controller::Impl::OnCursorKeyEvent( const Event& event )
{
  if( NULL == mEventData )
  {
    // Nothing to do if there is no text input.
    return;
  }

  int keyCode = event.p1.mInt;

  if( Dali::DALI_KEY_CURSOR_LEFT == keyCode )
  {
    if( mEventData->mPrimaryCursorPosition > 0u )
    {
      mEventData->mPrimaryCursorPosition = CalculateNewCursorIndex( mEventData->mPrimaryCursorPosition - 1u );
    }
  }
  else if( Dali::DALI_KEY_CURSOR_RIGHT == keyCode )
  {
    if( mLogicalModel->mText.Count() > mEventData->mPrimaryCursorPosition )
    {
      mEventData->mPrimaryCursorPosition = CalculateNewCursorIndex( mEventData->mPrimaryCursorPosition );
    }
  }
  else if( Dali::DALI_KEY_CURSOR_UP == keyCode )
  {
    // Get first the line index of the current cursor position index.
    CharacterIndex characterIndex = 0u;

    if( mEventData->mPrimaryCursorPosition > 0u )
    {
      characterIndex = mEventData->mPrimaryCursorPosition - 1u;
    }

    const LineIndex lineIndex = mVisualModel->GetLineOfCharacter( characterIndex );

    if( lineIndex > 0u )
    {
      // Retrieve the cursor position info.
      CursorInfo cursorInfo;
      GetCursorPosition( mEventData->mPrimaryCursorPosition,
                         cursorInfo );

      // Get the line above.
      const LineRun& line = *( mVisualModel->mLines.Begin() + ( lineIndex - 1u ) );

      // Get the next hit 'y' point.
      const float hitPointY = cursorInfo.lineOffset - 0.5f * ( line.ascender - line.descender );

      // Use the cursor hook position 'x' and the next hit 'y' position to calculate the new cursor index.
      mEventData->mPrimaryCursorPosition = Text::GetClosestCursorIndex( mVisualModel,
                                                                        mLogicalModel,
                                                                        mMetrics,
                                                                        mEventData->mCursorHookPositionX,
                                                                        hitPointY );
    }
  }
  else if(   Dali::DALI_KEY_CURSOR_DOWN == keyCode )
  {
    // Get first the line index of the current cursor position index.
    CharacterIndex characterIndex = 0u;

    if( mEventData->mPrimaryCursorPosition > 0u )
    {
      characterIndex = mEventData->mPrimaryCursorPosition - 1u;
    }

    const LineIndex lineIndex = mVisualModel->GetLineOfCharacter( characterIndex );

    if( lineIndex + 1u < mVisualModel->mLines.Count() )
    {
      // Retrieve the cursor position info.
      CursorInfo cursorInfo;
      GetCursorPosition( mEventData->mPrimaryCursorPosition,
                         cursorInfo );

      // Get the line below.
      const LineRun& line = *( mVisualModel->mLines.Begin() + lineIndex + 1u );

      // Get the next hit 'y' point.
      const float hitPointY = cursorInfo.lineOffset + cursorInfo.lineHeight + 0.5f * ( line.ascender - line.descender );

      // Use the cursor hook position 'x' and the next hit 'y' position to calculate the new cursor index.
      mEventData->mPrimaryCursorPosition = Text::GetClosestCursorIndex( mVisualModel,
                                                                        mLogicalModel,
                                                                        mMetrics,
                                                                        mEventData->mCursorHookPositionX,
                                                                        hitPointY );
    }
  }

  mEventData->mUpdateCursorPosition = true;
  mEventData->mUpdateInputStyle = true;
  mEventData->mScrollAfterUpdatePosition = true;
}

void Controller::Impl::OnTapEvent( const Event& event )
{
  if( NULL != mEventData )
  {
    const unsigned int tapCount = event.p1.mUint;

    if( 1u == tapCount )
    {
      if( IsShowingRealText() )
      {
        // Convert from control's coords to text's coords.
        const float xPosition = event.p2.mFloat - mScrollPosition.x;
        const float yPosition = event.p3.mFloat - mScrollPosition.y;

        // Keep the tap 'x' position. Used to move the cursor.
        mEventData->mCursorHookPositionX = xPosition;

        mEventData->mPrimaryCursorPosition = Text::GetClosestCursorIndex( mVisualModel,
                                                                          mLogicalModel,
                                                                          mMetrics,
                                                                          xPosition,
                                                                          yPosition );

        // When the cursor position is changing, delay cursor blinking
        mEventData->mDecorator->DelayCursorBlink();
      }
      else
      {
        mEventData->mPrimaryCursorPosition = 0u;
      }

      mEventData->mUpdateCursorPosition = true;
      mEventData->mUpdateGrabHandlePosition = true;
      mEventData->mScrollAfterUpdatePosition = true;
      mEventData->mUpdateInputStyle = true;

      // Notify the cursor position to the imf manager.
      if( mEventData->mImfManager )
      {
        mEventData->mImfManager.SetCursorPosition( mEventData->mPrimaryCursorPosition );
        mEventData->mImfManager.NotifyCursorPosition();
      }
    }
  }
}

void Controller::Impl::OnPanEvent( const Event& event )
{
  if( NULL == mEventData )
  {
    // Nothing to do if there is no text input.
    return;
  }

  int state = event.p1.mInt;

  if( ( Gesture::Started == state ) ||
      ( Gesture::Continuing == state ) )
  {
    if( mEventData->mDecorator )
    {
      const Vector2& layoutSize = mVisualModel->GetLayoutSize();
      const Vector2 currentScroll = mScrollPosition;

      if( mEventData->mDecorator->IsHorizontalScrollEnabled() )
      {
        const float displacementX = event.p2.mFloat;
        mScrollPosition.x += displacementX;

        ClampHorizontalScroll( layoutSize );
      }

      if( mEventData->mDecorator->IsVerticalScrollEnabled() )
      {
        const float displacementY = event.p3.mFloat;
        mScrollPosition.y += displacementY;

        ClampVerticalScroll( layoutSize );
      }

      mEventData->mDecorator->UpdatePositions( mScrollPosition - currentScroll );
    }
  }
}

void Controller::Impl::OnLongPressEvent( const Event& event )
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "Controller::OnLongPressEvent\n" );

  if( EventData::EDITING == mEventData->mState )
  {
    ChangeState ( EventData::EDITING_WITH_POPUP );
    mEventData->mDecoratorUpdated = true;
  }
}

void Controller::Impl::OnHandleEvent( const Event& event )
{
  if( NULL == mEventData )
  {
    // Nothing to do if there is no text input.
    return;
  }

  const unsigned int state = event.p1.mUint;
  const bool handleStopScrolling = ( HANDLE_STOP_SCROLLING == state );
  const bool isSmoothHandlePanEnabled = mEventData->mDecorator->IsSmoothHandlePanEnabled();

  if( HANDLE_PRESSED == state )
  {
    // Convert from decorator's coords to text's coords.
    const float xPosition = event.p2.mFloat - mScrollPosition.x;
    const float yPosition = event.p3.mFloat - mScrollPosition.y;

    // Need to calculate the handle's new position.
    const CharacterIndex handleNewPosition = Text::GetClosestCursorIndex( mVisualModel,
                                                                          mLogicalModel,
                                                                          mMetrics,
                                                                          xPosition,
                                                                          yPosition );

    if( Event::GRAB_HANDLE_EVENT == event.type )
    {
      ChangeState ( EventData::GRAB_HANDLE_PANNING );

      if( handleNewPosition != mEventData->mPrimaryCursorPosition )
      {
        // Updates the cursor position if the handle's new position is different than the current one.
        mEventData->mUpdateCursorPosition = true;
        // Does not update the grab handle position if the smooth panning is enabled. (The decorator does it smooth).
        mEventData->mUpdateGrabHandlePosition = !isSmoothHandlePanEnabled;
        mEventData->mPrimaryCursorPosition = handleNewPosition;
      }

      // Updates the decorator if the soft handle panning is enabled. It triggers a relayout in the decorator and the new position of the handle is set.
      mEventData->mDecoratorUpdated = isSmoothHandlePanEnabled;
    }
    else if( Event::LEFT_SELECTION_HANDLE_EVENT == event.type )
    {
      ChangeState ( EventData::SELECTION_HANDLE_PANNING );

      if( ( handleNewPosition != mEventData->mLeftSelectionPosition ) &&
          ( handleNewPosition != mEventData->mRightSelectionPosition ) )
      {
        // Updates the highlight box if the handle's new position is different than the current one.
        mEventData->mUpdateHighlightBox = true;
        // Does not update the selection handle position if the smooth panning is enabled. (The decorator does it smooth).
        mEventData->mUpdateLeftSelectionPosition = !isSmoothHandlePanEnabled;
        mEventData->mLeftSelectionPosition = handleNewPosition;
      }

      // Updates the decorator if the soft handle panning is enabled. It triggers a relayout in the decorator and the new position of the handle is set.
      mEventData->mDecoratorUpdated = isSmoothHandlePanEnabled;
    }
    else if( Event::RIGHT_SELECTION_HANDLE_EVENT == event.type )
    {
      ChangeState ( EventData::SELECTION_HANDLE_PANNING );

      if( ( handleNewPosition != mEventData->mRightSelectionPosition ) &&
          ( handleNewPosition != mEventData->mLeftSelectionPosition ) )
      {
        // Updates the highlight box if the handle's new position is different than the current one.
        mEventData->mUpdateHighlightBox = true;
        // Does not update the selection handle position if the smooth panning is enabled. (The decorator does it smooth).
        mEventData->mUpdateRightSelectionPosition = !isSmoothHandlePanEnabled;
        mEventData->mRightSelectionPosition = handleNewPosition;
      }

      // Updates the decorator if the soft handle panning is enabled. It triggers a relayout in the decorator and the new position of the handle is set.
      mEventData->mDecoratorUpdated = isSmoothHandlePanEnabled;
    }
  } // end ( HANDLE_PRESSED == state )
  else if( ( HANDLE_RELEASED == state ) ||
           handleStopScrolling )
  {
    CharacterIndex handlePosition = 0u;
    if( handleStopScrolling || isSmoothHandlePanEnabled )
    {
      // Convert from decorator's coords to text's coords.
      const float xPosition = event.p2.mFloat - mScrollPosition.x;
      const float yPosition = event.p3.mFloat - mScrollPosition.y;

      handlePosition = Text::GetClosestCursorIndex( mVisualModel,
                                                    mLogicalModel,
                                                    mMetrics,
                                                    xPosition,
                                                    yPosition );
    }

    if( Event::GRAB_HANDLE_EVENT == event.type )
    {
      mEventData->mUpdateCursorPosition = true;
      mEventData->mUpdateGrabHandlePosition = true;
      mEventData->mUpdateInputStyle = true;

      if( !IsClipboardEmpty() )
      {
        ChangeState( EventData::EDITING_WITH_PASTE_POPUP ); // Moving grabhandle will show Paste Popup
      }

      if( handleStopScrolling || isSmoothHandlePanEnabled )
      {
        mEventData->mScrollAfterUpdatePosition = true;
        mEventData->mPrimaryCursorPosition = handlePosition;
      }
    }
    else if( Event::LEFT_SELECTION_HANDLE_EVENT == event.type )
    {
      ChangeState( EventData::SELECTING );

      mEventData->mUpdateHighlightBox = true;
      mEventData->mUpdateLeftSelectionPosition = true;

      if( handleStopScrolling || isSmoothHandlePanEnabled )
      {
        mEventData->mScrollAfterUpdatePosition = true;

        if( ( handlePosition != mEventData->mRightSelectionPosition ) &&
            ( handlePosition != mEventData->mLeftSelectionPosition ) )
        {
          mEventData->mLeftSelectionPosition = handlePosition;
        }
      }
    }
    else if( Event::RIGHT_SELECTION_HANDLE_EVENT == event.type )
    {
      ChangeState( EventData::SELECTING );

      mEventData->mUpdateHighlightBox = true;
      mEventData->mUpdateRightSelectionPosition = true;

      if( handleStopScrolling || isSmoothHandlePanEnabled )
      {
        mEventData->mScrollAfterUpdatePosition = true;
        if( ( handlePosition != mEventData->mRightSelectionPosition ) &&
            ( handlePosition != mEventData->mLeftSelectionPosition ) )
        {
          mEventData->mRightSelectionPosition = handlePosition;
        }
      }
    }

    mEventData->mDecoratorUpdated = true;
  } // end ( ( HANDLE_RELEASED == state ) || ( HANDLE_STOP_SCROLLING == state ) )
  else if( HANDLE_SCROLLING == state )
  {
    const float xSpeed = event.p2.mFloat;
    const float ySpeed = event.p3.mFloat;
    const Vector2& layoutSize = mVisualModel->GetLayoutSize();
    const Vector2 currentScrollPosition = mScrollPosition;

    mScrollPosition.x += xSpeed;
    mScrollPosition.y += ySpeed;

    ClampHorizontalScroll( layoutSize );
    ClampVerticalScroll( layoutSize );

    bool endOfScroll = false;
    if( Vector2::ZERO == ( currentScrollPosition - mScrollPosition ) )
    {
      // Notify the decorator there is no more text to scroll.
      // The decorator won't send more scroll events.
      mEventData->mDecorator->NotifyEndOfScroll();
      // Still need to set the position of the handle.
      endOfScroll = true;
    }

    // Set the position of the handle.
    const bool scrollRightDirection = xSpeed > 0.f;
    const bool scrollBottomDirection = ySpeed > 0.f;
    const bool leftSelectionHandleEvent = Event::LEFT_SELECTION_HANDLE_EVENT == event.type;
    const bool rightSelectionHandleEvent = Event::RIGHT_SELECTION_HANDLE_EVENT == event.type;

    if( Event::GRAB_HANDLE_EVENT == event.type )
    {
      ChangeState( EventData::GRAB_HANDLE_PANNING );

      // Get the grab handle position in decorator coords.
      Vector2 position = mEventData->mDecorator->GetPosition( GRAB_HANDLE );

      if( mEventData->mDecorator->IsHorizontalScrollEnabled() )
      {
        // Position the grag handle close to either the left or right edge.
        position.x = scrollRightDirection ? 0.f : mVisualModel->mControlSize.width;
      }

      if( mEventData->mDecorator->IsVerticalScrollEnabled() )
      {
        position.x = mEventData->mCursorHookPositionX;

        // Position the grag handle close to either the top or bottom edge.
        position.y = scrollBottomDirection ? 0.f : mVisualModel->mControlSize.height;
      }

      // Get the new handle position.
      // The grab handle's position is in decorator's coords. Need to transforms to text's coords.
      const CharacterIndex handlePosition = Text::GetClosestCursorIndex( mVisualModel,
                                                                         mLogicalModel,
                                                                         mMetrics,
                                                                         position.x - mScrollPosition.x,
                                                                         position.y - mScrollPosition.y );

      if( mEventData->mPrimaryCursorPosition != handlePosition )
      {
        mEventData->mUpdateCursorPosition = true;
        mEventData->mUpdateGrabHandlePosition = !isSmoothHandlePanEnabled;
        mEventData->mScrollAfterUpdatePosition = true;
        mEventData->mPrimaryCursorPosition = handlePosition;
      }
      mEventData->mUpdateInputStyle = mEventData->mUpdateCursorPosition;

      // Updates the decorator if the soft handle panning is enabled.
      mEventData->mDecoratorUpdated = isSmoothHandlePanEnabled;
    }
    else if( leftSelectionHandleEvent || rightSelectionHandleEvent )
    {
      ChangeState( EventData::SELECTION_HANDLE_PANNING );

      // Get the selection handle position in decorator coords.
      Vector2 position = mEventData->mDecorator->GetPosition( leftSelectionHandleEvent ? Text::LEFT_SELECTION_HANDLE : Text::RIGHT_SELECTION_HANDLE );

      if( mEventData->mDecorator->IsHorizontalScrollEnabled() )
      {
        // Position the selection handle close to either the left or right edge.
        position.x = scrollRightDirection ? 0.f : mVisualModel->mControlSize.width;
      }

      if( mEventData->mDecorator->IsVerticalScrollEnabled() )
      {
        position.x = mEventData->mCursorHookPositionX;

        // Position the grag handle close to either the top or bottom edge.
        position.y = scrollBottomDirection ? 0.f : mVisualModel->mControlSize.height;
      }

      // Get the new handle position.
      // The selection handle's position is in decorator's coords. Need to transform to text's coords.
      const CharacterIndex handlePosition = Text::GetClosestCursorIndex( mVisualModel,
                                                                         mLogicalModel,
                                                                         mMetrics,
                                                                         position.x - mScrollPosition.x,
                                                                         position.y - mScrollPosition.y );

      if( leftSelectionHandleEvent )
      {
        const bool differentHandles = ( mEventData->mLeftSelectionPosition != handlePosition ) && ( mEventData->mRightSelectionPosition != handlePosition );

        if( differentHandles || endOfScroll )
        {
          mEventData->mUpdateHighlightBox = true;
          mEventData->mUpdateLeftSelectionPosition = !isSmoothHandlePanEnabled;
          mEventData->mUpdateRightSelectionPosition = isSmoothHandlePanEnabled;
          mEventData->mLeftSelectionPosition = handlePosition;
        }
      }
      else
      {
        const bool differentHandles = ( mEventData->mRightSelectionPosition != handlePosition ) && ( mEventData->mLeftSelectionPosition != handlePosition );
        if( differentHandles || endOfScroll )
        {
          mEventData->mUpdateHighlightBox = true;
          mEventData->mUpdateRightSelectionPosition = !isSmoothHandlePanEnabled;
          mEventData->mUpdateLeftSelectionPosition = isSmoothHandlePanEnabled;
          mEventData->mRightSelectionPosition = handlePosition;
        }
      }

      if( mEventData->mUpdateLeftSelectionPosition || mEventData->mUpdateRightSelectionPosition )
      {
        RepositionSelectionHandles();

        mEventData->mScrollAfterUpdatePosition = !isSmoothHandlePanEnabled;
      }
    }
    mEventData->mDecoratorUpdated = true;
  } // end ( HANDLE_SCROLLING == state )
}

void Controller::Impl::OnSelectEvent( const Event& event )
{
  if( NULL == mEventData )
  {
    // Nothing to do if there is no text.
    return;
  }

  if( mEventData->mSelectionEnabled )
  {
    // Convert from control's coords to text's coords.
    const float xPosition = event.p2.mFloat - mScrollPosition.x;
    const float yPosition = event.p3.mFloat - mScrollPosition.y;

    // Calculates the logical position from the x,y coords.
    RepositionSelectionHandles( xPosition,
                                yPosition );
  }
}

void Controller::Impl::OnSelectAllEvent()
{
  DALI_LOG_INFO( gLogFilter, Debug::Verbose, "OnSelectAllEvent mEventData->mSelectionEnabled%s \n", mEventData->mSelectionEnabled?"true":"false");

  if( NULL == mEventData )
  {
    // Nothing to do if there is no text.
    return;
  }

  if( mEventData->mSelectionEnabled )
  {
    ChangeState( EventData::SELECTING );

    mEventData->mLeftSelectionPosition = 0u;
    mEventData->mRightSelectionPosition = mLogicalModel->mText.Count();

    mEventData->mScrollAfterUpdatePosition = true;
    mEventData->mUpdateLeftSelectionPosition = true;
    mEventData->mUpdateRightSelectionPosition = true;
    mEventData->mUpdateHighlightBox = true;
  }
}

void Controller::Impl::RetrieveSelection( std::string& selectedText, bool deleteAfterRetrieval )
{
  if( mEventData->mLeftSelectionPosition == mEventData->mRightSelectionPosition )
  {
    // Nothing to select if handles are in the same place.
    selectedText.clear();
    return;
  }

  const bool handlesCrossed = mEventData->mLeftSelectionPosition > mEventData->mRightSelectionPosition;

  //Get start and end position of selection
  const CharacterIndex startOfSelectedText = handlesCrossed ? mEventData->mRightSelectionPosition : mEventData->mLeftSelectionPosition;
  const Length lengthOfSelectedText = ( handlesCrossed ? mEventData->mLeftSelectionPosition : mEventData->mRightSelectionPosition ) - startOfSelectedText;

  Vector<Character>& utf32Characters = mLogicalModel->mText;
  const Length numberOfCharacters = utf32Characters.Count();

  // Validate the start and end selection points
  if( ( startOfSelectedText + lengthOfSelectedText ) <= numberOfCharacters )
  {
    //Get text as a UTF8 string
    Utf32ToUtf8( &utf32Characters[startOfSelectedText], lengthOfSelectedText, selectedText );

    if( deleteAfterRetrieval ) // Only delete text if copied successfully
    {
      // Set as input style the style of the first deleted character.
      mLogicalModel->RetrieveStyle( startOfSelectedText, mEventData->mInputStyle );

      mLogicalModel->UpdateTextStyleRuns( startOfSelectedText, -static_cast<int>( lengthOfSelectedText ) );

      // Mark the paragraphs to be updated.
      mTextUpdateInfo.mCharacterIndex = startOfSelectedText;
      mTextUpdateInfo.mNumberOfCharactersToRemove = lengthOfSelectedText;

      // Delete text between handles
      Vector<Character>::Iterator first = utf32Characters.Begin() + startOfSelectedText;
      Vector<Character>::Iterator last  = first + lengthOfSelectedText;
      utf32Characters.Erase( first, last );

      // Will show the cursor at the first character of the selection.
      mEventData->mPrimaryCursorPosition = handlesCrossed ? mEventData->mRightSelectionPosition : mEventData->mLeftSelectionPosition;
    }
    else
    {
      // Will show the cursor at the last character of the selection.
      mEventData->mPrimaryCursorPosition = handlesCrossed ? mEventData->mLeftSelectionPosition : mEventData->mRightSelectionPosition;
    }

    mEventData->mDecoratorUpdated = true;
  }
}

void Controller::Impl::ShowClipboard()
{
  if( mClipboard )
  {
    mClipboard.ShowClipboard();
  }
}

void Controller::Impl::HideClipboard()
{
  if( mClipboard && mClipboardHideEnabled )
  {
    mClipboard.HideClipboard();
  }
}

void Controller::Impl::SetClipboardHideEnable(bool enable)
{
  mClipboardHideEnabled = enable;
}

bool Controller::Impl::CopyStringToClipboard( std::string& source )
{
  //Send string to clipboard
  return ( mClipboard && mClipboard.SetItem( source ) );
}

void Controller::Impl::SendSelectionToClipboard( bool deleteAfterSending )
{
  std::string selectedText;
  RetrieveSelection( selectedText, deleteAfterSending );
  CopyStringToClipboard( selectedText );
  ChangeState( EventData::EDITING );
}

void Controller::Impl::GetTextFromClipboard( unsigned int itemIndex, std::string& retrievedString )
{
  if ( mClipboard )
  {
    retrievedString =  mClipboard.GetItem( itemIndex );
  }
}

void Controller::Impl::RepositionSelectionHandles()
{
  CharacterIndex selectionStart = mEventData->mLeftSelectionPosition;
  CharacterIndex selectionEnd = mEventData->mRightSelectionPosition;

  if( selectionStart == selectionEnd )
  {
    // Nothing to select if handles are in the same place.
    return;
  }

  mEventData->mDecorator->ClearHighlights();

  const GlyphIndex* const charactersToGlyphBuffer = mVisualModel->mCharactersToGlyph.Begin();
  const Length* const glyphsPerCharacterBuffer = mVisualModel->mGlyphsPerCharacter.Begin();
  const GlyphInfo* const glyphsBuffer = mVisualModel->mGlyphs.Begin();
  const Vector2* const positionsBuffer = mVisualModel->mGlyphPositions.Begin();
  const Length* const charactersPerGlyphBuffer = mVisualModel->mCharactersPerGlyph.Begin();
  const CharacterIndex* const glyphToCharacterBuffer = mVisualModel->mGlyphsToCharacters.Begin();
  const CharacterDirection* const modelCharacterDirectionsBuffer = ( 0u != mLogicalModel->mCharacterDirections.Count() ) ? mLogicalModel->mCharacterDirections.Begin() : NULL;

  const bool isLastCharacter = selectionEnd >= mLogicalModel->mText.Count();
  const CharacterDirection startDirection = ( ( NULL == modelCharacterDirectionsBuffer ) ? false : *( modelCharacterDirectionsBuffer + selectionStart ) );
  const CharacterDirection endDirection = ( ( NULL == modelCharacterDirectionsBuffer ) ? false : *( modelCharacterDirectionsBuffer + ( selectionEnd - ( isLastCharacter ? 1u : 0u ) ) ) );

  // Swap the indices if the start is greater than the end.
  const bool indicesSwapped = selectionStart > selectionEnd;

  // Tell the decorator to flip the selection handles if needed.
  mEventData->mDecorator->SetSelectionHandleFlipState( indicesSwapped, startDirection, endDirection );

  if( indicesSwapped )
  {
    std::swap( selectionStart, selectionEnd );
  }

  // Get the indices to the first and last selected glyphs.
  const CharacterIndex selectionEndMinusOne = selectionEnd - 1u;
  const GlyphIndex glyphStart = *( charactersToGlyphBuffer + selectionStart );
  const Length numberOfGlyphs = *( glyphsPerCharacterBuffer + selectionEndMinusOne );
  const GlyphIndex glyphEnd = *( charactersToGlyphBuffer + selectionEndMinusOne ) + ( ( numberOfGlyphs > 0 ) ? numberOfGlyphs - 1u : 0u );

  // Get the lines where the glyphs are laid-out.
  const LineRun* lineRun = mVisualModel->mLines.Begin();

  LineIndex lineIndex = 0u;
  Length numberOfLines = 0u;
  mVisualModel->GetNumberOfLines( glyphStart,
                                  1u + glyphEnd - glyphStart,
                                  lineIndex,
                                  numberOfLines );
  const LineIndex firstLineIndex = lineIndex;

  // Create the structure to store some selection box info.
  Vector<SelectionBoxInfo> selectionBoxLinesInfo;
  selectionBoxLinesInfo.Resize( numberOfLines );

  SelectionBoxInfo* selectionBoxInfo = selectionBoxLinesInfo.Begin();
  selectionBoxInfo->minX = MAX_FLOAT;
  selectionBoxInfo->maxX = MIN_FLOAT;

  // Keep the min and max 'x' position to calculate the size and position of the highlighed text.
  float minHighlightX = std::numeric_limits<float>::max();
  float maxHighlightX = std::numeric_limits<float>::min();
  Size highLightSize;
  Vector2 highLightPosition; // The highlight position in decorator's coords.

  // Retrieve the first line and get the line's vertical offset, the line's height and the index to the last glyph.

  // The line's vertical offset of all the lines before the line where the first glyph is laid-out.
  selectionBoxInfo->lineOffset = CalculateLineOffset( mVisualModel->mLines,
                                                      firstLineIndex );

  // Transform to decorator's (control) coords.
  selectionBoxInfo->lineOffset += mScrollPosition.y;

  lineRun += firstLineIndex;

  // The line height is the addition of the line ascender and the line descender.
  // However, the line descender has a negative value, hence the subtraction.
  selectionBoxInfo->lineHeight = lineRun->ascender - lineRun->descender;

  GlyphIndex lastGlyphOfLine = lineRun->glyphRun.glyphIndex + lineRun->glyphRun.numberOfGlyphs - 1u;

  // Check if the first glyph is a ligature that must be broken like Latin ff, fi, or Arabic ﻻ, etc which needs special code.
  const Length numberOfCharactersStart = *( charactersPerGlyphBuffer + glyphStart );
  bool splitStartGlyph = ( numberOfCharactersStart > 1u ) && HasLigatureMustBreak( mLogicalModel->GetScript( selectionStart ) );

  // Check if the last glyph is a ligature that must be broken like Latin ff, fi, or Arabic ﻻ, etc which needs special code.
  const Length numberOfCharactersEnd = *( charactersPerGlyphBuffer + glyphEnd );
  bool splitEndGlyph = ( glyphStart != glyphEnd ) && ( numberOfCharactersEnd > 1u ) && HasLigatureMustBreak( mLogicalModel->GetScript( selectionEndMinusOne ) );

  // Traverse the glyphs.
  for( GlyphIndex index = glyphStart; index <= glyphEnd; ++index )
  {
    const GlyphInfo& glyph = *( glyphsBuffer + index );
    const Vector2& position = *( positionsBuffer + index );

    if( splitStartGlyph )
    {
      // If the first glyph is a ligature that must be broken it may be needed to add only part of the glyph to the highlight box.

      const float glyphAdvance = glyph.advance / static_cast<float>( numberOfCharactersStart );
      const CharacterIndex interGlyphIndex = selectionStart - *( glyphToCharacterBuffer + glyphStart );
      // Get the direction of the character.
      CharacterDirection isCurrentRightToLeft = false;
      if( NULL != modelCharacterDirectionsBuffer ) // If modelCharacterDirectionsBuffer is NULL, it means the whole text is left to right.
      {
        isCurrentRightToLeft = *( modelCharacterDirectionsBuffer + selectionStart );
      }

      // The end point could be in the middle of the ligature.
      // Calculate the number of characters selected.
      const Length numberOfCharacters = ( glyphStart == glyphEnd ) ? ( selectionEnd - selectionStart ) : ( numberOfCharactersStart - interGlyphIndex );

      const float xPosition = lineRun->alignmentOffset + position.x - glyph.xBearing + mScrollPosition.x + glyphAdvance * static_cast<float>( isCurrentRightToLeft ? ( numberOfCharactersStart - interGlyphIndex - numberOfCharacters ) : interGlyphIndex );
      const float xPositionAdvance = xPosition + static_cast<float>( numberOfCharacters ) * glyphAdvance;
      const float yPosition = selectionBoxInfo->lineOffset;

      // Store the min and max 'x' for each line.
      selectionBoxInfo->minX = std::min( selectionBoxInfo->minX, xPosition );
      selectionBoxInfo->maxX = std::max( selectionBoxInfo->maxX, xPositionAdvance );

      mEventData->mDecorator->AddHighlight( xPosition,
                                            yPosition,
                                            xPositionAdvance,
                                            yPosition + selectionBoxInfo->lineHeight );

      splitStartGlyph = false;
      continue;
    }

    if( splitEndGlyph && ( index == glyphEnd ) )
    {
      // Equally, if the last glyph is a ligature that must be broken it may be needed to add only part of the glyph to the highlight box.

      const float glyphAdvance = glyph.advance / static_cast<float>( numberOfCharactersEnd );
      const CharacterIndex interGlyphIndex = selectionEnd - *( glyphToCharacterBuffer + glyphEnd );
      // Get the direction of the character.
      CharacterDirection isCurrentRightToLeft = false;
      if( NULL != modelCharacterDirectionsBuffer ) // If modelCharacterDirectionsBuffer is NULL, it means the whole text is left to right.
      {
        isCurrentRightToLeft = *( modelCharacterDirectionsBuffer + selectionEnd );
      }

      const Length numberOfCharacters = numberOfCharactersEnd - interGlyphIndex;

      const float xPosition = lineRun->alignmentOffset + position.x - glyph.xBearing + mScrollPosition.x + ( isCurrentRightToLeft ? ( glyphAdvance * static_cast<float>( numberOfCharacters ) ) : 0.f );
      const float xPositionAdvance = xPosition + static_cast<float>( interGlyphIndex ) * glyphAdvance;
      const float yPosition = selectionBoxInfo->lineOffset;

      // Store the min and max 'x' for each line.
      selectionBoxInfo->minX = std::min( selectionBoxInfo->minX, xPosition );
      selectionBoxInfo->maxX = std::max( selectionBoxInfo->maxX, xPositionAdvance );

      mEventData->mDecorator->AddHighlight( xPosition,
                                            yPosition,
                                            xPositionAdvance,
                                            yPosition + selectionBoxInfo->lineHeight );

      splitEndGlyph = false;
      continue;
    }

    const float xPosition = lineRun->alignmentOffset + position.x - glyph.xBearing + mScrollPosition.x;
    const float xPositionAdvance = xPosition + glyph.advance;
    const float yPosition = selectionBoxInfo->lineOffset;

    // Store the min and max 'x' for each line.
    selectionBoxInfo->minX = std::min( selectionBoxInfo->minX, xPosition );
    selectionBoxInfo->maxX = std::max( selectionBoxInfo->maxX, xPositionAdvance );

    mEventData->mDecorator->AddHighlight( xPosition,
                                          yPosition,
                                          xPositionAdvance,
                                          yPosition + selectionBoxInfo->lineHeight );

    // Whether to retrieve the next line.
    if( index == lastGlyphOfLine )
    {
      // Retrieve the next line.
      ++lineRun;

      // Get the last glyph of the new line.
      lastGlyphOfLine = lineRun->glyphRun.glyphIndex + lineRun->glyphRun.numberOfGlyphs - 1u;

      ++lineIndex;
      if( lineIndex < firstLineIndex + numberOfLines )
      {
        // Keep the offset and height of the current selection box.
        const float currentLineOffset = selectionBoxInfo->lineOffset;
        const float currentLineHeight = selectionBoxInfo->lineHeight;

        // Get the selection box info for the next line.
        ++selectionBoxInfo;

        selectionBoxInfo->minX = MAX_FLOAT;
        selectionBoxInfo->maxX = MIN_FLOAT;

        // Update the line's vertical offset.
        selectionBoxInfo->lineOffset = currentLineOffset + currentLineHeight;

        // The line height is the addition of the line ascender and the line descender.
        // However, the line descender has a negative value, hence the subtraction.
        selectionBoxInfo->lineHeight = lineRun->ascender - lineRun->descender;
      }
    }
  }

  // Traverses all the lines and updates the min and max 'x' positions and the total height.
  // The final width is calculated after 'boxifying' the selection.
  for( Vector<SelectionBoxInfo>::ConstIterator it = selectionBoxLinesInfo.Begin(),
         endIt = selectionBoxLinesInfo.End();
       it != endIt;
       ++it )
  {
    const SelectionBoxInfo& info = *it;

    // Update the size of the highlighted text.
    highLightSize.height += selectionBoxInfo->lineHeight;
    minHighlightX = std::min( minHighlightX, info.minX );
    maxHighlightX = std::max( maxHighlightX, info.maxX );
  }

  // Add extra geometry to 'boxify' the selection.

  if( 1u < numberOfLines )
  {
    // Boxify the first line.
    lineRun = mVisualModel->mLines.Begin() + firstLineIndex;
    const SelectionBoxInfo& firstSelectionBoxLineInfo = *( selectionBoxLinesInfo.Begin() );

    bool boxifyBegin = ( LTR != lineRun->direction ) && ( LTR != startDirection );
    bool boxifyEnd = ( LTR == lineRun->direction ) && ( LTR == startDirection );

    if( boxifyBegin )
    {
      // Boxify at the beginning of the line.
      mEventData->mDecorator->AddHighlight( 0.f,
                                            firstSelectionBoxLineInfo.lineOffset,
                                            firstSelectionBoxLineInfo.minX,
                                            firstSelectionBoxLineInfo.lineOffset + firstSelectionBoxLineInfo.lineHeight );

      // Update the size of the highlighted text.
      minHighlightX = 0.f;
    }

    if( boxifyEnd )
    {
      // Boxify at the end of the line.
      mEventData->mDecorator->AddHighlight( firstSelectionBoxLineInfo.maxX,
                                            firstSelectionBoxLineInfo.lineOffset,
                                            mVisualModel->mControlSize.width,
                                            firstSelectionBoxLineInfo.lineOffset + firstSelectionBoxLineInfo.lineHeight );

      // Update the size of the highlighted text.
      maxHighlightX = mVisualModel->mControlSize.width;
    }

    // Boxify the central lines.
    if( 2u < numberOfLines )
    {
      for( Vector<SelectionBoxInfo>::ConstIterator it = selectionBoxLinesInfo.Begin() + 1u,
             endIt = selectionBoxLinesInfo.End() - 1u;
           it != endIt;
           ++it )
      {
        const SelectionBoxInfo& info = *it;

        mEventData->mDecorator->AddHighlight( 0.f,
                                              info.lineOffset,
                                              info.minX,
                                              info.lineOffset + info.lineHeight );

        mEventData->mDecorator->AddHighlight( info.maxX,
                                              info.lineOffset,
                                              mVisualModel->mControlSize.width,
                                              info.lineOffset + info.lineHeight );
      }

      // Update the size of the highlighted text.
      minHighlightX = 0.f;
      maxHighlightX = mVisualModel->mControlSize.width;
    }

    // Boxify the last line.
    lineRun = mVisualModel->mLines.Begin() + firstLineIndex + numberOfLines - 1u;
    const SelectionBoxInfo& lastSelectionBoxLineInfo = *( selectionBoxLinesInfo.End() - 1u );

    boxifyBegin = ( LTR == lineRun->direction ) && ( LTR == endDirection );
    boxifyEnd = ( LTR != lineRun->direction ) && ( LTR != endDirection );

    if( boxifyBegin )
    {
      // Boxify at the beginning of the line.
      mEventData->mDecorator->AddHighlight( 0.f,
                                            lastSelectionBoxLineInfo.lineOffset,
                                            lastSelectionBoxLineInfo.minX,
                                            lastSelectionBoxLineInfo.lineOffset + lastSelectionBoxLineInfo.lineHeight );

      // Update the size of the highlighted text.
      minHighlightX = 0.f;
    }

    if( boxifyEnd )
    {
      // Boxify at the end of the line.
      mEventData->mDecorator->AddHighlight( lastSelectionBoxLineInfo.maxX,
                                            lastSelectionBoxLineInfo.lineOffset,
                                            mVisualModel->mControlSize.width,
                                            lastSelectionBoxLineInfo.lineOffset + lastSelectionBoxLineInfo.lineHeight );

      // Update the size of the highlighted text.
      maxHighlightX = mVisualModel->mControlSize.width;
    }
  }

  // Sets the highlight's size and position. In decorator's coords.
  // The highlight's height has been calculated above (before 'boxifying' the highlight).
  highLightSize.width = maxHighlightX - minHighlightX;

  highLightPosition.x = minHighlightX;
  const SelectionBoxInfo& firstSelectionBoxLineInfo = *( selectionBoxLinesInfo.Begin() );
  highLightPosition.y = firstSelectionBoxLineInfo.lineOffset;

  mEventData->mDecorator->SetHighLightBox( highLightPosition, highLightSize );

  if( !mEventData->mDecorator->IsSmoothHandlePanEnabled() )
  {
    CursorInfo primaryCursorInfo;
    GetCursorPosition( mEventData->mLeftSelectionPosition,
                       primaryCursorInfo );

    const Vector2 primaryPosition = primaryCursorInfo.primaryPosition + mScrollPosition;

    mEventData->mDecorator->SetPosition( LEFT_SELECTION_HANDLE,
                                         primaryPosition.x,
                                         primaryCursorInfo.lineOffset + mScrollPosition.y,
                                         primaryCursorInfo.lineHeight );

    CursorInfo secondaryCursorInfo;
    GetCursorPosition( mEventData->mRightSelectionPosition,
                       secondaryCursorInfo );

    const Vector2 secondaryPosition = secondaryCursorInfo.primaryPosition + mScrollPosition;

    mEventData->mDecorator->SetPosition( RIGHT_SELECTION_HANDLE,
                                         secondaryPosition.x,
                                         secondaryCursorInfo.lineOffset + mScrollPosition.y,
                                         secondaryCursorInfo.lineHeight );
  }

  // Cursor to be positioned at end of selection so if selection interrupted and edit mode restarted the cursor will be at end of selection
  mEventData->mPrimaryCursorPosition = ( indicesSwapped ) ? mEventData->mLeftSelectionPosition : mEventData->mRightSelectionPosition;

  // Set the flag to update the decorator.
  mEventData->mDecoratorUpdated = true;
}

void Controller::Impl::RepositionSelectionHandles( float visualX, float visualY )
{
  if( NULL == mEventData )
  {
    // Nothing to do if there is no text input.
    return;
  }

  if( IsShowingPlaceholderText() )
  {
    // Nothing to do if there is the place-holder text.
    return;
  }

  const Length numberOfGlyphs = mVisualModel->mGlyphs.Count();
  const Length numberOfLines  = mVisualModel->mLines.Count();
  if( ( 0 == numberOfGlyphs ) ||
      ( 0 == numberOfLines ) )
  {
    // Nothing to do if there is no text.
    return;
  }

  // Find which word was selected
  CharacterIndex selectionStart( 0 );
  CharacterIndex selectionEnd( 0 );
  const bool indicesFound = FindSelectionIndices( mVisualModel,
                                                  mLogicalModel,
                                                  mMetrics,
                                                  visualX,
                                                  visualY,
                                                  selectionStart,
                                                  selectionEnd );
  DALI_LOG_INFO( gLogFilter, Debug::Verbose, "%p selectionStart %d selectionEnd %d\n", this, selectionStart, selectionEnd );

  if( indicesFound )
  {
    ChangeState( EventData::SELECTING );

    mEventData->mLeftSelectionPosition = selectionStart;
    mEventData->mRightSelectionPosition = selectionEnd;

    mEventData->mUpdateLeftSelectionPosition = true;
    mEventData->mUpdateRightSelectionPosition = true;
    mEventData->mUpdateHighlightBox = true;

    mEventData->mScrollAfterUpdatePosition = ( mEventData->mLeftSelectionPosition != mEventData->mRightSelectionPosition );
  }
  else
  {
    // Nothing to select. i.e. a white space, out of bounds
    ChangeState( EventData::EDITING );

    mEventData->mPrimaryCursorPosition = selectionEnd;

    mEventData->mUpdateCursorPosition = true;
    mEventData->mUpdateGrabHandlePosition = true;
    mEventData->mScrollAfterUpdatePosition = true;
    mEventData->mUpdateInputStyle = true;
  }
}

void Controller::Impl::SetPopupButtons()
{
  /**
   *  Sets the Popup buttons to be shown depending on State.
   *
   *  If SELECTING :  CUT & COPY + ( PASTE & CLIPBOARD if content available to paste )
   *
   *  If EDITING_WITH_POPUP : SELECT & SELECT_ALL
   */

  TextSelectionPopup::Buttons buttonsToShow = TextSelectionPopup::NONE;

  if( EventData::SELECTING == mEventData->mState )
  {
    buttonsToShow = TextSelectionPopup::Buttons(  TextSelectionPopup::CUT | TextSelectionPopup::COPY );

    if( !IsClipboardEmpty() )
    {
      buttonsToShow = TextSelectionPopup::Buttons ( ( buttonsToShow | TextSelectionPopup::PASTE ) );
      buttonsToShow = TextSelectionPopup::Buttons ( ( buttonsToShow | TextSelectionPopup::CLIPBOARD ) );
    }

    if( !mEventData->mAllTextSelected )
    {
      buttonsToShow = TextSelectionPopup::Buttons ( ( buttonsToShow | TextSelectionPopup::SELECT_ALL ) );
    }
  }
  else if( EventData::EDITING_WITH_POPUP == mEventData->mState )
  {
    if( mLogicalModel->mText.Count() && !IsShowingPlaceholderText() )
    {
      buttonsToShow = TextSelectionPopup::Buttons( TextSelectionPopup::SELECT | TextSelectionPopup::SELECT_ALL );
    }

    if( !IsClipboardEmpty() )
    {
      buttonsToShow = TextSelectionPopup::Buttons ( ( buttonsToShow | TextSelectionPopup::PASTE ) );
      buttonsToShow = TextSelectionPopup::Buttons ( ( buttonsToShow | TextSelectionPopup::CLIPBOARD ) );
    }
  }
  else if( EventData::EDITING_WITH_PASTE_POPUP == mEventData->mState )
  {
    if ( !IsClipboardEmpty() )
    {
      buttonsToShow = TextSelectionPopup::Buttons ( ( buttonsToShow | TextSelectionPopup::PASTE ) );
      buttonsToShow = TextSelectionPopup::Buttons ( ( buttonsToShow | TextSelectionPopup::CLIPBOARD ) );
    }
  }

  mEventData->mDecorator->SetEnabledPopupButtons( buttonsToShow );
}

void Controller::Impl::ChangeState( EventData::State newState )
{
  if( NULL == mEventData )
  {
    // Nothing to do if there is no text input.
    return;
  }

  DALI_LOG_INFO( gLogFilter, Debug::General, "ChangeState state:%d  newstate:%d\n", mEventData->mState, newState );

  if( mEventData->mState != newState )
  {
    mEventData->mState = newState;

    if( EventData::INACTIVE == mEventData->mState )
    {
      mEventData->mDecorator->SetActiveCursor( ACTIVE_CURSOR_NONE );
      mEventData->mDecorator->StopCursorBlink();
      mEventData->mDecorator->SetHandleActive( GRAB_HANDLE, false );
      mEventData->mDecorator->SetHandleActive( LEFT_SELECTION_HANDLE, false );
      mEventData->mDecorator->SetHandleActive( RIGHT_SELECTION_HANDLE, false );
      mEventData->mDecorator->SetPopupActive( false );
      mEventData->mDecoratorUpdated = true;
      HideClipboard();
    }
    else if( EventData::INTERRUPTED  == mEventData->mState)
    {
      mEventData->mDecorator->SetHandleActive( GRAB_HANDLE, false );
      mEventData->mDecorator->SetHandleActive( LEFT_SELECTION_HANDLE, false );
      mEventData->mDecorator->SetHandleActive( RIGHT_SELECTION_HANDLE, false );
      mEventData->mDecorator->SetPopupActive( false );
      mEventData->mDecoratorUpdated = true;
      HideClipboard();
    }
    else if( EventData::SELECTING == mEventData->mState )
    {
      mEventData->mDecorator->SetActiveCursor( ACTIVE_CURSOR_NONE );
      mEventData->mDecorator->StopCursorBlink();
      mEventData->mDecorator->SetHandleActive( GRAB_HANDLE, false );
      mEventData->mDecorator->SetHandleActive( LEFT_SELECTION_HANDLE, true );
      mEventData->mDecorator->SetHandleActive( RIGHT_SELECTION_HANDLE, true );
      if( mEventData->mGrabHandlePopupEnabled )
      {
        SetPopupButtons();
        mEventData->mDecorator->SetPopupActive( true );
      }
      mEventData->mDecoratorUpdated = true;
    }
    else if( EventData::EDITING == mEventData->mState )
    {
      mEventData->mDecorator->SetActiveCursor( ACTIVE_CURSOR_PRIMARY );
      if( mEventData->mCursorBlinkEnabled )
      {
        mEventData->mDecorator->StartCursorBlink();
      }
      // Grab handle is not shown until a tap is received whilst EDITING
      mEventData->mDecorator->SetHandleActive( GRAB_HANDLE, false );
      mEventData->mDecorator->SetHandleActive( LEFT_SELECTION_HANDLE, false );
      mEventData->mDecorator->SetHandleActive( RIGHT_SELECTION_HANDLE, false );
      if( mEventData->mGrabHandlePopupEnabled )
      {
        mEventData->mDecorator->SetPopupActive( false );
      }
      mEventData->mDecoratorUpdated = true;
      HideClipboard();
    }
    else if( EventData::EDITING_WITH_POPUP == mEventData->mState )
    {
      DALI_LOG_INFO( gLogFilter, Debug::Verbose, "EDITING_WITH_POPUP \n", newState );

      mEventData->mDecorator->SetActiveCursor( ACTIVE_CURSOR_PRIMARY );
      if( mEventData->mCursorBlinkEnabled )
      {
        mEventData->mDecorator->StartCursorBlink();
      }
      if( mEventData->mSelectionEnabled )
      {
        mEventData->mDecorator->SetHandleActive( LEFT_SELECTION_HANDLE, false );
        mEventData->mDecorator->SetHandleActive( RIGHT_SELECTION_HANDLE, false );
      }
      else
      {
        mEventData->mDecorator->SetHandleActive( GRAB_HANDLE, true );
      }
      if( mEventData->mGrabHandlePopupEnabled )
      {
        SetPopupButtons();
        mEventData->mDecorator->SetPopupActive( true );
      }
      HideClipboard();
      mEventData->mDecoratorUpdated = true;
    }
    else if( EventData::EDITING_WITH_GRAB_HANDLE == mEventData->mState )
    {
      DALI_LOG_INFO( gLogFilter, Debug::Verbose, "EDITING_WITH_GRAB_HANDLE \n", newState );

      mEventData->mDecorator->SetActiveCursor( ACTIVE_CURSOR_PRIMARY );
      if( mEventData->mCursorBlinkEnabled )
      {
        mEventData->mDecorator->StartCursorBlink();
      }
      // Grab handle is not shown until a tap is received whilst EDITING
      mEventData->mDecorator->SetHandleActive( GRAB_HANDLE, true );
      mEventData->mDecorator->SetHandleActive( LEFT_SELECTION_HANDLE, false );
      mEventData->mDecorator->SetHandleActive( RIGHT_SELECTION_HANDLE, false );
      if( mEventData->mGrabHandlePopupEnabled )
      {
        mEventData->mDecorator->SetPopupActive( false );
      }
      mEventData->mDecoratorUpdated = true;
      HideClipboard();
    }
    else if( EventData::SELECTION_HANDLE_PANNING == mEventData->mState )
    {
      mEventData->mDecorator->SetActiveCursor( ACTIVE_CURSOR_NONE );
      mEventData->mDecorator->StopCursorBlink();
      mEventData->mDecorator->SetHandleActive( GRAB_HANDLE, false );
      mEventData->mDecorator->SetHandleActive( LEFT_SELECTION_HANDLE, true );
      mEventData->mDecorator->SetHandleActive( RIGHT_SELECTION_HANDLE, true );
      if( mEventData->mGrabHandlePopupEnabled )
      {
        mEventData->mDecorator->SetPopupActive( false );
      }
      mEventData->mDecoratorUpdated = true;
    }
    else if( EventData::GRAB_HANDLE_PANNING == mEventData->mState )
    {
      DALI_LOG_INFO( gLogFilter, Debug::Verbose, "GRAB_HANDLE_PANNING \n", newState );

      mEventData->mDecorator->SetActiveCursor( ACTIVE_CURSOR_PRIMARY );
      if( mEventData->mCursorBlinkEnabled )
      {
        mEventData->mDecorator->StartCursorBlink();
      }
      mEventData->mDecorator->SetHandleActive( GRAB_HANDLE, true );
      mEventData->mDecorator->SetHandleActive( LEFT_SELECTION_HANDLE, false );
      mEventData->mDecorator->SetHandleActive( RIGHT_SELECTION_HANDLE, false );
      if( mEventData->mGrabHandlePopupEnabled )
      {
        mEventData->mDecorator->SetPopupActive( false );
      }
      mEventData->mDecoratorUpdated = true;
    }
    else if( EventData::EDITING_WITH_PASTE_POPUP == mEventData->mState )
    {
      DALI_LOG_INFO( gLogFilter, Debug::Verbose, "EDITING_WITH_PASTE_POPUP \n", newState );

      mEventData->mDecorator->SetActiveCursor( ACTIVE_CURSOR_PRIMARY );
      if( mEventData->mCursorBlinkEnabled )
      {
        mEventData->mDecorator->StartCursorBlink();
      }

      mEventData->mDecorator->SetHandleActive( GRAB_HANDLE, true );
      mEventData->mDecorator->SetHandleActive( LEFT_SELECTION_HANDLE, false );
      mEventData->mDecorator->SetHandleActive( RIGHT_SELECTION_HANDLE, false );

      if( mEventData->mGrabHandlePopupEnabled )
      {
        SetPopupButtons();
        mEventData->mDecorator->SetPopupActive( true );
      }
      HideClipboard();
      mEventData->mDecoratorUpdated = true;
    }
  }
}

void Controller::Impl::GetCursorPosition( CharacterIndex logical,
                                          CursorInfo& cursorInfo )
{
  if( !IsShowingRealText() )
  {
    // Do not want to use the place-holder text to set the cursor position.

    // Use the line's height of the font's family set to set the cursor's size.
    // If there is no font's family set, use the default font.
    // Use the current alignment to place the cursor at the beginning, center or end of the box.

    cursorInfo.lineOffset = 0.f;
    cursorInfo.lineHeight = GetDefaultFontLineHeight();
    cursorInfo.primaryCursorHeight = cursorInfo.lineHeight;

    switch( mLayoutEngine.GetHorizontalAlignment() )
    {
      case LayoutEngine::HORIZONTAL_ALIGN_BEGIN:
      {
        cursorInfo.primaryPosition.x = 0.f;
        break;
      }
      case LayoutEngine::HORIZONTAL_ALIGN_CENTER:
      {
        cursorInfo.primaryPosition.x = floorf( 0.5f * mVisualModel->mControlSize.width );
        break;
      }
      case LayoutEngine::HORIZONTAL_ALIGN_END:
      {
        cursorInfo.primaryPosition.x = mVisualModel->mControlSize.width - mEventData->mDecorator->GetCursorWidth();
        break;
      }
    }

    // Nothing else to do.
    return;
  }

  Text::GetCursorPosition( mVisualModel,
                           mLogicalModel,
                           mMetrics,
                           logical,
                           cursorInfo );

  if( LayoutEngine::MULTI_LINE_BOX == mLayoutEngine.GetLayout() )
  {
    // If the text is editable and multi-line, the cursor position after a white space shouldn't exceed the boundaries of the text control.

    // Note the white spaces laid-out at the end of the line might exceed the boundaries of the control.
    // The reason is a wrapped line must not start with a white space so they are laid-out at the end of the line.

    if( 0.f > cursorInfo.primaryPosition.x )
    {
      cursorInfo.primaryPosition.x = 0.f;
    }

    const float edgeWidth = mVisualModel->mControlSize.width - static_cast<float>( mEventData->mDecorator->GetCursorWidth() );
    if( cursorInfo.primaryPosition.x > edgeWidth )
    {
      cursorInfo.primaryPosition.x = edgeWidth;
    }
  }
}

CharacterIndex Controller::Impl::CalculateNewCursorIndex( CharacterIndex index ) const
{
  if( NULL == mEventData )
  {
    // Nothing to do if there is no text input.
    return 0u;
  }

  CharacterIndex cursorIndex = mEventData->mPrimaryCursorPosition;

  const GlyphIndex* const charactersToGlyphBuffer = mVisualModel->mCharactersToGlyph.Begin();
  const Length* const charactersPerGlyphBuffer = mVisualModel->mCharactersPerGlyph.Begin();

  GlyphIndex glyphIndex = *( charactersToGlyphBuffer + index );
  Length numberOfCharacters = *( charactersPerGlyphBuffer + glyphIndex );

  if( numberOfCharacters > 1u )
  {
    const Script script = mLogicalModel->GetScript( index );
    if( HasLigatureMustBreak( script ) )
    {
      // Prevents to jump the whole Latin ligatures like fi, ff, or Arabic ﻻ,  ...
      numberOfCharacters = 1u;
    }
  }
  else
  {
    while( 0u == numberOfCharacters )
    {
      ++glyphIndex;
      numberOfCharacters = *( charactersPerGlyphBuffer + glyphIndex );
    }
  }

  if( index < mEventData->mPrimaryCursorPosition )
  {
    cursorIndex -= numberOfCharacters;
  }
  else
  {
    cursorIndex += numberOfCharacters;
  }

  // Will update the cursor hook position.
  mEventData->mUpdateCursorHookPosition = true;

  return cursorIndex;
}

void Controller::Impl::UpdateCursorPosition( const CursorInfo& cursorInfo )
{
  DALI_LOG_INFO( gLogFilter, Debug::Verbose, "-->Controller::UpdateCursorPosition %p\n", this );
  if( NULL == mEventData )
  {
    // Nothing to do if there is no text input.
    DALI_LOG_INFO( gLogFilter, Debug::Verbose, "<--Controller::UpdateCursorPosition no event data\n" );
    return;
  }

  const Vector2 cursorPosition = cursorInfo.primaryPosition + mScrollPosition;

  // Sets the cursor position.
  mEventData->mDecorator->SetPosition( PRIMARY_CURSOR,
                                       cursorPosition.x,
                                       cursorPosition.y,
                                       cursorInfo.primaryCursorHeight,
                                       cursorInfo.lineHeight );
  DALI_LOG_INFO( gLogFilter, Debug::Verbose, "Primary cursor position: %f,%f\n", cursorPosition.x, cursorPosition.y );

  if( mEventData->mUpdateGrabHandlePosition )
  {
    // Sets the grab handle position.
    mEventData->mDecorator->SetPosition( GRAB_HANDLE,
                                         cursorPosition.x,
                                         cursorInfo.lineOffset + mScrollPosition.y,
                                         cursorInfo.lineHeight );
  }

  if( cursorInfo.isSecondaryCursor )
  {
    mEventData->mDecorator->SetPosition( SECONDARY_CURSOR,
                                         cursorInfo.secondaryPosition.x + mScrollPosition.x,
                                         cursorInfo.secondaryPosition.y + mScrollPosition.y,
                                         cursorInfo.secondaryCursorHeight,
                                         cursorInfo.lineHeight );
    DALI_LOG_INFO( gLogFilter, Debug::Verbose, "Secondary cursor position: %f,%f\n", cursorInfo.secondaryPosition.x + mScrollPosition.x, cursorInfo.secondaryPosition.y + mScrollPosition.y );
  }

  // Set which cursors are active according the state.
  if( EventData::IsEditingState( mEventData->mState ) || ( EventData::GRAB_HANDLE_PANNING == mEventData->mState ) )
  {
    if( cursorInfo.isSecondaryCursor )
    {
      mEventData->mDecorator->SetActiveCursor( ACTIVE_CURSOR_BOTH );
    }
    else
    {
      mEventData->mDecorator->SetActiveCursor( ACTIVE_CURSOR_PRIMARY );
    }
  }
  else
  {
    mEventData->mDecorator->SetActiveCursor( ACTIVE_CURSOR_NONE );
  }

  DALI_LOG_INFO( gLogFilter, Debug::Verbose, "<--Controller::UpdateCursorPosition\n" );
}

void Controller::Impl::UpdateSelectionHandle( HandleType handleType,
                                              const CursorInfo& cursorInfo )
{
  if( ( LEFT_SELECTION_HANDLE != handleType ) &&
      ( RIGHT_SELECTION_HANDLE != handleType ) )
  {
    return;
  }

  const Vector2 cursorPosition = cursorInfo.primaryPosition + mScrollPosition;

  // Sets the handle's position.
  mEventData->mDecorator->SetPosition( handleType,
                                       cursorPosition.x,
                                       cursorInfo.lineOffset + mScrollPosition.y,
                                       cursorInfo.lineHeight );

  // If selection handle at start of the text and other at end of the text then all text is selected.
  const CharacterIndex startOfSelection = std::min( mEventData->mLeftSelectionPosition, mEventData->mRightSelectionPosition );
  const CharacterIndex endOfSelection = std::max ( mEventData->mLeftSelectionPosition, mEventData->mRightSelectionPosition );
  mEventData->mAllTextSelected = ( startOfSelection == 0 ) && ( endOfSelection == mLogicalModel->mText.Count() );
}

void Controller::Impl::ClampHorizontalScroll( const Vector2& actualSize )
{
  // Clamp between -space & 0.

  if( actualSize.width > mVisualModel->mControlSize.width )
  {
    const float space = ( actualSize.width - mVisualModel->mControlSize.width );
    mScrollPosition.x = ( mScrollPosition.x < -space ) ? -space : mScrollPosition.x;
    mScrollPosition.x = ( mScrollPosition.x > 0.f ) ? 0.f : mScrollPosition.x;

    mEventData->mDecoratorUpdated = true;
  }
  else
  {
    mScrollPosition.x = 0.f;
  }
}

void Controller::Impl::ClampVerticalScroll( const Vector2& actualSize )
{
  // Clamp between -space & 0.
  if( actualSize.height > mVisualModel->mControlSize.height )
  {
    const float space = ( actualSize.height - mVisualModel->mControlSize.height );
    mScrollPosition.y = ( mScrollPosition.y < -space ) ? -space : mScrollPosition.y;
    mScrollPosition.y = ( mScrollPosition.y > 0.f ) ? 0.f : mScrollPosition.y;

    mEventData->mDecoratorUpdated = true;
  }
  else
  {
    mScrollPosition.y = 0.f;
  }
}

void Controller::Impl::ScrollToMakePositionVisible( const Vector2& position, float lineHeight )
{
  const float cursorWidth = mEventData->mDecorator ? static_cast<float>( mEventData->mDecorator->GetCursorWidth() ) : 0.f;

  // position is in actor's coords.
  const float positionEndX = position.x + cursorWidth;
  const float positionEndY = position.y + lineHeight;

  // Transform the position to decorator coords.
  const float decoratorPositionBeginX = position.x + mScrollPosition.x;
  const float decoratorPositionEndX = positionEndX + mScrollPosition.x;

  const float decoratorPositionBeginY = position.y + mScrollPosition.y;
  const float decoratorPositionEndY = positionEndY + mScrollPosition.y;

  if( decoratorPositionBeginX < 0.f )
  {
    mScrollPosition.x = -position.x;
  }
  else if( decoratorPositionEndX > mVisualModel->mControlSize.width )
  {
    mScrollPosition.x = mVisualModel->mControlSize.width - positionEndX;
  }

  if( decoratorPositionBeginY < 0.f )
  {
    mScrollPosition.y = -position.y;
  }
  else if( decoratorPositionEndY > mVisualModel->mControlSize.height )
  {
    mScrollPosition.y = mVisualModel->mControlSize.height - positionEndY;
  }
}

void Controller::Impl::ScrollTextToMatchCursor( const CursorInfo& cursorInfo )
{
  // Get the current cursor position in decorator coords.
  const Vector2& currentCursorPosition = mEventData->mDecorator->GetPosition( PRIMARY_CURSOR );

  // Calculate the offset to match the cursor position before the character was deleted.
  mScrollPosition.x = currentCursorPosition.x - cursorInfo.primaryPosition.x;
  mScrollPosition.y = currentCursorPosition.y - cursorInfo.lineOffset;

  ClampHorizontalScroll( mVisualModel->GetLayoutSize() );
  ClampVerticalScroll( mVisualModel->GetLayoutSize() );

  // Makes the new cursor position visible if needed.
  ScrollToMakePositionVisible( cursorInfo.primaryPosition, cursorInfo.lineHeight );
}

void Controller::Impl::RequestRelayout()
{
  mControlInterface.RequestTextRelayout();
}

} // namespace Text

} // namespace Toolkit

} // namespace Dali
