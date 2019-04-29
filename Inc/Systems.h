#pragma once
#include <ExternalObject.h>
#include <Action.h>
#include <TemplateUtils.h>

namespace Relib {

class CComponentGroup;
class CEntityGroupRange;
class CEntityGroupConstRange;
class CEntityComponentSystem;
//////////////////////////////////////////////////////////////////////////

// Base system context class. This class will be provided to systems as an argument during update.
class ISystemContext {
public:
	virtual ~ISystemContext() {}
};

//////////////////////////////////////////////////////////////////////////

// Basic system that does not necessarily operator on an entity group.
class IBaseSystem : public IExternalObject {
public:
	// Systems are executed by the order of their priority. The higher priority is, the faster it executes.
	// System priority is queried once when the system is added to the ECS.
	virtual int GetPriority() const
		{ return 0; }
};

//////////////////////////////////////////////////////////////////////////

// Constant system that does arbitrary work. This system is not tied to an entity group but still executes at a give priority.
class IReadSystem : public IBaseSystem {
public:
	virtual void RunGeneralDraw( const ISystemContext& context ) const = 0;
};

// System that does arbitrary work. This system is not tied to an entity group but still executes at a give priority.
class IWriteSystem : public IBaseSystem {
public:
	virtual void RunGeneralUpdate( ISystemContext& context ) = 0;
};

//////////////////////////////////////////////////////////////////////////

// An entity update mechanism. Entity operations are unrestricted.
class IUpdateSystem : public IBaseSystem {
public:
	virtual void RunEntityListUpdate( CEntityGroupRange range, ISystemContext& context ) = 0;

	// Systems operate on entities that possess a certain set of components. GetTargetGroup determines, which entity group is passed during the system operation.
	// Target group is queried once when the system is added to the ECS.
	virtual const CComponentGroup& GetTargetGroup() const = 0;
};

// A System that does not change the given entity group or the contents of the ECS.
class IDrawSystem : public IBaseSystem {
public:
	virtual void RunEntityListDraw( CEntityGroupConstRange range, const ISystemContext& context ) const = 0;

	// Systems operate on entities that possess a certain set of components. GetTargetGroup determines, which entity group is passed during the system operation.
	// Target group is queried once when the system is added to the ECS.
	virtual const CComponentGroup& GetTargetGroup() const = 0;
};

//////////////////////////////////////////////////////////////////////////

// Common base class for read systems. Uses CRTP to call the Update method with the casted context.
template <class System>
class CBaseReadSystem : public IReadSystem {
public:
	virtual void RunGeneralDraw( const ISystemContext& context ) const override final;
};

template <class System>
void CBaseReadSystem<System>::RunGeneralDraw( const ISystemContext& context ) const
{
	staticAssert( ( Types::IsDerivedFrom<System, CBaseReadSystem<System>>::Result ) );

	// Get the actual type of required update context. It is assumed to be the second argument of the Update method.
	typedef Types::FunctionInfo<decltype( &System::Draw )>::ArgsTuple TArgsTuple;
	using TRealContextType = typename TArgsTuple::template Elem<1>;
	typedef Types::PureType<TRealContextType>::Result TPureContextType;
	staticAssert( ( Types::IsDerivedFrom<TPureContextType, ISystemContext>::Result ) );

	const auto& realContext = static_cast<const TRealContextType&>( context );
	static_cast<const System&>( *this ).Draw( realContext );
}

//////////////////////////////////////////////////////////////////////////

// Common base class for write systems. Uses CRTP to call the Update method with the casted context.
template <class System>
class CBaseWriteSystem : public IWriteSystem {
public:
	virtual void RunGeneralUpdate( ISystemContext& context ) override final;
};

template <class System>
void CBaseWriteSystem<System>::RunGeneralUpdate( ISystemContext& context )
{
	staticAssert( ( Types::IsDerivedFrom<System, CBaseWriteSystem<System>>::Result ) );

	// Get the actual type of required update context. It is assumed to be the second argument of the Update method.
	typedef Types::FunctionInfo<decltype( &System::Update )>::ArgsTuple TArgsTuple;
	using TRealContextType = typename TArgsTuple::template Elem<1>;
	typedef Types::PureType<TRealContextType>::Result TPureContextType;
	staticAssert( ( Types::IsDerivedFrom<TPureContextType, ISystemContext>::Result ) );

	auto& realContext = static_cast<TRealContextType&>( context );
	static_cast<System&>( *this ).Update( realContext );
}

//////////////////////////////////////////////////////////////////////////

// Common base class for update systems. Uses CRTP to call the UpdateEntities method with the casted context.
// TODO: different overloads for UpdateSingleEntity and UpdateEntityRange.
template <class System>
class CBaseUpdateSystem : public IUpdateSystem {
public:
	virtual void RunEntityListUpdate( CEntityGroupRange range, ISystemContext& context ) override final;
};

template <class System>
void CBaseUpdateSystem<System>::RunEntityListUpdate( CEntityGroupRange range, ISystemContext& context )
{
	staticAssert( ( Types::IsDerivedFrom<System, IUpdateSystem>::Result ) );

	// Get the actual type of required update context. It is assumed to be the second argument of the UpdateEntities method.
	typedef Types::FunctionInfo<decltype( &System::UpdateEntities )>::ArgsTuple TArgsTuple;
	using TRealContextType = typename TArgsTuple::template Elem<1>;
	typedef Types::PureType<TRealContextType>::Result TPureContextType;
	staticAssert( ( Types::IsDerivedFrom<TPureContextType, ISystemContext>::Result ) );

	auto& realContext = static_cast<TRealContextType&>( context );
	static_cast<System&>( *this ).UpdateEntities( range, realContext );
}

//////////////////////////////////////////////////////////////////////////

// Common base class for draw systems. Uses CRTP to call the DrawEntities method with the casted context.
template <class System>
class CBaseDrawSystem : public IDrawSystem {
public:
	virtual void RunEntityListDraw( CEntityGroupConstRange range, const ISystemContext& context ) const override final;
};

template <class System>
void CBaseDrawSystem<System>::RunEntityListDraw( CEntityGroupConstRange range, const ISystemContext& context ) const
{
	staticAssert( ( Types::IsDerivedFrom<System, CBaseDrawSystem<System>>::Result ) );

	// Get the actual type of required system context. It is assumed to be the second argument of the DrawEntities method.
	typedef Types::FunctionInfo<decltype( &System::DrawEntities )>::ArgsTuple TArgsTuple;
	using TRealContextType = typename TArgsTuple::template Elem<1>;
	typedef Types::PureType<TRealContextType>::Result TPureContextType;
	staticAssert( ( Types::IsDerivedFrom<TPureContextType, ISystemContext>::Result ) );

	const auto& realContext = static_cast<const TRealContextType&>( context );
	static_cast<const System&>( *this ).DrawEntities( range, realContext );
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

