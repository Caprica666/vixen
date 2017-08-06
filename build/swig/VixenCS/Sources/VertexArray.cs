/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 2.0.1
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

namespace Vixen {

using System;
using System.Runtime.InteropServices;

public class VertexArray : VertexPool, System.Collections.IEnumerable, System.Collections.Generic.IList< float[] > {
  private HandleRef swigCPtr;

  internal VertexArray(IntPtr cPtr, bool cMemoryOwn) : base(VixenLibPINVOKE.VertexArray_SWIGUpcast(cPtr), cMemoryOwn) {
    swigCPtr = new HandleRef(this, cPtr);
  }

  internal static HandleRef getCPtr(VertexArray obj) {
    return (obj == null) ? new HandleRef(null, IntPtr.Zero) : obj.swigCPtr;
  }

  ~VertexArray() {
    Dispose();
  }

  public override void Dispose() {
    lock(this) {
      if (swigCPtr.Handle != IntPtr.Zero) {
        if (swigCMemOwn) {
          swigCMemOwn = false;
          VixenLibPINVOKE.delete_VertexArray(swigCPtr);
        }
        swigCPtr = new HandleRef(null, IntPtr.Zero);
      }
      GC.SuppressFinalize(this);
      base.Dispose();
    }
  }

	public VertexArray(System.Collections.ICollection collection) : this()
	{
		if (collection == null)
			throw new ArgumentNullException("collection");
		foreach (float[] element in collection)
			this.Add(element);
	}

	public bool	IsFixedSize		{ get { return false; } }
	public bool	IsReadOnly		{ get { return false; } }
	public bool	IsSynchronized	{ get { return false; } }
	public int Capacity			{ get { return (int) MaxVerts; } set { MaxVerts = value; }}
	public int Count			{ get { return (int) VertexCount; } set { VertexCount = value; }}


	public float[] this[long index]
	{
		get
		{
			float[] tmp = new float[VertexSize];
			getitem(index, tmp);
			return tmp;
		}
		set { setitem(index, value); }
	}

	public float[] this[int index]
	{
		get
		{
			float[] tmp = new float[VertexSize];
			getitem(index, tmp);
			return tmp;
		}
		set { setitem(index, value); }
	}
	
	public void Add(float[] v)
		{	AddVertices(v, 1); }

	public void Clear()
		{	Capacity = 0; }

	public void CopyFrom(float[] floatArray)
		{ CopyFrom(floatArray, 0, 0, floatArray.Length / VertexSize); }

	public void CopyTo(float[] floatArray)
		{ CopyTo(0L, floatArray, 0L, VertexCount); }

	public void CopyTo(float[] floatArray, long i)
		{ CopyTo(0L, floatArray, i, VertexCount); }

	public void CopyFrom(float[] floatArray, long i)
		{ CopyFrom(floatArray, i, 0, floatArray.Length / VertexSize); }

	public void CopyTo(long srcindex, float[] dstArray, long dstindex, long numverts)
	{
		if (dstArray == null)
			throw new ArgumentNullException("dstArray");
		if (srcindex < 0)
			throw new ArgumentOutOfRangeException("srcindex", "Value is less than zero");
		if (dstindex < 0)
			throw new ArgumentOutOfRangeException("dstArray", "Value is less than zero");
		if (numverts < 0)
			throw new ArgumentOutOfRangeException("numverts", "Value is less than zero");
		if (dstArray.Rank > 1)
			throw new ArgumentException("Multi dimensional dstArray.", "dstArray");
		if (srcindex + numverts > this.Count || ((dstindex + numverts) * this.VertexSize) > dstArray.Length)
			throw new ArgumentException("Number of elements to copy is too large.");
		float[] vertex = new float[VertexSize];
		for (long i = 0; i < numverts; i++)
		{
			getitem(srcindex + i, vertex);
			Array.Copy(vertex, 0, dstArray, dstindex + i, VertexSize);
		}
	}

	public void CopyTo(float[][] dstArray, int dstindex)
	{
		int dstverts;

		if (dstArray == null)
			throw new ArgumentNullException("dstArray");
		if (dstindex < 0)
			throw new ArgumentOutOfRangeException("dstArray", "Value is less than zero");
		if (dstArray.Rank != 2)
			throw new ArgumentException("Need two dimensional dstArray.", "dstArray");
		if (dstArray.GetLength(1) != VertexSize)
			throw new ArgumentException("dstArray incompatible with vertex size", "dstArray");
		dstverts = dstArray.GetLength(0) - dstindex;
		if (dstverts < this.Count)
			throw new ArgumentException("Number of elements to copy is too large.");
		for (int i = 0; i < dstverts; i++)
		{
			float[] vertex = dstArray[dstindex + i];
			getitem(i, vertex);
		}
	}

