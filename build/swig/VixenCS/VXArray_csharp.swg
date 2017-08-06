/* -----------------------------------------------------------------------------
 * Array.swg
 *
 * SWIG typemaps for Array<T>
 * C# implementation
 * The C# wrapper is made to look and feel like a C# System.Collections.Generic.List<> collection.
 * For .NET 1 compatibility, define SWIG_DOTNET_1 when compiling the C# code; then the C# wrapper is 
 * made to look and feel like a typesafe C# System.Collections.ArrayList.
 *
 * The IList<> interface is also implemented to provide enhanced functionality
 * whenever we are confident that the required C++ operator== is available. This is the case for when 
 * T is a primitive type or a pointer.
 *
 * Warning: heavy macro usage in this file. Use swig -E to get a sane view on the real file contents!
 * ----------------------------------------------------------------------------- */

%define SWIG_VXARRAY_MINIMUM_INTERNAL(ARRAY, CSBASE, CELEMTYPE, CSELEMTYPE)
%newobject ARRAY::GetRange(int index, int count);
%newobject ARRAY::Repeat(CELEMTYPE const& value, int count);	
%csmethodmodifiers ARRAY::getitem "private"
%csmethodmodifiers ARRAY::setitem "private"
%csmethodmodifiers ARRAY::size "private"
%csmethodmodifiers ARRAY::capacity "private"
%csmethodmodifiers ARRAY::reserve "private"
%csmethodmodifiers ARRAY::GetAt "new public"
%ignore ARRAY::Opcode;

%apply	SharedObj*	{ ARRAY* }

%typemap(csinterfaces_derived) ARRAY "System.Collections.Generic.IList< CSELEMTYPE >, System.Collections.Specialized.INotifyCollectionChanged"

