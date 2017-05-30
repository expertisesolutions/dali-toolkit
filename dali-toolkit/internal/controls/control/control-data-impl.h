#ifndef DALI_TOOLKIT_CONTROL_DATA_IMPL_H
#define DALI_TOOLKIT_CONTROL_DATA_IMPL_H

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
#include <dali/public-api/object/type-registry.h>
#include <dali-toolkit/devel-api/controls/control-devel.h>
#include <string>

// INTERNAL INCLUDES
#include <dali-toolkit/internal/visuals/visual-resource-observer.h>
#include <dali-toolkit/public-api/controls/control-impl.h>
#include <dali/devel-api/common/owner-container.h>
#include <dali-toolkit/devel-api/visual-factory/visual-base.h>
#include <dali-toolkit/internal/controls/tooltip/tooltip.h>
#include <dali-toolkit/internal/builder/style.h>
#include <dali-toolkit/internal/builder/dictionary.h>

namespace Dali
{

namespace Toolkit
{

namespace Internal
{

/**
  * Struct used to store Visual within the control, index is a unique key for each visual.
  */
 struct RegisteredVisual
 {
   Property::Index index;
   Toolkit::Visual::Base visual;
   bool enabled;

   RegisteredVisual( Property::Index aIndex, Toolkit::Visual::Base &aVisual, bool aEnabled)
   : index(aIndex), visual(aVisual), enabled(aEnabled)
   {
   }
 };

typedef Dali::OwnerContainer< RegisteredVisual* > RegisteredVisualContainer;


/**
 * @brief Holds the Implementation for the internal control class
 */
class Control::Impl : public ConnectionTracker, public Visual::ResourceObserver
{

public:

  /**
   * @brief Retrieves the implementation of the internal control class.
   * @param[in] internalControl A ref to the control whose internal implementation is required
   * @return The internal implementation
   */
  static Control::Impl& Get( Internal::Control& internalControl );

  /**
   * @copydoc Get( Internal::Control& )
   */
  static const Control::Impl& Get( const Internal::Control& internalControl );

  /**
   * @brief Constructor.
   * @param[in] controlImpl The control which own this implementation
   */
  Impl( Control& controlImpl );

  /**
   * @brief Destructor.
   */
  ~Impl();

  /**
   * @brief Called when a pinch is detected.
   * @param[in] actor The actor the pinch occurred on
   * @param[in] pinch The pinch gesture details
   */
  void PinchDetected(Actor actor, const PinchGesture& pinch);

  /**
   * @brief Called when a pan is detected.
   * @param[in] actor The actor the pan occurred on
   * @param[in] pinch The pan gesture details
   */
  void PanDetected(Actor actor, const PanGesture& pan);

  /**
   * @brief Called when a tap is detected.
   * @param[in] actor The actor the tap occurred on
   * @param[in] pinch The tap gesture details
   */
  void TapDetected(Actor actor, const TapGesture& tap);

  /**
   * @brief Called when a long-press is detected.
   * @param[in] actor The actor the long-press occurred on
   * @param[in] pinch The long-press gesture details
   */
  void LongPressDetected(Actor actor, const LongPressGesture& longPress);

  /**
   * @brief Called when a resource is ready.
   * @param[in] object The visual whose resources are ready
   * @note Overriding method in Visual::ResourceObserver.
   */
  virtual void ResourceReady( Visual::Base& object );

  /**
   * @copydoc Dali::Toolkit::DevelControl::RegisterVisual()
   */
  void RegisterVisual( Property::Index index, Toolkit::Visual::Base& visual );

  /**
   * @copydoc Dali::Toolkit::DevelControl::RegisterVisual()
   */
  void RegisterVisual( Property::Index index, Toolkit::Visual::Base& visual, bool enabled );

  /**
   * @copydoc Dali::Toolkit::DevelControl::UnregisterVisual()
   */
  void UnregisterVisual( Property::Index index );

  /**
   * @copydoc Dali::Toolkit::DevelControl::GetVisual()
   */
  Toolkit::Visual::Base GetVisual( Property::Index index ) const;

  /**
   * @copydoc Dali::Toolkit::DevelControl::EnableVisual()
   */
  void EnableVisual( Property::Index index, bool enable );

  /**
   * @copydoc Dali::Toolkit::DevelControl::IsVisualEnabled()
   */
  bool IsVisualEnabled( Property::Index index ) const;

  /**
   * @brief Stops observing the given visual.
   * @param[in] visual The visual to stop observing
   */
  void StopObservingVisual( Toolkit::Visual::Base& visual );

  /**
   * @brief Starts observing the given visual.
   * @param[in] visual The visual to start observing
   */
  void StartObservingVisual( Toolkit::Visual::Base& visual);

  /**
   * @copydoc Dali::Toolkit::DevelControl::CreateTransition()
   */
  Dali::Animation CreateTransition( const Toolkit::TransitionData& transitionData );

  /**
   * @brief Function used to set control properties.
   * @param[in] object The object whose property to set
   * @param[in] index The index of the property to set
   * @param[in] value The value of the property to set
   */
  static void SetProperty( BaseObject* object, Property::Index index, const Property::Value& value );