	public void CopyFrom(float[] srcArray, long srcindex, int dstindex, int count)
	{
		if (srcArray == null)
			throw new ArgumentNullException("srcArray");
		if (dstindex < 0)
			throw new ArgumentOutOfRangeException("dstindex", "Value is less than zero");
		if (srcindex < 0)
			throw new ArgumentOutOfRangeException("srcindex", "Value is less than zero");
		if (count < 0)
			throw new ArgumentOutOfRangeException("count", "Value is less than zero");
		if (srcArray.Rank > 1)
			throw new ArgumentException("Multi dimensional srcArray.", "srcArray");
		if ((count * VertexSize - srcindex) > srcArray.Length)
			throw new ArgumentOutOfRangeException("count", "Value is larger that source array length");
		if (dstindex + count > this.MaxVerts)
			this.MaxVerts = dstindex + count;
		for (long i = 0; i < count; i++)
		{
			float[] tmp = new float[this.VertexSize];

			for (int j = 0; j < this.VertexSize; ++j)
				tmp[j] = srcArray[srcindex + j];
			setitem(dstindex + i, tmp);
		}
	}

	public float[] GetRange(long index, long count)
	{
		float[] temp = new float[count * this.VertexSize];

		CopyTo(index, temp, 0, count);
		return temp;
	}

	public void SetRange(long index, float[] srcArray)
	{
		CopyFrom(srcArray, 0, 0, srcArray.Length / VertexSize);
	}
	
	public void AddRange(float[] srcArray)
	{
		AddVertices(srcArray, srcArray.Length / VertexSize);
	}

	public bool	Contains(float[] srcArray)
	{
		return (IndexOf(srcArray) >= 0);
	}

	System.Collections.Generic.IEnumerator<float[]> System.Collections.Generic.IEnumerable<float[]>.GetEnumerator()
	{
		return new VertexEnumerator(this);
	}

	System.Collections.IEnumerator System.Collections.IEnumerable.GetEnumerator()
	{
		return new VertexEnumerator(this);
	}

	public VertexEnumerator GetEnumerator()
	{
		return new VertexEnumerator(this);
	}

  // Type-safe enumerator
  /// Note that the IEnumerator documentation requires an InvalidOperationException to be thrown
  /// whenever the collection is modified. This has been done for changes in the size of the
  /// collection but not when one of the elements of the collection is modified as it is a bit
  /// tricky to detect unmanaged code that modifies the collection under our feet.
	public sealed class VertexEnumerator : System.Collections.IEnumerator, System.Collections.Generic.IEnumerator<float[]>
	{
		private VertexArray collectionRef;
		private long currentIndex;
		private object currentObject;
		private long currentSize;

		public VertexEnumerator(VertexArray collection)
		{
			collectionRef = collection;
			currentIndex = -1;
			currentObject = null;
			currentSize = collectionRef.Count;
		}

		// Type-safe iterator Current
		public float[] Current
		{
			get
			{
				if (currentIndex == -1)
					throw new InvalidOperationException("Enumeration not started.");
				if (currentIndex > currentSize - 1)
					throw new InvalidOperationException("Enumeration finished.");
				if (currentObject == null)
					throw new InvalidOperationException("Collection modified.");
				return (float[])currentObject;
			}
		}

		// Type-unsafe IEnumerator.Current
		object System.Collections.IEnumerator.Current { get { return Current; } }

		public bool MoveNext()
		{
			long size = collectionRef.Count;
			bool moveOkay = (currentIndex+1 < size) && (size == currentSize);
			if (moveOkay)
			{
				currentIndex++;
				currentObject = collectionRef[currentIndex];
			}
			else
				currentObject = null;
		  return moveOkay;
		}

		public void Reset()
		{
			currentIndex = -1;
			currentObject = null;
			if (collectionRef.VertexCount != currentSize)
				throw new InvalidOperationException("Collection modified.");
		}

		public void Dispose()
		{
			currentIndex = -1;
			currentObject = null;
		}
		
		public void Reverse()
		{
			collectionRef.Reverse(0, (int) currentSize);
		}
	}

  public VertexArray(int style, long size) : this(VixenLibPINVOKE.new_VertexArray__SWIG_0(style, size), true) {
  }

  public VertexArray(int style) : this(VixenLibPINVOKE.new_VertexArray__SWIG_1(style), true) {
  }

  public VertexArray() : this(VixenLibPINVOKE.new_VertexArray__SWIG_2(), true) {
  }

  public VertexArray(string layout_desc, long size) : this(VixenLibPINVOKE.new_VertexArray__SWIG_3(layout_desc, size), true) {
  }

  public VertexArray(string layout_desc) : this(VixenLibPINVOKE.new_VertexArray__SWIG_4(layout_desc), true) {
  }

  public VertexArray(VertexArray arg0) : this(VixenLibPINVOKE.new_VertexArray__SWIG_5(VertexArray.getCPtr(arg0)), true) {
    if (VixenLibPINVOKE.SWIGPendingException.Pending) throw VixenLibPINVOKE.SWIGPendingException.Retrieve();
  }

