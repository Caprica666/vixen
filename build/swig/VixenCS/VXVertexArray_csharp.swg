/* -----------------------------------------------------------------------------
 * VertexArray.swg
 *
 * SWIG typemaps for VertesPool
 * C# implementation
 * The C# wrapper is made to look and feel like a C# System.Collections.Generic.List<> collection.
 * The IList<> interface is also implemented.
 *
 * ----------------------------------------------------------------------------- */
//%newobject VertexArray::getitem;	
%csmethodmodifiers VertexArray::getitem "private"
%csmethodmodifiers VertexArray::setitem "private"
%csmethodmodifiers VertexArray::size "private"
%csmethodmodifiers VertexArray::capacity "private"
%csmethodmodifiers VertexArray::reserve "private"

%typemap(csinterfaces_derived) VertexArray "System.Collections.IEnumerable, System.Collections.Generic.IList< float[] >"

%typemap(cscode) VertexArray %{
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
%}

%extend VertexArray
{
	void getitem(long long index, float* dstArray) throw (std::out_of_range)
	{
		VertexPool::ConstIter iter($self);
		const float* srcArray = iter.GetVtx(index);

		if (srcArray == NULL)
			throw std::out_of_range("index");
		memcpy(dstArray, srcArray, iter.GetVtxSize() * sizeof(float));
	}

	void setitem(long long index, const float* floatArray) throw (std::out_of_range)
	{
		if (index > $self->GetMaxVtx())
		{
			$self->SetMaxVtx(index + 1);
			$self->SetNumVtx(index + 1);
		}
		VertexPool::Iter iter($self);
		if (!iter.CopyVtx(index, floatArray))
			throw std::out_of_range("index");
	}

	int	LastIndexOf(const float* floatArray) throw (std::out_of_range)
	{
		VertexArray::ConstIter iter($self);
		intptr nverts = $self->GetNumVtx();

		if (nverts >= INT_MAX)
			throw std::out_of_range("IndexOf");
		for (int i = 0; i < (int) nverts; ++i)
		{
			if (iter.CompareVtx(i, floatArray))
				return i;
		}
		return -1;
	}

	int	IndexOf(const float* floatArray) throw (std::out_of_range)
	{
		VertexArray::ConstIter iter($self);
		intptr nverts = $self->GetNumVtx();

		if (nverts >= INT_MAX)
			throw std::out_of_range("IndexOf");
		for (int i = 0; i < (int) nverts; ++i)
		{
			if (iter.CompareVtx(i, floatArray))
				return i;
		}
		return -1;
	}

	static VertexArray* Repeat(const float* floatArray, int count) throw (std::invalid_argument)
	{
		throw std::invalid_argument("Repeat");
	}

	void Insert(int index, float* floatArray) throw (std::invalid_argument)
	{
		throw std::invalid_argument("Insert");
	}

	void InsertRange(int index, const float* floatArray) throw (std::invalid_argument)
	{
		throw std::invalid_argument("InsertRange");
	}

	void RemoveElem(int index) throw (std::invalid_argument)
	{
		throw std::invalid_argument("RemoveElem");
	}

	void RemoveAt(int index) throw (std::invalid_argument)
	{
		throw std::invalid_argument("RemoveAt");
	}

	void RemoveRange(int index, int count) throw (std::invalid_argument)
	{
		throw std::invalid_argument("RemoveRange");
	}

	void Reverse(long index, long count) throw (std::invalid_argument)
	{
		throw std::invalid_argument("Reverse");
    }

	bool Remove(const float* floatArray) throw (std::invalid_argument)
	{
		throw std::invalid_argument("Remove");
	}
}