  /**
   * @brief Function used to retrieve the value of control properties.
   * @param[in] object The object whose property to get
   * @param[in] index The index of the property to get
   * @return The value of the property
   */
  static Property::Value GetProperty( BaseObject* object, Property::Index index );

  /**
   * @brief Sets the state of the control.
   * @param[in] newState The state to set
   * @param[in] withTransitions Whether to show a transition when changing to the new state
   */
  void SetState( DevelControl::State newState, bool withTransitions=true );

  /**
   * @brief Sets the sub-state of the control.
   * @param[in] newState The sub-state to set
   * @param[in] withTransitions Whether to show a transition when changing to the new sub-state
   */
  void SetSubState( const std::string& subStateName, bool withTransitions=true );

  /**
   * @brief Replaces visuals and properties from the old state to the new state.
   * @param[in] oldState The old state
   * @param[in] newState The new state
   * @param[in] subState The current sub state
   */
  void ReplaceStateVisualsAndProperties( const StylePtr oldState, const StylePtr newState, const std::string& subState );

  /**
   * @brief Removes a visual from the control's container.
   * @param[in] visuals The container of visuals
   * @param[in] visualName The name of the visual to remove
   */
  void RemoveVisual( RegisteredVisualContainer& visuals, const std::string& visualName );

  /**
   * @brief Removes several visuals from the control's container.
   * @param[in] visuals The container of visuals
   * @param[in] removeVisuals The visuals to remove
   */
  void RemoveVisuals( RegisteredVisualContainer& visuals, DictionaryKeys& removeVisuals );

  /**
   * @brief Copies the visual properties that are specific to the control instance into the instancedProperties container.
   * @param[in] visuals The control's visual container
   * @param[out] instancedProperties The instanced properties are added to this container
   */
  void CopyInstancedProperties( RegisteredVisualContainer& visuals, Dictionary<Property::Map>& instancedProperties );

  /**
   * @brief On state change, ensures visuals are moved or created appropriately.
   *
   * Go through the list of visuals that are common to both states.
   * If they are different types, or are both image types with different
   * URLs, then the existing visual needs moving and the new visual needs creating
   *
   * @param[in] stateVisualsToChange The visuals to change
   * @param[in] instancedProperties The instanced properties @see CopyInstancedProperties
   */
  void RecreateChangedVisuals( Dictionary<Property::Map>& stateVisualsToChange, Dictionary<Property::Map>& instancedProperties );

  /**
   * @brief Whether the resource is ready
   * @return True if the resource is read.
   */
  bool IsResourceReady() const;

  Control& mControlImpl;
  DevelControl::State mState;
  std::string mSubStateName;

  int mLeftFocusableActorId;       ///< Actor ID of Left focusable control.
  int mRightFocusableActorId;      ///< Actor ID of Right focusable control.
  int mUpFocusableActorId;         ///< Actor ID of Up focusable control.
  int mDownFocusableActorId;       ///< Actor ID of Down focusable control.

  RegisteredVisualContainer mVisuals;     ///< Stores visuals needed by the control, non trivial type so std::vector used.
  std::string mStyleName;
  Vector4 mBackgroundColor;               ///< The color of the background visual
  Vector3* mStartingPinchScale;           ///< The scale when a pinch gesture starts, TODO: consider removing this
  Toolkit::Control::KeyEventSignalType mKeyEventSignal;
  Toolkit::Control::KeyInputFocusSignalType mKeyInputFocusGainedSignal;
  Toolkit::Control::KeyInputFocusSignalType mKeyInputFocusLostSignal;

  Toolkit::DevelControl::ResourceReadySignalType mResourceReadySignal;

  // Gesture Detection
  PinchGestureDetector mPinchGestureDetector;
  PanGestureDetector mPanGestureDetector;
  TapGestureDetector mTapGestureDetector;
  LongPressGestureDetector mLongPressGestureDetector;

  // Tooltip
  TooltipPtr mTooltip;

  ControlBehaviour mFlags : CONTROL_BEHAVIOUR_FLAG_COUNT;    ///< Flags passed in from constructor.
  bool mIsKeyboardNavigationSupported :1;  ///< Stores whether keyboard navigation is supported by the control.
  bool mIsKeyboardFocusGroup :1;           ///< Stores whether the control is a focus group.

  // Properties - these need to be members of Internal::Control::Impl as they access private methods/data of Internal::Control and Internal::Control::Impl.
  static const PropertyRegistration PROPERTY_1;
  static const PropertyRegistration PROPERTY_2;
  static const PropertyRegistration PROPERTY_3;
  static const PropertyRegistration PROPERTY_4;
  static const PropertyRegistration PROPERTY_5;
  static const PropertyRegistration PROPERTY_6;
  static const PropertyRegistration PROPERTY_7;
  static const PropertyRegistration PROPERTY_8;
  static const PropertyRegistration PROPERTY_9;
  static const PropertyRegistration PROPERTY_10;
  static const PropertyRegistration PROPERTY_11;
  static const PropertyRegistration PROPERTY_12;
};


} // namespace Internal

} // namespace Toolkit

} // namespace Dali

#endif // DALI_TOOLKIT_CONTROL_DATA_IMPL_H