  public static BaseObj VertexAlloc {
    set {
      VixenLibPINVOKE.VertexArray_VertexAlloc_set(BaseObj.getCPtr(value));
    } 
    get {
      IntPtr cPtr = VixenLibPINVOKE.VertexArray_VertexAlloc_get();
      BaseObj ret = (cPtr == IntPtr.Zero) ? null : new BaseObj(cPtr, false);
      return ret;
    } 
  }

  private void getitem(long index, float[] dstArray) {
    VixenLibPINVOKE.VertexArray_getitem(swigCPtr, index, dstArray);
    if (VixenLibPINVOKE.SWIGPendingException.Pending) throw VixenLibPINVOKE.SWIGPendingException.Retrieve();
  }

  private void setitem(long index, float[] floatArray) {
    VixenLibPINVOKE.VertexArray_setitem(swigCPtr, index, floatArray);
    if (VixenLibPINVOKE.SWIGPendingException.Pending) throw VixenLibPINVOKE.SWIGPendingException.Retrieve();
  }

  public int LastIndexOf(float[] floatArray) {
    int ret = VixenLibPINVOKE.VertexArray_LastIndexOf(swigCPtr, floatArray);
    if (VixenLibPINVOKE.SWIGPendingException.Pending) throw VixenLibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int IndexOf(float[] floatArray) {
    int ret = VixenLibPINVOKE.VertexArray_IndexOf(swigCPtr, floatArray);
    if (VixenLibPINVOKE.SWIGPendingException.Pending) throw VixenLibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public static VertexArray Repeat(float[] floatArray, int count) 
	{
		IntPtr cPtr = VixenLibPINVOKE.VertexArray_Repeat(floatArray, count);
    if (VixenLibPINVOKE.SWIGPendingException.Pending) throw VixenLibPINVOKE.SWIGPendingException.Retrieve();
		return (VertexArray)  SharedObj.MakeObject(cPtr, true);
	}


  public void Insert(int index, float[] floatArray) {
    VixenLibPINVOKE.VertexArray_Insert(swigCPtr, index, floatArray);
    if (VixenLibPINVOKE.SWIGPendingException.Pending) throw VixenLibPINVOKE.SWIGPendingException.Retrieve();
  }

  public void InsertRange(int index, float[] floatArray) {
    VixenLibPINVOKE.VertexArray_InsertRange(swigCPtr, index, floatArray);
    if (VixenLibPINVOKE.SWIGPendingException.Pending) throw VixenLibPINVOKE.SWIGPendingException.Retrieve();
  }

  public void RemoveElem(int index) {
    VixenLibPINVOKE.VertexArray_RemoveElem(swigCPtr, index);
    if (VixenLibPINVOKE.SWIGPendingException.Pending) throw VixenLibPINVOKE.SWIGPendingException.Retrieve();
  }

  public void RemoveAt(int index) {
    VixenLibPINVOKE.VertexArray_RemoveAt(swigCPtr, index);
    if (VixenLibPINVOKE.SWIGPendingException.Pending) throw VixenLibPINVOKE.SWIGPendingException.Retrieve();
  }

  public void RemoveRange(int index, int count) {
    VixenLibPINVOKE.VertexArray_RemoveRange(swigCPtr, index, count);
    if (VixenLibPINVOKE.SWIGPendingException.Pending) throw VixenLibPINVOKE.SWIGPendingException.Retrieve();
  }

  public void Reverse(int index, int count) {
    VixenLibPINVOKE.VertexArray_Reverse(swigCPtr, index, count);
    if (VixenLibPINVOKE.SWIGPendingException.Pending) throw VixenLibPINVOKE.SWIGPendingException.Retrieve();
  }

  public bool Remove(float[] floatArray) {
    bool ret = VixenLibPINVOKE.VertexArray_Remove(swigCPtr, floatArray);
    if (VixenLibPINVOKE.SWIGPendingException.Pending) throw VixenLibPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int Style {
    set {
      VixenLibPINVOKE.VertexArray_Style_set(swigCPtr, value);
    } 
    get {
      int ret = VixenLibPINVOKE.VertexArray_Style_get(swigCPtr);
      return ret;
    } 
  }

  public int VertexSize {
    set {
      VixenLibPINVOKE.VertexArray_VertexSize_set(swigCPtr, value);
    } 
    get {
      int ret = VixenLibPINVOKE.VertexArray_VertexSize_get(swigCPtr);
      return ret;
    } 
  }

  public long VertexCount {
    set {
      VixenLibPINVOKE.VertexArray_VertexCount_set(swigCPtr, value);
    } 
    get {
      long ret = VixenLibPINVOKE.VertexArray_VertexCount_get(swigCPtr);
      return ret;
    } 
  }

  public long MaxVerts {
    set {
      VixenLibPINVOKE.VertexArray_MaxVerts_set(swigCPtr, value);
    } 
    get {
      long ret = VixenLibPINVOKE.VertexArray_MaxVerts_get(swigCPtr);
      return ret;
    } 
  }

}

}