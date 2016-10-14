//------------------------------------------------------------------------------
// <auto-generated />
//
// This file was automatically generated by SWIG (http://www.swig.org).
// Version 3.0.9
//
// Do not make changes to this file unless you know what you are doing--modify
// the SWIG interface file instead.
//------------------------------------------------------------------------------

namespace Dali {

public class TapGesture : Gesture {
  private global::System.Runtime.InteropServices.HandleRef swigCPtr;

  internal TapGesture(global::System.IntPtr cPtr, bool cMemoryOwn) : base(NDalicPINVOKE.TapGesture_SWIGUpcast(cPtr), cMemoryOwn) {
    swigCPtr = new global::System.Runtime.InteropServices.HandleRef(this, cPtr);
  }

  internal static global::System.Runtime.InteropServices.HandleRef getCPtr(TapGesture obj) {
    return (obj == null) ? new global::System.Runtime.InteropServices.HandleRef(null, global::System.IntPtr.Zero) : obj.swigCPtr;
  }

  ~TapGesture() {
    Dispose();
  }

  public override void Dispose() {
    lock(this) {
      if (swigCPtr.Handle != global::System.IntPtr.Zero) {
        if (swigCMemOwn) {
          swigCMemOwn = false;
          NDalicPINVOKE.delete_TapGesture(swigCPtr);
        }
        swigCPtr = new global::System.Runtime.InteropServices.HandleRef(null, global::System.IntPtr.Zero);
      }
      global::System.GC.SuppressFinalize(this);
      base.Dispose();
    }
  }

 public static TapGesture GetTapGestureFromPtr(global::System.IntPtr cPtr) {
    TapGesture ret = new TapGesture(cPtr, false);
	if (NDalicPINVOKE.SWIGPendingException.Pending) throw NDalicPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public TapGesture() : this(NDalicPINVOKE.new_TapGesture__SWIG_0(), true) {
    if (NDalicPINVOKE.SWIGPendingException.Pending) throw NDalicPINVOKE.SWIGPendingException.Retrieve();
  }

  public TapGesture(TapGesture rhs) : this(NDalicPINVOKE.new_TapGesture__SWIG_1(TapGesture.getCPtr(rhs)), true) {
    if (NDalicPINVOKE.SWIGPendingException.Pending) throw NDalicPINVOKE.SWIGPendingException.Retrieve();
  }

  public TapGesture Assign(TapGesture rhs) {
    TapGesture ret = new TapGesture(NDalicPINVOKE.TapGesture_Assign(swigCPtr, TapGesture.getCPtr(rhs)), false);
    if (NDalicPINVOKE.SWIGPendingException.Pending) throw NDalicPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public uint numberOfTaps {
    set {
      NDalicPINVOKE.TapGesture_numberOfTaps_set(swigCPtr, value);
      if (NDalicPINVOKE.SWIGPendingException.Pending) throw NDalicPINVOKE.SWIGPendingException.Retrieve();
    } 
    get {
      uint ret = NDalicPINVOKE.TapGesture_numberOfTaps_get(swigCPtr);
      if (NDalicPINVOKE.SWIGPendingException.Pending) throw NDalicPINVOKE.SWIGPendingException.Retrieve();
      return ret;
    } 
  }

  public uint numberOfTouches {
    set {
      NDalicPINVOKE.TapGesture_numberOfTouches_set(swigCPtr, value);
      if (NDalicPINVOKE.SWIGPendingException.Pending) throw NDalicPINVOKE.SWIGPendingException.Retrieve();
    } 
    get {
      uint ret = NDalicPINVOKE.TapGesture_numberOfTouches_get(swigCPtr);
      if (NDalicPINVOKE.SWIGPendingException.Pending) throw NDalicPINVOKE.SWIGPendingException.Retrieve();
      return ret;
    } 
  }

  public Vector2 screenPoint {
    set {
      NDalicPINVOKE.TapGesture_screenPoint_set(swigCPtr, Vector2.getCPtr(value));
      if (NDalicPINVOKE.SWIGPendingException.Pending) throw NDalicPINVOKE.SWIGPendingException.Retrieve();
    } 
    get {
      global::System.IntPtr cPtr = NDalicPINVOKE.TapGesture_screenPoint_get(swigCPtr);
      Vector2 ret = (cPtr == global::System.IntPtr.Zero) ? null : new Vector2(cPtr, false);
      if (NDalicPINVOKE.SWIGPendingException.Pending) throw NDalicPINVOKE.SWIGPendingException.Retrieve();
      return ret;
    } 
  }

  public Vector2 localPoint {
    set {
      NDalicPINVOKE.TapGesture_localPoint_set(swigCPtr, Vector2.getCPtr(value));
      if (NDalicPINVOKE.SWIGPendingException.Pending) throw NDalicPINVOKE.SWIGPendingException.Retrieve();
    } 
    get {
      global::System.IntPtr cPtr = NDalicPINVOKE.TapGesture_localPoint_get(swigCPtr);
      Vector2 ret = (cPtr == global::System.IntPtr.Zero) ? null : new Vector2(cPtr, false);
      if (NDalicPINVOKE.SWIGPendingException.Pending) throw NDalicPINVOKE.SWIGPendingException.Retrieve();
      return ret;
    } 
  }

}

}
