/** Copyright (c) 2017 Samsung Electronics Co., Ltd.
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
// This File has been auto-generated by SWIG and then modified using DALi Ruby Scripts
//------------------------------------------------------------------------------
// <auto-generated />
//
// This file was automatically generated by SWIG (http://www.swig.org).
// Version 3.0.10
//
// Do not make changes to this file unless you know what you are doing--modify
// the SWIG interface file instead.
//------------------------------------------------------------------------------

namespace Dali {

using System;
using System.Runtime.InteropServices;


public class ScrollBar : View {
  private global::System.Runtime.InteropServices.HandleRef swigCPtr;

  internal ScrollBar(global::System.IntPtr cPtr, bool cMemoryOwn) : base(NDalicPINVOKE.ScrollBar_SWIGUpcast(cPtr), cMemoryOwn) {
    swigCPtr = new global::System.Runtime.InteropServices.HandleRef(this, cPtr);
  }

  internal static global::System.Runtime.InteropServices.HandleRef getCPtr(ScrollBar obj) {
    return (obj == null) ? new global::System.Runtime.InteropServices.HandleRef(null, global::System.IntPtr.Zero) : obj.swigCPtr;
  }

  ~ScrollBar() {
    DisposeQueue.Instance.Add(this);
  }

  public override void Dispose() {
    if (!Stage.IsInstalled()) {
      DisposeQueue.Instance.Add(this);
      return;
    }

    lock(this) {
      if (swigCPtr.Handle != global::System.IntPtr.Zero) {
        if (swigCMemOwn) {
          swigCMemOwn = false;
          NDalicPINVOKE.delete_ScrollBar(swigCPtr);
        }
        swigCPtr = new global::System.Runtime.InteropServices.HandleRef(null, global::System.IntPtr.Zero);
      }
      global::System.GC.SuppressFinalize(this);
      base.Dispose();
    }
  }




public class PanFinishedEventArgs : EventArgs
{
}

public class ScrollPositionIntervalReachedEventArgs : EventArgs
{
   private float _currentScrollPosition;

   public float CurrentScrollPosition
   {
      get
      {
         return _currentScrollPosition;
      }
      set
      {
         _currentScrollPosition = value;
      }
   }
}

  [UnmanagedFunctionPointer(CallingConvention.StdCall)]
  private delegate void PanFinishedEventCallbackDelegate();
  private DaliEventHandler<object,PanFinishedEventArgs> _scrollBarPanFinishedEventHandler;
  private PanFinishedEventCallbackDelegate _scrollBarPanFinishedEventCallbackDelegate;
  
  [UnmanagedFunctionPointer(CallingConvention.StdCall)]
  private delegate void ScrollPositionIntervalReachedEventCallbackDelegate();
  private DaliEventHandler<object,ScrollPositionIntervalReachedEventArgs> _scrollBarScrollPositionIntervalReachedEventHandler;
  private ScrollPositionIntervalReachedEventCallbackDelegate _scrollBarScrollPositionIntervalReachedEventCallbackDelegate;

  public event DaliEventHandler<object,PanFinishedEventArgs> PanFinished
  {
     add
     {
        lock(this)
        {
           // Restricted to only one listener
           if (_scrollBarPanFinishedEventHandler == null)
           {
              _scrollBarPanFinishedEventHandler += value;

              _scrollBarPanFinishedEventCallbackDelegate = new PanFinishedEventCallbackDelegate(OnScrollBarPanFinished);
              this.PanFinishedSignal().Connect(_scrollBarPanFinishedEventCallbackDelegate);
           }
        }
     }

     remove
     {
        lock(this)
        {
           if (_scrollBarPanFinishedEventHandler != null)
           {
              this.PanFinishedSignal().Disconnect(_scrollBarPanFinishedEventCallbackDelegate);
           }

           _scrollBarPanFinishedEventHandler -= value;
        }
     }
  }

  // Callback for ScrollBar PanFinishedSignal
  private void OnScrollBarPanFinished()
  {
     PanFinishedEventArgs e = new PanFinishedEventArgs();

     if (_scrollBarPanFinishedEventHandler != null)
     {
        //here we send all data to user event handlers
        _scrollBarPanFinishedEventHandler(this, e);
     }
  }


  public event DaliEventHandler<object,ScrollPositionIntervalReachedEventArgs> ScrollPositionIntervalReached
  {
     add
     {
        lock(this)
        {
           // Restricted to only one listener
           if (_scrollBarScrollPositionIntervalReachedEventHandler == null)
           {
              _scrollBarScrollPositionIntervalReachedEventHandler += value;

              _scrollBarScrollPositionIntervalReachedEventCallbackDelegate = new ScrollPositionIntervalReachedEventCallbackDelegate(OnScrollBarScrollPositionIntervalReached);
              this.ScrollPositionIntervalReachedSignal().Connect(_scrollBarScrollPositionIntervalReachedEventCallbackDelegate);
           }
        }
     }

     remove
     {
        lock(this)
        {
           if (_scrollBarScrollPositionIntervalReachedEventHandler != null)
           {
              this.ScrollPositionIntervalReachedSignal().Disconnect(_scrollBarScrollPositionIntervalReachedEventCallbackDelegate);
           }

           _scrollBarScrollPositionIntervalReachedEventHandler -= value;
        }
     }
  }

  // Callback for ScrollBar ScrollPositionIntervalReachedSignal
  private void OnScrollBarScrollPositionIntervalReached()
  {
     ScrollPositionIntervalReachedEventArgs e = new ScrollPositionIntervalReachedEventArgs();

     if (_scrollBarScrollPositionIntervalReachedEventHandler != null)
     {
        //here we send all data to user event handlers
        _scrollBarScrollPositionIntervalReachedEventHandler(this, e);
     }
  }


  public class Property : global::System.IDisposable {
    private global::System.Runtime.InteropServices.HandleRef swigCPtr;
    protected bool swigCMemOwn;
  
    internal Property(global::System.IntPtr cPtr, bool cMemoryOwn) {
      swigCMemOwn = cMemoryOwn;
      swigCPtr = new global::System.Runtime.InteropServices.HandleRef(this, cPtr);
    }
  
    internal static global::System.Runtime.InteropServices.HandleRef getCPtr(Property obj) {
      return (obj == null) ? new global::System.Runtime.InteropServices.HandleRef(null, global::System.IntPtr.Zero) : obj.swigCPtr;
    }
  
    ~Property() {
      Dispose();
    }
  
    public virtual void Dispose() {
      lock(this) {
        if (swigCPtr.Handle != global::System.IntPtr.Zero) {
          if (swigCMemOwn) {
            swigCMemOwn = false;
            NDalicPINVOKE.delete_ScrollBar_Property(swigCPtr);
          }
          swigCPtr = new global::System.Runtime.InteropServices.HandleRef(null, global::System.IntPtr.Zero);
        }
        global::System.GC.SuppressFinalize(this);
      }
    }
  
    public Property() : this(NDalicPINVOKE.new_ScrollBar_Property(), true) {
      if (NDalicPINVOKE.SWIGPendingException.Pending) throw NDalicPINVOKE.SWIGPendingException.Retrieve();
    }
  
    public static readonly int SCROLL_DIRECTION = NDalicPINVOKE.ScrollBar_Property_SCROLL_DIRECTION_get();
    public static readonly int INDICATOR_HEIGHT_POLICY = NDalicPINVOKE.ScrollBar_Property_INDICATOR_HEIGHT_POLICY_get();
    public static readonly int INDICATOR_FIXED_HEIGHT = NDalicPINVOKE.ScrollBar_Property_INDICATOR_FIXED_HEIGHT_get();
    public static readonly int INDICATOR_SHOW_DURATION = NDalicPINVOKE.ScrollBar_Property_INDICATOR_SHOW_DURATION_get();
    public static readonly int INDICATOR_HIDE_DURATION = NDalicPINVOKE.ScrollBar_Property_INDICATOR_HIDE_DURATION_get();
    public static readonly int SCROLL_POSITION_INTERVALS = NDalicPINVOKE.ScrollBar_Property_SCROLL_POSITION_INTERVALS_get();
    public static readonly int INDICATOR_MINIMUM_HEIGHT = NDalicPINVOKE.ScrollBar_Property_INDICATOR_MINIMUM_HEIGHT_get();
    public static readonly int INDICATOR_START_PADDING = NDalicPINVOKE.ScrollBar_Property_INDICATOR_START_PADDING_get();
    public static readonly int INDICATOR_END_PADDING = NDalicPINVOKE.ScrollBar_Property_INDICATOR_END_PADDING_get();
  
  }

  public ScrollBar (ScrollBar.Direction direction) : this (NDalicPINVOKE.ScrollBar_New__SWIG_0((int)direction), true) {
      if (NDalicPINVOKE.SWIGPendingException.Pending) throw NDalicPINVOKE.SWIGPendingException.Retrieve();

  }
  public ScrollBar () : this (NDalicPINVOKE.ScrollBar_New__SWIG_1(), true) {
      if (NDalicPINVOKE.SWIGPendingException.Pending) throw NDalicPINVOKE.SWIGPendingException.Retrieve();

  }
  public ScrollBar(ScrollBar scrollBar) : this(NDalicPINVOKE.new_ScrollBar__SWIG_1(ScrollBar.getCPtr(scrollBar)), true) {
    if (NDalicPINVOKE.SWIGPendingException.Pending) throw NDalicPINVOKE.SWIGPendingException.Retrieve();
  }

  public ScrollBar Assign(ScrollBar scrollBar) {
    ScrollBar ret = new ScrollBar(NDalicPINVOKE.ScrollBar_Assign(swigCPtr, ScrollBar.getCPtr(scrollBar)), false);
    if (NDalicPINVOKE.SWIGPendingException.Pending) throw NDalicPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public new static ScrollBar DownCast(BaseHandle handle) {
    ScrollBar ret = new ScrollBar(NDalicPINVOKE.ScrollBar_DownCast(BaseHandle.getCPtr(handle)), true);
    if (NDalicPINVOKE.SWIGPendingException.Pending) throw NDalicPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public void SetScrollPropertySource(Animatable handle, int propertyScrollPosition, int propertyMinScrollPosition, int propertyMaxScrollPosition, int propertyScrollContentSize) {
    NDalicPINVOKE.ScrollBar_SetScrollPropertySource(swigCPtr, Animatable.getCPtr(handle), propertyScrollPosition, propertyMinScrollPosition, propertyMaxScrollPosition, propertyScrollContentSize);
    if (NDalicPINVOKE.SWIGPendingException.Pending) throw NDalicPINVOKE.SWIGPendingException.Retrieve();
  }

  public void SetScrollIndicator(View indicator) {
    NDalicPINVOKE.ScrollBar_SetScrollIndicator(swigCPtr, View.getCPtr(indicator));
    if (NDalicPINVOKE.SWIGPendingException.Pending) throw NDalicPINVOKE.SWIGPendingException.Retrieve();
  }

  public View GetScrollIndicator() {
    View ret = new View(NDalicPINVOKE.ScrollBar_GetScrollIndicator(swigCPtr), true);
    if (NDalicPINVOKE.SWIGPendingException.Pending) throw NDalicPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public void SetScrollPositionIntervals(VectorFloat positions) {
    NDalicPINVOKE.ScrollBar_SetScrollPositionIntervals(swigCPtr, VectorFloat.getCPtr(positions));
    if (NDalicPINVOKE.SWIGPendingException.Pending) throw NDalicPINVOKE.SWIGPendingException.Retrieve();
  }

  public VectorFloat GetScrollPositionIntervals() {
    VectorFloat ret = new VectorFloat(NDalicPINVOKE.ScrollBar_GetScrollPositionIntervals(swigCPtr), true);
    if (NDalicPINVOKE.SWIGPendingException.Pending) throw NDalicPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public void SetScrollDirection(ScrollBar.Direction direction) {
    NDalicPINVOKE.ScrollBar_SetScrollDirection(swigCPtr, (int)direction);
    if (NDalicPINVOKE.SWIGPendingException.Pending) throw NDalicPINVOKE.SWIGPendingException.Retrieve();
  }

  public ScrollBar.Direction GetScrollDirection() {
    ScrollBar.Direction ret = (ScrollBar.Direction)NDalicPINVOKE.ScrollBar_GetScrollDirection(swigCPtr);
    if (NDalicPINVOKE.SWIGPendingException.Pending) throw NDalicPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public void SetIndicatorHeightPolicy(ScrollBar.IndicatorHeightPolicyType policy) {
    NDalicPINVOKE.ScrollBar_SetIndicatorHeightPolicy(swigCPtr, (int)policy);
    if (NDalicPINVOKE.SWIGPendingException.Pending) throw NDalicPINVOKE.SWIGPendingException.Retrieve();
  }

  public ScrollBar.IndicatorHeightPolicyType GetIndicatorHeightPolicy() {
    ScrollBar.IndicatorHeightPolicyType ret = (ScrollBar.IndicatorHeightPolicyType)NDalicPINVOKE.ScrollBar_GetIndicatorHeightPolicy(swigCPtr);
    if (NDalicPINVOKE.SWIGPendingException.Pending) throw NDalicPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public void SetIndicatorFixedHeight(float height) {
    NDalicPINVOKE.ScrollBar_SetIndicatorFixedHeight(swigCPtr, height);
    if (NDalicPINVOKE.SWIGPendingException.Pending) throw NDalicPINVOKE.SWIGPendingException.Retrieve();
  }

  public float GetIndicatorFixedHeight() {
    float ret = NDalicPINVOKE.ScrollBar_GetIndicatorFixedHeight(swigCPtr);
    if (NDalicPINVOKE.SWIGPendingException.Pending) throw NDalicPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public void SetIndicatorShowDuration(float durationSeconds) {
    NDalicPINVOKE.ScrollBar_SetIndicatorShowDuration(swigCPtr, durationSeconds);
    if (NDalicPINVOKE.SWIGPendingException.Pending) throw NDalicPINVOKE.SWIGPendingException.Retrieve();
  }

  public float GetIndicatorShowDuration() {
    float ret = NDalicPINVOKE.ScrollBar_GetIndicatorShowDuration(swigCPtr);
    if (NDalicPINVOKE.SWIGPendingException.Pending) throw NDalicPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public void SetIndicatorHideDuration(float durationSeconds) {
    NDalicPINVOKE.ScrollBar_SetIndicatorHideDuration(swigCPtr, durationSeconds);
    if (NDalicPINVOKE.SWIGPendingException.Pending) throw NDalicPINVOKE.SWIGPendingException.Retrieve();
  }

  public float GetIndicatorHideDuration() {
    float ret = NDalicPINVOKE.ScrollBar_GetIndicatorHideDuration(swigCPtr);
    if (NDalicPINVOKE.SWIGPendingException.Pending) throw NDalicPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public void ShowIndicator() {
    NDalicPINVOKE.ScrollBar_ShowIndicator(swigCPtr);
    if (NDalicPINVOKE.SWIGPendingException.Pending) throw NDalicPINVOKE.SWIGPendingException.Retrieve();
  }

  public void HideIndicator() {
    NDalicPINVOKE.ScrollBar_HideIndicator(swigCPtr);
    if (NDalicPINVOKE.SWIGPendingException.Pending) throw NDalicPINVOKE.SWIGPendingException.Retrieve();
  }

  public VoidSignal PanFinishedSignal() {
    VoidSignal ret = new VoidSignal(NDalicPINVOKE.ScrollBar_PanFinishedSignal(swigCPtr), false);
    if (NDalicPINVOKE.SWIGPendingException.Pending) throw NDalicPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public FloatSignal ScrollPositionIntervalReachedSignal() {
    FloatSignal ret = new FloatSignal(NDalicPINVOKE.ScrollBar_ScrollPositionIntervalReachedSignal(swigCPtr), false);
    if (NDalicPINVOKE.SWIGPendingException.Pending) throw NDalicPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public enum PropertyRange {
    PROPERTY_START_INDEX = PropertyRanges.PROPERTY_REGISTRATION_START_INDEX,
    PROPERTY_END_INDEX = View.PropertyRange.PROPERTY_START_INDEX+1000
  }

  public enum Direction {
    Vertical = 0,
    Horizontal
  }

  public enum IndicatorHeightPolicyType {
    Variable = 0,
    Fixed
  }

  public string ScrollDirection
  {
    get
    {
      string temp;
      GetProperty( ScrollBar.Property.SCROLL_DIRECTION).Get( out temp );
      return temp;
    }
    set
    {
      SetProperty( ScrollBar.Property.SCROLL_DIRECTION, new Dali.Property.Value( value ) );
    }
  }
  public string IndicatorHeightPolicy
  {
    get
    {
      string temp;
      GetProperty( ScrollBar.Property.INDICATOR_HEIGHT_POLICY).Get( out temp );
      return temp;
    }
    set
    {
      SetProperty( ScrollBar.Property.INDICATOR_HEIGHT_POLICY, new Dali.Property.Value( value ) );
    }
  }
  public float IndicatorFixedHeight
  {
    get
    {
      float temp = 0.0f;
      GetProperty( ScrollBar.Property.INDICATOR_FIXED_HEIGHT).Get( ref temp );
      return temp;
    }
    set
    {
      SetProperty( ScrollBar.Property.INDICATOR_FIXED_HEIGHT, new Dali.Property.Value( value ) );
    }
  }
  public float IndicatorShowDuration
  {
    get
    {
      float temp = 0.0f;
      GetProperty( ScrollBar.Property.INDICATOR_SHOW_DURATION).Get( ref temp );
      return temp;
    }
    set
    {
      SetProperty( ScrollBar.Property.INDICATOR_SHOW_DURATION, new Dali.Property.Value( value ) );
    }
  }
  public float IndicatorHideDuration
  {
    get
    {
      float temp = 0.0f;
      GetProperty( ScrollBar.Property.INDICATOR_HIDE_DURATION).Get( ref temp );
      return temp;
    }
    set
    {
      SetProperty( ScrollBar.Property.INDICATOR_HIDE_DURATION, new Dali.Property.Value( value ) );
    }
  }
  public Dali.Property.Array ScrollPositionIntervals
  {
    get
    {
      Dali.Property.Array temp = new Dali.Property.Array();
      GetProperty( ScrollBar.Property.SCROLL_POSITION_INTERVALS).Get(  temp );
      return temp;
    }
    set
    {
      SetProperty( ScrollBar.Property.SCROLL_POSITION_INTERVALS, new Dali.Property.Value( value ) );
    }
  }
  public float IndicatorMinimumHeight
  {
    get
    {
      float temp = 0.0f;
      GetProperty( ScrollBar.Property.INDICATOR_MINIMUM_HEIGHT).Get( ref temp );
      return temp;
    }
    set
    {
      SetProperty( ScrollBar.Property.INDICATOR_MINIMUM_HEIGHT, new Dali.Property.Value( value ) );
    }
  }
  public float IndicatorStartPadding
  {
    get
    {
      float temp = 0.0f;
      GetProperty( ScrollBar.Property.INDICATOR_START_PADDING).Get( ref temp );
      return temp;
    }
    set
    {
      SetProperty( ScrollBar.Property.INDICATOR_START_PADDING, new Dali.Property.Value( value ) );
    }
  }
  public float IndicatorEndPadding
  {
    get
    {
      float temp = 0.0f;
      GetProperty( ScrollBar.Property.INDICATOR_END_PADDING).Get( ref temp );
      return temp;
    }
    set
    {
      SetProperty( ScrollBar.Property.INDICATOR_END_PADDING, new Dali.Property.Value( value ) );
    }
  }

}

}
