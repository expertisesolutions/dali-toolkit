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

public class BaseObject : RefObject {
  private global::System.Runtime.InteropServices.HandleRef swigCPtr;

  internal BaseObject(global::System.IntPtr cPtr, bool cMemoryOwn) : base(NDalicPINVOKE.BaseObject_SWIGUpcast(cPtr), cMemoryOwn) {
    swigCPtr = new global::System.Runtime.InteropServices.HandleRef(this, cPtr);
  }

  internal static global::System.Runtime.InteropServices.HandleRef getCPtr(BaseObject obj) {
    return (obj == null) ? new global::System.Runtime.InteropServices.HandleRef(null, global::System.IntPtr.Zero) : obj.swigCPtr;
  }

  public override void Dispose() {
    lock(this) {
      if (swigCPtr.Handle != global::System.IntPtr.Zero) {
        if (swigCMemOwn) {
          swigCMemOwn = false;
          throw new global::System.MethodAccessException("C++ destructor does not have public access");
        }
        swigCPtr = new global::System.Runtime.InteropServices.HandleRef(null, global::System.IntPtr.Zero);
      }
      global::System.GC.SuppressFinalize(this);
      base.Dispose();
    }
  }

  public bool DoAction(string actionName, Property.Map attributes) {
    bool ret = NDalicPINVOKE.BaseObject_DoAction(swigCPtr, actionName, Property.Map.getCPtr(attributes));
    if (NDalicPINVOKE.SWIGPendingException.Pending) throw NDalicPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public string GetTypeName() {
    string ret = NDalicPINVOKE.BaseObject_GetTypeName(swigCPtr);
    if (NDalicPINVOKE.SWIGPendingException.Pending) throw NDalicPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public bool GetTypeInfo(TypeInfo info) {
    bool ret = NDalicPINVOKE.BaseObject_GetTypeInfo(swigCPtr, TypeInfo.getCPtr(info));
    if (NDalicPINVOKE.SWIGPendingException.Pending) throw NDalicPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public bool DoConnectSignal(ConnectionTrackerInterface connectionTracker, string signalName, SWIGTYPE_p_FunctorDelegate functorDelegate) {
    bool ret = NDalicPINVOKE.BaseObject_DoConnectSignal(swigCPtr, ConnectionTrackerInterface.getCPtr(connectionTracker), signalName, SWIGTYPE_p_FunctorDelegate.getCPtr(functorDelegate));
    if (NDalicPINVOKE.SWIGPendingException.Pending) throw NDalicPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

}

}
