#pragma once
// Utility classes that can be used as template parameters.
// These class create a basic traits support.

#include <type_traits>

namespace Relib {

namespace Types {

//////////////////////////////////////////////////////////////////////////

// "Enable if" support class.
// Class is used as a template specialization parameter.
// Condition specifies the compile-time assertion that must be met for the specialization to work on the given type.
// For example:
// template<class T, class Enable = void> class CMyClass;	<--- General case.
// template<class T> class CMyClass <T, typename Types::EnableIf<sizeof( T ) == 4>::Result>;	<--- Specialization.
template<bool Condition, class T = void>
struct EnableIf {
	typedef T Result;
};

template<class T>
struct EnableIf<false, T> {
};

//////////////////////////////////////////////////////////////////////////

// An abstract type for function overload resolution.
template <class T>
struct Type {};

//////////////////////////////////////////////////////////////////////////

// Class that contains a static constant of given type and value.
template<class Type, Type val>
struct ConstantType {
	static const Type Result = val;
};

template <bool val>
using BoolType = ConstantType<bool, val>;
// True/False constants.
typedef BoolType<true> TrueType;
typedef BoolType<false> FalseType;

//////////////////////////////////////////////////////////////////////////

// "Is Same" support class.
// This class has a true type if A and B are the same class.
template<class A, class B>
struct IsSame : public FalseType {
};

template<class A>
struct IsSame<A, A> : public TrueType {
};

//////////////////////////////////////////////////////////////////////////

// "Is Array" support class.
// This class has a true type if A is a static array.
template <class A>
struct IsArray : public FalseType {
};

template <class A, int i>
struct IsArray<A[i]> : public TrueType {
};

// Template for determining array's element type.
template <class A>
struct ArrayElemType {
	typedef typename A::TElemType Result;
};

template <class Elem, int i>
struct ArrayElemType<Elem[i]> {
	typedef Elem Result;
};

template <class Elem>
struct ArrayElemType<std::initializer_list<Elem>> {
	typedef Elem Result;
};

template <class Ptr>
struct ArrayElemType<Ptr*> {
	typedef Ptr Result;
};

template <class Ptr>
struct ArrayElemType<const Ptr*> {
	typedef Ptr Result;
};

//////////////////////////////////////////////////////////////////////////

// "Is rvalue reference" support class.
// Derives from true type if the given type is an rvalue reference.
template <class T>
struct IsRValue : public FalseType {

};

template <class T>
struct IsRValue<T&&> : public TrueType {
};

//////////////////////////////////////////////////////////////////////////

// "Is reference" support class.
// Derives from true type if the given type is a reference.
template <class T>
struct IsRef : public FalseType {

};

template <class T>
struct IsRef<T&> : public TrueType {
};

template <class T>
struct IsRef<T&&> : public TrueType {
};

//////////////////////////////////////////////////////////////////////////

// "Is const reference" support class.
// Derives from true type if the given type is a constant reference.
template <class T>
struct IsConstRef : public FalseType {

};

template <class T>
struct IsConstRef<const T&> : public TrueType {
};

template <class T>
struct IsConstRef<const T&&> : public TrueType {
};

//////////////////////////////////////////////////////////////////////////

template <class Type>
struct IsClass : BoolType<std::is_class<Type>::value> {};

template <class Type>
struct IsFundamental : BoolType<std::is_fundamental<Type>::value> {};

template <class Type>
struct IsEnum : BoolType<std::is_enum<Type>::value> {};

template <class Type>
struct IsPOD : BoolType<std::is_pod<Type>::value> {};

template <class Type>
struct IsFloatingPoint : BoolType<std::is_floating_point<Type>::value> {};

template <class Type>
struct IsNumeric : BoolType<std::is_arithmetic<Type>::value || std::is_enum<Type>::value> {};

template <class Type>
struct IsFunction : public BoolType<std::is_function<Type>::value> {};

template <class Type>
struct IsMethodPtr : public BoolType<std::is_member_function_pointer<Type>::value> {};

template <class Type>
struct IsClassMemberPtr : public BoolType<std::is_member_object_pointer<Type>::value> {};

template <class Type, class... Args>
struct HasConstructor : public BoolType<std::is_constructible<Type, Args...>::value> {};

template <class Type>
struct HasMoveConstructor : public BoolType<std::is_move_constructible<Type>::value> {};

template <class Type>
struct HasCopyConstructor : public BoolType<std::is_copy_constructible<Type>::value> {};

template <class Type>
struct HasTrivialCopyConstructor : BoolType<std::is_trivially_copyable<Type>::value> {};

template <class Type>
struct HasTrivialMoveConstructor : BoolType<std::is_trivially_move_constructible<Type>::value> {};

template <class Type>
struct HasTrivialDestructor : BoolType<std::is_trivially_destructible<Type>::value> {};

template <class Type>
struct HasVirtualDestructor : BoolType<std::has_virtual_destructor<Type>::value> {};

template <class Type>
struct IsStandardLayout : BoolType<std::is_standard_layout<Type>::value> {};

template <class From, class To>
struct IsConvertible : public BoolType<std::is_convertible<From, To>::value> {};

//////////////////////////////////////////////////////////////////////////

// "Is pointer" trait class.
template <class T>
struct IsPointer : FalseType {};
template <class T>
struct IsPointer<T*> : TrueType {};

//////////////////////////////////////////////////////////////////////////

// Conditional class. Result's type depends on the condition.
template<bool condition, class IfTrue, class IfFalse>
struct Conditional {
	typedef IfFalse Result;
};

template<class IfTrue, class IfFalse>
struct Conditional<true, IfTrue, IfFalse> {
	typedef IfTrue Result;
};

//////////////////////////////////////////////////////////////////////////

// "Derived from" support class.
// This class has an inner static boolean value that is true only if T is derived from B.
template<class T, class B>
struct BaseIsDerivedFrom {
	static T* CreateT();
	// Two function pointers with return values of different sizes.
	static char ( &CheckT( const B* ) )[1];
	static char ( &CheckT( ... ) )[2];

