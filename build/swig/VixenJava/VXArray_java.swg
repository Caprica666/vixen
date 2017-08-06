/* -----------------------------------------------------------------------------
 * Array.swg
 *
 * SWIG typemaps for Array<T>
 * Java implementation
 * The Java wrapper is made to look and feel like a Java java.util.ArrayList<> collection.
 *
 * The List<> interface is also implemented to provide enhanced functionality
 * whenever we are confident that the required C++ operator== is available. This is the case for when 
 * T is a primitive type or a pointer.
 *
 * Warning: heavy macro usage in this file. Use swig -E to get a sane view on the real file contents!
 * ----------------------------------------------------------------------------- */


// MACRO for use within the	Array class body
%define SWIG_VXARRAY_MINIMUM_INTERNAL(CSINTERFACE, CONST_REFERENCE, ARRAY, CTYPE...)

%typemap(javainterfaces) ARRAY "IList\n";

%typemap(javacode) ARRAY
%{
	public $javaclassname(IList c) : this()
	{
		if (c == null)
			throw new ArgumentNullException("c");
		foreach ($typemap(cstype, CTYPE) element in c)
			this.Add(element);
	}

	public bool	IsFixedSize		{ get { return false; } }
	public bool	IsReadOnly		{ get { return false; } }
	public int	Count			{ get { return (int) GetSize(); } }
	public bool	IsSynchronized	{ get { return false; } }

	public $typemap(cstype, CTYPE) this[int index]
	{
		get { return getitem(index); }
		set { setitem(index, value); }
	}

	public int Capacity
	{
		get { return (int) GetMaxSize(); }
		set
		{
			if (value < GetSize())
				throw new ArgumentOutOfRangeException("Capacity");
			SetMaxSize((uint)value);
		}
	}


public void CopyTo($typemap(cstype, CTYPE)[] array)
	{ CopyTo(0, array, 0, this.Count); }

public void CopyTo($typemap(cstype, CTYPE)[] array, int arrayIndex)
	{ CopyTo(0, array, arrayIndex, this.Count); }

public void CopyTo(int index, $typemap(cstype, CTYPE)[] array, int arrayIndex, int count)
	{
		if (array == null)
			throw new ArgumentNullException("array");
		if (index < 0)
			throw new ArgumentOutOfRangeException("index", "Value is less than zero");
		if (arrayIndex < 0)
			throw new ArgumentOutOfRangeException("arrayIndex", "Value is less than zero");
		if (count < 0)
			throw new ArgumentOutOfRangeException("count", "Value is less than zero");
		if (array.Rank > 1)
			throw new ArgumentException("Multi dimensional array.", "array");
		if (index+count > this.Count || arrayIndex+count > array.Length)
			throw new ArgumentException("Number of elements to copy is too large.");
		for (int i = 0; i < count; i++)
			array.SetValue(getitem(index+i), arrayIndex + i);
	}

	System.Collections.Generic.IEnumerator<$typemap(cstype, CTYPE)> System.Collections.Generic.IEnumerable<$typemap(cstype, CTYPE)>.GetEnumerator()
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
	public sealed class $csclassnameEnumerator : System.Collections.IEnumerator, System.Collections.Generic.IEnumerator<$typemap(cstype, CTYPE)>
	{
		private $csclassname collectionRef;
		private int currentIndex;
		private object currentObject;
		private int currentSize;

		public $csclassnameEnumerator($csclassname collection)
		{
			collectionRef = collection;
			currentIndex = -1;
			currentObject = null;
			currentSize = collectionRef.Count;
		}

		// Type-safe iterator Current
		public $typemap(cstype, CTYPE) Current
		{
			get
			{
				if (currentIndex == -1)
					throw new InvalidOperationException("Enumeration not started.");
				if (currentIndex > currentSize - 1)
					throw new InvalidOperationException("Enumeration finished.");
				if (currentObject == null)
					throw new InvalidOperationException("Collection modified.");
				return ($typemap(cstype, CTYPE))currentObject;
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
			if (collectionRef.Count != currentSize)
				throw new InvalidOperationException("Collection modified.");
		}

		public void Dispose()
		{
			currentIndex = -1;
			currentObject = null;
		}
		
		public void Reverse()
		{
			Reverse(0, GetSize());
		}
	}
%}

public:
    typedef CTYPE value_type;
	%rename(Clear) Empty;
    %rename(Add) Append;
    %newobject GetRange(int index, int count);
    %newobject Repeat(CTYPE const& value, int count);
    ARRAY();

    %extend
	{
		CTYPE getitem(int index) throw (std::out_of_range)
		{
			if ((index >= 0) && (index < $self->GetSize()))
				return $self->GetAt(index);
			else
				throw std::out_of_range("index");
		}

		void setitem(int index, CTYPE const& val) throw (std::out_of_range)
		{
			if (!$self->SetAt(index, val))
				throw std::out_of_range("index");
		}

		// Takes a deep copy of the elements unlike ArrayList.AddRange
		void AddRange(const ARRAY& values)
		{
			$self->Merge(values);
		}

      // Takes a deep copy of the elements unlike ArrayList.GetRange
		ARRAY* GetRange(int index, int count) throw (std::out_of_range, std::invalid_argument)
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

		void Insert(int index, CTYPE const& x) throw (std::out_of_range)
		{
			throw std::out_of_range("index");
		}

		// Takes a deep copy of the elements unlike ArrayList.InsertRange
		void InsertRange(int index, const ARRAY& values) throw (std::out_of_range)
		{
			throw std::out_of_range("index");
		}

		void RemoveElem(int index) throw (std::out_of_range)
		{
			if (!$self->RemoveAt(index))
				throw std::out_of_range("index");
		}

		void RemoveRange(int index, int count) throw (std::out_of_range, std::invalid_argument)
		{
			if (index < 0)
				throw std::out_of_range("index");
			if (count < 0)
				throw std::out_of_range("count");
			if ((index >= $self->GetSize() + 1) || (index + count > $self->GetSize()))
				throw std::invalid_argument("invalid range");
			throw std::invalid_argument("invalid range");
		}

		static ARRAY* Repeat(CTYPE const& value, int count) throw (std::out_of_range)
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
			int start, end;
			for (start = index, end = index + count - 1; start < end; ++start, --end)
			{
				CTYPE temp = $self->GetAt(start);
				$self->SetAt(start, $self->GetAt(end));
				$self->SetAt(end, temp);
			}
      }

      // Takes a deep copy of the elements unlike ArrayList.SetRange
      void SetRange(int index, const ARRAY& values) throw (std::out_of_range)
	  {
		if (index < 0)
			throw std::out_of_range("index");
		if ((index + values.GetSize()) > $self->GetSize())
			throw std::out_of_range("index");
		for (int i = 0; i < values.GetSize(); ++i)
			$self->SetAt(index + i, values.GetAt(i));
		}
	}