%typemap(cscode) ARRAY %{
	public $csclassname(System.Collections.ICollection c) : this()
	{
		if (c == null)
			throw new ArgumentNullException("c");
		foreach (CSELEMTYPE element in c)
			this.Append(element);
	}

    protected void OnCollectionChanged(System.Collections.Specialized.NotifyCollectionChangedEventArgs e)
    {
        if (CollectionChanged != null)
            CollectionChanged(this, e);
    }

	public event System.Collections.Specialized.NotifyCollectionChangedEventHandler CollectionChanged;
	public bool	IsFixedSize		{ get { return false; } }
	public bool	IsReadOnly		{ get { return false; } }
	public int	Count			{ get { return (int) GetSize(); } }
	public bool	IsSynchronized	{ get { return false; } }

	public CSELEMTYPE this[int index]
	{
		get { return getitem(index); }
		set { setitem(index, value); OnCollectionChanged(new System.Collections.Specialized.NotifyCollectionChangedEventArgs(System.Collections.Specialized.NotifyCollectionChangedAction.Replace, value, index)); }
	}

	public CSELEMTYPE this[long index]
	{
		get { return getitem(index); }
		set { setitem(index, value); OnCollectionChanged(new System.Collections.Specialized.NotifyCollectionChangedEventArgs(System.Collections.Specialized.NotifyCollectionChangedAction.Replace, value, index)); }
	}

	public int Capacity
	{
		get { return (int) GetMaxSize(); }
		set { SetMaxSize(value); }
	}

	public void Add(CSELEMTYPE v)
	{
		Append(v);
		OnCollectionChanged(new System.Collections.Specialized.NotifyCollectionChangedEventArgs(System.Collections.Specialized.NotifyCollectionChangedAction.Add, v));
	}

	public void Clear()
	{
		Empty();
		OnCollectionChanged(new System.Collections.Specialized.NotifyCollectionChangedEventArgs(System.Collections.Specialized.NotifyCollectionChangedAction.Reset));
	}

	public void CopyFrom(CSELEMTYPE[] srcarray)
		{ CopyFrom(srcarray, 0, 0); }

	public void CopyTo(CSELEMTYPE[] dstarray)
		{ CopyTo(0, dstarray, 0, this.Count); }

	public void CopyTo(CSELEMTYPE[] dstarray, int dstindex)
		{ CopyTo(0, dstarray, dstindex, this.Count); }

	public void CopyFrom(CSELEMTYPE[] srcarray, long dstindex)
		{ CopyFrom(srcarray, 0, dstindex); }

	public void CopyTo(long srcindex, CSELEMTYPE[] dstarray, long dstindex, long count)
	{
		if (dstarray == null)
			throw new ArgumentNullException("dstarray");
		if (srcindex < 0)
			throw new ArgumentOutOfRangeException("srcindex", "Value is less than zero");
		if (dstindex < 0)
			throw new ArgumentOutOfRangeException("dstindex", "Value is less than zero");
		if (count < 0)
			throw new ArgumentOutOfRangeException("count", "Value is less than zero");
		if (dstarray.Rank > 1)
			throw new ArgumentException("Multi dimensional array.", "dstarray");
		if (dstindex + count > dstarray.Length)
			throw new ArgumentException("Number of elements to copy is too large.");
		for (int i = 0; i < count; i++)
			dstarray[dstindex + i] = getitem(srcindex + i);
	}

	public void CopyFrom(CSELEMTYPE[] srcarray, long srcindex, long dstindex)
	{
		int count;

		if (srcarray == null)
			throw new ArgumentNullException("srcarray");
		if (dstindex < 0)
			throw new ArgumentOutOfRangeException("dstindex", "Value is less than zero");
		if (srcindex < 0)
			throw new ArgumentOutOfRangeException("srcindex", "Value is less than zero");
		if (srcarray.Rank > 1)
			throw new ArgumentException("Multi dimensional array.", "srcarray");
		count = srcarray.Length;
		for (int i = 0; i < count; i++)
			setitem(dstindex + i, srcarray[srcindex + i]);
		OnCollectionChanged(new System.Collections.Specialized.NotifyCollectionChangedEventArgs(System.Collections.Specialized.NotifyCollectionChangedAction.Replace, this));
	}

	System.Collections.Generic.IList<CSELEMTYPE> ToList()
	{
		return this as System.Collections.Generic.IList<CSELEMTYPE>;
	}

	System.Collections.Generic.IEnumerator<CSELEMTYPE> System.Collections.Generic.IEnumerable<CSELEMTYPE>.GetEnumerator()
	{
		return new $csclassnameEnumerator(this);
	}

	System.Collections.IEnumerator System.Collections.IEnumerable.GetEnumerator()
	{
		return new $csclassnameEnumerator(this);
	}

	public $csclassnameEnumerator GetEnumerator()
	{
		return new $csclassnameEnumerator(this);
	}

  // Type-safe enumerator
  /// Note that the IEnumerator documentation requires an InvalidOperationException to be thrown
  /// whenever the collection is modified. This has been done for changes in the size of the
  /// collection but not when one of the elements of the collection is modified as it is a bit
  /// tricky to detect unmanaged code that modifies the collection under our feet.
	public sealed class $csclassnameEnumerator : System.Collections.IEnumerator, System.Collections.Generic.IEnumerator<CSELEMTYPE>
	{
		private $csclassname collectionRef;
		private long currentIndex;
		private object currentObject;
		private long currentSize;

		public $csclassnameEnumerator($csclassname collection)
		{
			collectionRef = collection;
			currentIndex = -1;
			currentObject = null;
			currentSize = collectionRef.Count;
		}

		// Type-safe iterator Current
		public CSELEMTYPE Current
		{
			get
			{
				if (currentIndex == -1)
					throw new InvalidOperationException("Enumeration not started.");
				if (currentIndex > currentSize - 1)
					throw new InvalidOperationException("Enumeration finished.");
				if (currentObject == null)
					throw new InvalidOperationException("Collection modified.");
				return (CSELEMTYPE) currentObject;
			}
		}

		// Type-unsafe IEnumerator.Current
		object System.Collections.IEnumerator.Current { get { return Current; } }

		public bool MoveNext()
		{
			int size = collectionRef.Count;
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
			if (collectionRef.GetSize() != currentSize)
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

%extend ARRAY
{
	CELEMTYPE getitem(long long index) throw (std::out_of_range)
	{
		long long size = $self->GetSize();
		if ((index >= 0) && (index < size))
			return $self->GetAt(index);
		throw std::out_of_range("index");
	}

	void setitem(long long index, CELEMTYPE const& val) throw (std::out_of_range)
	{
		if (!$self->SetAt(index, val))
			throw std::out_of_range("index");
	}

	void AddRange(const Array<CELEMTYPE>& values)
	{
		$self->Merge(values);
	}

    // Takes a deep copy of the elements unlike ArrayList.GetRange
	ARRAY* GetRange(long index, long count) throw (std::out_of_range, std::invalid_argument)
	{
		if (index < 0)
			throw std::out_of_range("index");
		if (count < 0)
			throw std::out_of_range("count");
		if ((index >= $self->GetSize() + 1) ||
			(index + count) > $self->GetSize())
			throw std::invalid_argument("invalid range");
		ARRAY* temp = new ARRAY(count);

		for (int i = 0; i < count; ++i)
			temp->SetAt(i, $self->GetAt(index + i));
		return temp;
	}

	void Insert(int index, CELEMTYPE const& x) throw (std::out_of_range)
	{
		throw std::out_of_range("index");
	}

	// Takes a deep copy of the elements unlike ArrayList.InsertRange
	void InsertRange(int index, const Array<CELEMTYPE>& values) throw (std::out_of_range)
	{
		throw std::out_of_range("index");
	}

	void RemoveAt(int index) throw (std::out_of_range)
	{
		if ((index < 0) || (index >= $self->GetSize()))
			throw std::out_of_range("index");
		$self->RemoveAt(index);
	}
	
	void RemoveElem(int index) throw (std::out_of_range)
	{
		if ((index < 0) || (index >= $self->GetSize()))
			throw std::out_of_range("index");
		$self->RemoveAt(index);
	}

	void RemoveRange(int index, int count) throw (std::out_of_range, std::invalid_argument)
	{
		if (index < 0)
			throw std::out_of_range("index");
		if (count < 0)
			throw std::out_of_range("count");
		if (index + count > $self->GetSize())
			throw std::invalid_argument("invalid range");
		throw std::invalid_argument("invalid range");
	}

	static ARRAY* Repeat(CELEMTYPE const& value, int count) throw (std::out_of_range)
	{
		if (count < 0)
			throw std::out_of_range("count");
		ARRAY* temp = new ARRAY(count);

		for (int i = 0; i < count; ++i)
			temp->SetAt(i, value);
		return temp;
	}

	void Reverse(int index, int count) throw (std::out_of_range, std::invalid_argument)
	{
		if (index < 0)
			throw std::out_of_range("index");
		if (count < 0)
			throw std::out_of_range("count");
		if ((index >= $self->GetSize() + 1) || (index + count > $self->GetSize()))
			throw std::invalid_argument("invalid range");
		intptr start, end;
		for (start = index, end = index + count - 1; start < end; ++start, --end)
		{
			CELEMTYPE temp = $self->GetAt(start);
			$self->SetAt(start, $self->GetAt(end));
			$self->SetAt(end, temp);
		}
    }

    // Takes a deep copy of the elements unlike ArrayList.SetRange
    void SetRange(long index, const Array<CELEMTYPE>& values) throw (std::out_of_range)
	{
	if (index < 0)
		throw std::out_of_range("index");
	if ((index + values.GetSize()) > $self->GetSize())
		throw std::out_of_range("index");
	for (intptr i = 0; i < values.GetSize(); ++i)
		$self->SetAt(index + i, values.GetAt(i));
	}
}
%enddef

// Extra methods added to the collection class if operator== is defined for the class being wrapped
// The class will then implement IList<>, which adds extra functionality
%define SWIG_VXARRAY_EXTRA_OP_EQUALS_EQUALS(ARRAY, CELEMTYPE)
    %extend ARRAY
	{
		bool	Contains(CELEMTYPE const& value)	{ return ($self->Find(value) >= 0); }

		int		IndexOf(CELEMTYPE const& value) throw (std::out_of_range)
		{
			intptr i = $self->Find(value);
			if (i >= INT_MAX)
				throw std::out_of_range("IndexOf");
			return (int) i;
		}

		int		LastIndexOf(CELEMTYPE const& value) throw (std::out_of_range)
		{
			intptr n = $self->GetSize();
			for (intptr i = n - 1; i >= 0; --i)
				if ($self->GetAt(i) == value)
				{
					if (i >= INT_MAX)
						throw std::out_of_range("LastIndexOf");
					return (int) i;
				}
			return -1;
		}

		bool Remove(CELEMTYPE const& value)
		{
			intptr index = $self->Find(value);
			if (index < 0)
				return false;
			$self->RemoveAt(index);
			return true;
		}
	}
%enddef

// Macros for Array class specializations/enhancements
%define SWIG_VXARRAY_ENHANCED(ARRAY, BASE, CELEMTYPE, CSELEMTYPE)
	SWIG_VXARRAY_MINIMUM_INTERNAL(%arg(ARRAY), %arg(BASE), %arg(CELEMTYPE), %arg(CSELEMTYPE))
	SWIG_VXARRAY_EXTRA_OP_EQUALS_EQUALS(%arg(ARRAY), %arg(CELEMTYPE))
%enddef

namespace Vixen
{
	SWIG_VXARRAY_ENHANCED(IntArray, SharedIntArray, int32, int)
	SWIG_VXARRAY_ENHANCED(FloatArray, SharedFloatArray, float, float)
	SWIG_VXARRAY_ENHANCED(ObjArray, SharedObjArray, ObjRef, ObjRef)
}