	// The value of result equals true only if the first CheckT is used.
	// This happens only if T* can be implicitly converted to B*.
	// This can happen if B* is void* or if T is derived from B.
	// The second condition eliminates the former.
	static const bool Result = sizeof( CheckT( CreateT() ) ) == 1 && !IsSame<const volatile B, const volatile void>::Result;
};

template<class T, class B>
struct IsDerivedFrom : public ConstantType<bool, BaseIsDerivedFrom<T, B>::Result> {
};

//////////////////////////////////////////////////////////////////////////

// "Remove pointer" support class.
// Reduces pointer level of a given type by one.
template <class T>
struct RemovePointer {
	typedef T Result;
};

template <class T>
struct RemovePointer<T*> {
	typedef T Result;
};

//////////////////////////////////////////////////////////////////////////


// "Remove reference" support class.
// Removes references from reference types. Used as a part of move function.
template <class Type>
struct RemoveReference {   
	// Does nothing for non-reference types.
	typedef Type Result;
};

template <class Type>
struct RemoveReference<Type&> {   
	// Removes reference.
	typedef Type Result;
};

template <class Type>
struct RemoveReference<Type&&> {   
	// Removes rvalue reference.
	typedef Type Result;
};

//////////////////////////////////////////////////////////////////////////

template <class Type>
struct AddRValueReference {
	Type&& Result;
};

//////////////////////////////////////////////////////////////////////////

// "Remove all qualifiers" support class.
// Removes references and CV qualifiers from types.
template <class T>
struct PureType {
	typedef T Result;
};

template <class T>
struct PureType<T&> {
	typedef T Result;
};

template <class T>
struct PureType<const T&> {
	typedef T Result;
};

template <class T>
struct PureType<T&&> {
	typedef T Result;
};

template <class T>
struct PureType<const T&&> {
	typedef T Result;
};

//////////////////////////////////////////////////////////////////////////

}	// namespace Types.

}	// namespace Relib.