%enddef

// Extra methods added to the collection class if operator== is defined for the class being wrapped
// The class will then implement IList<>, which adds extra functionality
%define SWIG_VXARRAY_EXTRA_OP_EQUALS_EQUALS(CTYPE...)
    %extend
	{
		bool	Contains(CTYPE const& value)	{ return ($self->Find(value) >= 0); }
		int		IndexOf(CTYPE const& value)		{ return $self->Find(value); }

		int		LastIndexOf(CTYPE const& value)
		{
			for (int i = $self->GetSize() - 1; i >= 0; --i)
				if ($self->GetAt(i) == value)
					return i;
			return -1;
		}

		bool Remove(CTYPE const& value)
		{
			int index = $self->Find(value);
			if (index < 0)
				return false;
			return $self->RemoveAt(index);
		}
	}
%enddef

// Macros for Array class specializations/enhancements
%define SWIG_VXARRAY_ENHANCED(ARRAY, CTYPE...)
	template<> class ARRAY
	{
		SWIG_VXARRAY_MINIMUM_INTERNAL(IList, %arg(CTYPE const&), %arg(ARRAY), %arg(CTYPE))
		SWIG_VXARRAY_EXTRA_OP_EQUALS_EQUALS(CTYPE)
	};
%enddef

%{
#include <stdexcept>
%}


%javamethodmodifiers ARRAY::getitem "private"
%javamethodmodifiers ARRAY::setitem "private"
%javamethodmodifiers ARRAY::size "private"
%javamethodmodifiers ARRAY::capacity "private"
%javamethodmodifiers ARRAY::reserve "private"

// specialization for pointers
//template<class T> class Array<T*>
//{
//    SWIG_VXARRAY_MINIMUM_INTERNAL(IList, T *const&, T *)
//    SWIG_VXARRAY_OP_EQUALS_EQUALS(T *)
//};

// template specializations for Array
// these provide extra collections methods as operator== is defined

namespace Vixen {
using namespace Core;
	typedef Core::BaseArray< ObjRef, SharedObj > CoreObjArray;
	typedef Core::BaseArray< int32, SharedObj > CoreIntArray;
	typedef Core::BaseArray< float, SharedObj > CoreFloatArray;
	
	SWIG_VXARRAY_ENHANCED(CoreIntArray, int32)
	SWIG_VXARRAY_ENHANCED(CoreFloatArray, float)
	SWIG_VXARRAY_ENHANCED(CoreObjArray, ObjRef)
	SWIG_VXARRAY_ENHANCED(CoreVec3Array, Vec3)
}



