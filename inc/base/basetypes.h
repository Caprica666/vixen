/****
 *
 * Basic types and constants for the Visual Experience Engine Base layer
 *
 ****/
#pragma once

namespace Vixen {


#define PROP_NameTag	intptr(CLASS_(NameProp))

class Event;
class Property;
class SharedObj;
	class Matrix;
	template <class KEY, class VAL> class Dictionary;
		template <class E> class NameDict;
			class NameTable;
	template <class E> class Array;
	class Group;
	class FileLoader;
		class SceneLoader;
	class Messenger;
		class FileMessenger;
		class BufMessenger;
			class Synchronizer;

} // end Vixen