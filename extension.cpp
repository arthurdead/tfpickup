/**
 * vim: set ts=4 :
 * =============================================================================
 * SourceMod Sample Extension
 * Copyright (C) 2004-2008 AlliedModders LLC.  All rights reserved.
 * =============================================================================
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License, version 3.0, as published by the
 * Free Software Foundation.
 * 
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * As a special exception, AlliedModders LLC gives you permission to link the
 * code of this program (as well as its derivative works) to "Half-Life 2," the
 * "Source Engine," the "SourcePawn JIT," and any Game MODs that run on software
 * by the Valve Corporation.  You must obey the GNU General Public License in
 * all respects for all other code used.  Additionally, AlliedModders LLC grants
 * this exception to all derivative works.  AlliedModders LLC defines further
 * exceptions, found in LICENSE.txt (as of this writing, version JULY-31-2007),
 * or <http://www.sourcemod.net/license.php>.
 *
 * Version: $Id$
 */

#define swap V_swap

#define BASEENTITY_H
#define NEXT_BOT
#define GLOWS_ENABLE
#define TF_DLL
#define USES_ECON_ITEMS
#define USE_NAV_MESH
#define RAD_TELEMETRY_DISABLED

#include "extension.h"
#include <CDetour/detours.h>

/**
 * @file extension.cpp
 * @brief Implement extension code here.
 */

Sample g_Sample;		/**< Global singleton for extension's main interface */

SMEXT_LINK(&g_Sample);

extern "C"
{
__attribute__((__visibility__("default"), __cdecl__)) double __pow_finite(double a, double b)
{
	return pow(a, b);
}

__attribute__((__visibility__("default"), __cdecl__)) double __log_finite(double a)
{
	return log(a);
}

__attribute__((__visibility__("default"), __cdecl__)) double __exp_finite(double a)
{
	return exp(a);
}

__attribute__((__visibility__("default"), __cdecl__)) double __atan2_finite(double a, double b)
{
	return atan2(a, b);
}

__attribute__((__visibility__("default"), __cdecl__)) float __atan2f_finite(float a, float b)
{
	return atan2f(a, b);
}

__attribute__((__visibility__("default"), __cdecl__)) float __powf_finite(float a, float b)
{
	return powf(a, b);
}

__attribute__((__visibility__("default"), __cdecl__)) float __logf_finite(float a)
{
	return logf(a);
}

__attribute__((__visibility__("default"), __cdecl__)) float __expf_finite(float a)
{
	return expf(a);
}

__attribute__((__visibility__("default"), __cdecl__)) float __acosf_finite(float a)
{
	return acosf(a);
}

__attribute__((__visibility__("default"), __cdecl__)) double __asin_finite(double a)
{
	return asin(a);
}

__attribute__((__visibility__("default"), __cdecl__)) double __acos_finite(double a)
{
	return acos(a);
}
}

int CBaseEntityPostConstructor = -1;
void *CBaseEntityCTOR = nullptr;
int CBaseEntityVPhysicsGetObjectList = -1;
void *CBaseEntityHasNPCsOnIt = nullptr;
int CBaseEntityEyeAngles = -1;
void *CBaseEntityCalcAbsolutePosition = nullptr;
void *Pickup_GetPreferredCarryAnglesPtr = nullptr;
int CBaseEntityWorldSpaceCenter = -1;
void *Pickup_OnPhysGunDropPtr = nullptr;
void *Pickup_OnPhysGunPickupPtr = nullptr;
int CBaseEntitySetParent = -1;
void *PhysComputeSlideDirectionPtr = nullptr;
void *PhysForceClearVelocityPtr = nullptr;
int CBaseEntityEyePosition = -1;
int CBasePlayerWeapon_ShootPosition = -1;
int CBaseCombatWeaponHolster = -1;
int CBaseCombatWeaponDeploy = -1;
int CBaseCombatWeaponCanHolster = -1;
void *CBaseCombatCharacterSwitchToNextBestWeapon = nullptr;
void *StandardFilterRulesPtr = nullptr;
void *CTraceFilterSimpleShouldHitEntity = nullptr;
int CBreakablePropHasInteraction = -1;
int CBaseEntityUse = -1;
void *CCollisionPropertyCollisionToWorldTransform = nullptr;
void *PhysicsImpactSoundPtr = nullptr;
void *PhysRemoveShadowPtr = nullptr;

int sizeofCBaseEntity = -1;

int m_hGroundEntityOffset = -1;
int m_MoveTypeOffset = -1;
int m_pPhysicsObjectOffset = -1;
int m_rgflCoordinateFrameOffset = -1;
int m_iEFlagsOffset = -1;
int m_hUseEntityOffset = -1;
int m_spawnflagsOffset = -1;
int m_vecAbsOriginOffset = -1;
int m_angAbsRotationOffset = -1;
int m_hOwnerEntityOffset = -1;
int m_collisionGroupOffset = -1;
int m_iHideHUDOffset = -1;
int m_hActiveWeaponOffset = -1;
int m_nButtonsOffset = -1;

ICvar *icvar = nullptr;
class CBaseEntityList *g_pEntityList = nullptr;
class IPhysicsEnvironment *physenv = nullptr;
class IPhysicsCollision *physcollision = nullptr;
class IEngineTrace *enginetrace = nullptr;
class IStaticPropMgrServer *staticpropmgr = nullptr;

template <typename T>
T void_to_func(void *ptr)
{
	union { T f; void *p; };
	p = ptr;
	return f;
}

template <typename R, typename T, typename ...Args>
R call_mfunc(T *pThisPtr, void *offset, Args ...args)
{
	class VEmptyClass {};
	
	void **this_ptr = *reinterpret_cast<void ***>(&pThisPtr);
	
	union
	{
		R (VEmptyClass::*mfpnew)(Args...);
#ifndef PLATFORM_POSIX
		void *addr;
	} u;
	u.addr = offset;
#else
		struct  
		{
			void *addr;
			intptr_t adjustor;
		} s;
	} u;
	u.s.addr = offset;
	u.s.adjustor = 0;
#endif
	
	return (R)(reinterpret_cast<VEmptyClass *>(this_ptr)->*u.mfpnew)(args...);
}

template <typename R, typename T, typename ...Args>
R call_mfunc(const T *pThisPtr, void *offset, Args ...args)
{
	return call_mfunc<R, T, Args...>(const_cast<T *>(pThisPtr), offset, args...);
}

template <typename R, typename T, typename ...Args>
R call_vfunc(T *pThisPtr, size_t offset, Args ...args)
{
	void **vtable = *reinterpret_cast<void ***>(pThisPtr);
	void *vfunc = vtable[offset];
	
	return call_mfunc<R, T, Args...>(pThisPtr, vfunc, args...);
}

#include <vphysics_interface.h>
#include <ehandle.h>
#include <vcollide_parse.h>
#include <SoundEmitterSystem/isoundemittersystembase.h>
#include <vphysics/friction.h>
#define DECLARE_PREDICTABLE()
#include <shareddefs.h>
#include <mathlib/vmatrix.h>
#include <util.h>
#include <collisionproperty.h>
#include <engine/ivmodelinfo.h>
#include <toolframework/itoolentity.h>

CGlobalVars *gpGlobals = nullptr;
IVModelInfo *modelinfo = nullptr;
ConVar physcannon_maxmass( "physcannon_maxmass", "250", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar hl2_normspeed( "hl2_normspeed", "190" );
ConVar g_debug_physcannon( "g_debug_physcannon", "0" );
ConVar player_throwforce( "player_throwforce", "1000" );

class IGameSystem;

class CUserCmd;
using EHANDLE = CHandle<CBaseEntity>;

using CPhysicsProp = CBaseEntity;

#include <props_shared.h>

// Maximum number of vphysics objects per entity
#define VPHYSICS_MAX_OBJECT_LIST_COUNT	1024

void PhysRemoveShadow( CBaseEntity *pEntity )
{
	(void_to_func<void(*)(CBaseEntity *)>(PhysRemoveShadowPtr))(pEntity);
}

void UTIL_StringToFloatArray( float *pVector, int count, const char *pString )
{
	char *pstr, *pfront, tempString[128];
	int	j;

	Q_strncpy( tempString, pString, sizeof(tempString) );
	pstr = pfront = tempString;

	for ( j = 0; j < count; j++ )			// lifted from pr_edict.c
	{
		pVector[j] = atof( pfront );

		// skip any leading whitespace
		while ( *pstr && *pstr <= ' ' )
			pstr++;

		// skip to next whitespace
		while ( *pstr && *pstr > ' ' )
			pstr++;

		if (!*pstr)
			break;

		pstr++;
		pfront = pstr;
	}
	for ( j++; j < count; j++ )
	{
		pVector[j] = 0;
	}
}

void UTIL_StringToVector( float *pVector, const char *pString )
{
	UTIL_StringToFloatArray( pVector, 3, pString );
}

#define DOT_1DEGREE   0.9998476951564
#define DOT_2DEGREE   0.9993908270191
#define DOT_3DEGREE   0.9986295347546
#define DOT_4DEGREE   0.9975640502598
#define DOT_5DEGREE   0.9961946980917
#define DOT_6DEGREE   0.9945218953683
#define DOT_7DEGREE   0.9925461516413
#define DOT_8DEGREE   0.9902680687416
#define DOT_9DEGREE   0.9876883405951
#define DOT_10DEGREE  0.9848077530122
#define DOT_15DEGREE  0.9659258262891
#define DOT_20DEGREE  0.9396926207859
#define DOT_25DEGREE  0.9063077870367
#define DOT_30DEGREE  0.866025403784
#define DOT_45DEGREE  0.707106781187

IServerTools *servertools = nullptr;

class CBaseEntity : public IServerEntity
{
public:
	CBaseEntity *GetGroundEntity()
	{
		return *(EHANDLE *)((unsigned char *)this + m_hGroundEntityOffset);
	}
	
	unsigned char GetMoveType()
	{
		if(m_MoveTypeOffset == -1) {
			sm_datatable_info_t info{};
			datamap_t *pMap = gamehelpers->GetDataMap(this);
			gamehelpers->FindDataMapInfo(pMap, "m_MoveType", &info);
			m_MoveTypeOffset = info.actual_offset;
		}
		
		return *(unsigned char *)((unsigned char *)this + m_MoveTypeOffset);
	}
	
	SolidType_t GetSolid()
	{
		return CollisionProp()->GetSolid();
	}
	
	int VPhysicsGetObjectList( IPhysicsObject **pList, int listMax )
	{
		return call_vfunc<int, CBaseEntity, IPhysicsObject **, int>(this, CBaseEntityVPhysicsGetObjectList, pList, listMax);
	}
	
	bool HasNPCsOnIt( void )
	{
		return call_mfunc<bool>(this, CBaseEntityHasNPCsOnIt);
	}
	
	CCollisionProperty *CollisionProp()
	{
		return (CCollisionProperty *)GetCollideable();
	}
	
	IPhysicsObject *&VPhysicsGetObject()
	{
		if(m_pPhysicsObjectOffset == -1) {
			sm_datatable_info_t info{};
			datamap_t *pMap = gamehelpers->GetDataMap(this);
			gamehelpers->FindDataMapInfo(pMap, "m_pPhysicsObject", &info);
			m_pPhysicsObjectOffset = info.actual_offset;
		}
		
		return *(IPhysicsObject **)((unsigned char *)this + m_pPhysicsObjectOffset);
	}
	
	EHANDLE GetOwnerEntity() const
	{
		return *(EHANDLE *)((unsigned char *)this + m_hOwnerEntityOffset);
	}
	
	const QAngle &EyeAngles()
	{
		return call_vfunc<const QAngle &>(this, CBaseEntityEyeAngles);
	}
	
	Vector EyePosition()
	{
		return call_vfunc<Vector>(this, CBaseEntityEyePosition);
	}
	
	matrix3x4_t &EntityToWorldTransform()
	{
		if(m_rgflCoordinateFrameOffset == -1) {
			sm_datatable_info_t info{};
			datamap_t *pMap = gamehelpers->GetDataMap(this);
			gamehelpers->FindDataMapInfo(pMap, "m_rgflCoordinateFrame", &info);
			m_rgflCoordinateFrameOffset = info.actual_offset;
		}
		
		if(GetIEFlags() & EFL_DIRTY_ABSTRANSFORM) {
			CalcAbsolutePosition();
		}
		
		return *(matrix3x4_t *)((unsigned char *)this + m_rgflCoordinateFrameOffset);
	}
	
	int &GetIEFlags()
	{
		if(m_iEFlagsOffset == -1) {
			sm_datatable_info_t info{};
			datamap_t *pMap = gamehelpers->GetDataMap(this);
			gamehelpers->FindDataMapInfo(pMap, "m_iEFlags", &info);
			m_iEFlagsOffset = info.actual_offset;
		}
		
		return *(int *)((unsigned char *)this + m_iEFlagsOffset);
	}
	
	int &GetCollisionGroup()
	{
		if(m_collisionGroupOffset == -1) {
			sm_datatable_info_t info{};
			datamap_t *pMap = gamehelpers->GetDataMap(this);
			gamehelpers->FindDataMapInfo(pMap, "m_collisionGroup", &info);
			m_collisionGroupOffset = info.actual_offset;
		}
		
		return *(int *)((unsigned char *)this + m_collisionGroupOffset);
	}
	
	void SetCollisionGroup(int g)
	{
		GetCollisionGroup() = g;
	}
	
	const Vector &WorldSpaceCenter()
	{
		return call_vfunc<const Vector &>(this, CBaseEntityWorldSpaceCenter);
	}
	
	void CalcAbsolutePosition()
	{
		call_mfunc<void, CBaseEntity>(this, CBaseEntityCalcAbsolutePosition);
	}
	
	bool BlocksLOS()
	{
		return GetIEFlags() & EFL_DONTBLOCKLOS;
	}
	
	bool ClassnameIs(const char *cls)
	{
		return strcmp(GetClassname(), cls) == 0;
	}
	
	CBaseEntity *IsPhysicsProp()
	{
		if(ClassnameIs("physics_prop") ||
			ClassnameIs("prop_physics") ||
			ClassnameIs("prop_physics_override")) {
			return this;
		} else {
			return nullptr;
		}
	}
	
	CBaseEntity *IsPhysBox()
	{
		if(ClassnameIs("func_physbox_multiplayer") ||
			ClassnameIs("func_physbox")) {
			return this;
		} else {
			return nullptr;
		}
	}
	
	CBaseEntity *IsRagdoll()
	{
		if(ClassnameIs("prop_ragdoll") ||
			ClassnameIs("physics_prop_ragdoll")) {
			return this;
		} else {
			return nullptr;
		}
	}
	
	CBaseEntity *IsPickupController()
	{
		if(ClassnameIs("player_pickup")) {
			return this;
		} else {
			return nullptr;
		}
	}
	
	int &GetSpawnFlags()
	{
		if(m_spawnflagsOffset == -1) {
			sm_datatable_info_t info{};
			datamap_t *pMap = gamehelpers->GetDataMap(this);
			gamehelpers->FindDataMapInfo(pMap, "m_spawnflags", &info);
			m_spawnflagsOffset = info.actual_offset;
		}
		
		return *(int *)((unsigned char *)this + m_spawnflagsOffset);
	}
	
	bool HasSpawnFlags(int flags)
	{
		return GetSpawnFlags() & flags;
	}
	
	bool IsEFlagSet(int flags)
	{
		return GetIEFlags() & flags;
	}
	
	void Remove()
	{
		servertools->RemoveEntity(this);
	}
	
	void SetParent(CBaseEntity *pEntity, int i = -1)
	{
		call_vfunc<void, CBaseEntity, CBaseEntity *, int>(this, CBaseEntitySetParent, pEntity, i);
	}
	
	const char *GetClassname()
	{
		return gamehelpers->GetEntityClassname(this);
	}
	
	void SetBlocksLOS( bool bBlocksLOS )
	{
		if ( bBlocksLOS )
		{
			GetIEFlags() &= ~EFL_DONTBLOCKLOS;
		}
		else
		{
			GetIEFlags() |= EFL_DONTBLOCKLOS;
		}
	}
	
	void VPhysicsSwapObject( IPhysicsObject *pSwap )
	{
		if ( !pSwap )
		{
			PhysRemoveShadow(this);
		}

		if ( !VPhysicsGetObject() )
		{
			Warning( "Bad vphysics swap for %s\n", GetClassname() );
		}
		VPhysicsGetObject() = pSwap;
	}
	
	model_t *GetModel( void )
	{
		return (model_t *)modelinfo->GetModel( GetModelIndex() );
	}
	
	bool GetPropDataAngles( const char *pKeyName, QAngle &vecAngles )
	{
		KeyValues *modelKeyValues = new KeyValues("");
		if ( modelKeyValues->LoadFromBuffer( modelinfo->GetModelName( GetModel() ), modelinfo->GetModelKeyValueText( GetModel() ) ) )
		{
			KeyValues *pkvPropData = modelKeyValues->FindKey( "physgun_interactions" );
			if ( pkvPropData )
			{
				char const *pszBase = pkvPropData->GetString( pKeyName );
				if ( pszBase && pszBase[0] )
				{
					UTIL_StringToVector( vecAngles.Base(), pszBase );
					modelKeyValues->deleteThis();
					return true;
				}
			}
		}

		modelKeyValues->deleteThis();
		return false;
	}
	
	float GetCarryDistanceOffset( void )
	{
		KeyValues *modelKeyValues = new KeyValues("");
		if ( modelKeyValues->LoadFromBuffer( modelinfo->GetModelName( GetModel() ), modelinfo->GetModelKeyValueText( GetModel() ) ) )
		{
			KeyValues *pkvPropData = modelKeyValues->FindKey( "physgun_interactions" );
			if ( pkvPropData )
			{
				float flDistance = pkvPropData->GetFloat( "carry_distance_offset", 0 );
				modelKeyValues->deleteThis();
				return flDistance;
			}
		}

		modelKeyValues->deleteThis();
		return 0;
	}
	
	const Vector &GetAbsOrigin()
	{
		if(m_vecAbsOriginOffset == -1) {
			datamap_t *map = gamehelpers->GetDataMap(this);
			sm_datatable_info_t info{};
			gamehelpers->FindDataMapInfo(map, "m_vecAbsOrigin", &info);
			m_vecAbsOriginOffset = info.actual_offset;
		}
		
		return *(Vector *)(((unsigned char *)this) + m_vecAbsOriginOffset);
	}
	
	const QAngle &GetAbsAngles()
	{
		if(m_angAbsRotationOffset == -1) {
			datamap_t *map = gamehelpers->GetDataMap(this);
			sm_datatable_info_t info{};
			gamehelpers->FindDataMapInfo(map, "m_angAbsRotation", &info);
			m_angAbsRotationOffset = info.actual_offset;
		}
		
		return *(QAngle *)(((unsigned char *)this) + m_angAbsRotationOffset);
	}
	
	const Vector& WorldAlignMins( )
	{
		return CollisionProp()->OBBMins();
	}
	
	bool HasInteraction( propdata_interactions_t Interaction )
	{
		return call_vfunc<bool, CBaseEntity, propdata_interactions_t>(this, CBreakablePropHasInteraction, Interaction);
	}
	
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
	{
		call_vfunc<void, CBaseEntity, CBaseEntity *, CBaseEntity *, USE_TYPE, float>(this, CBaseEntityUse, pActivator, pCaller, useType, value);
	}
};

class CBaseCombatWeapon : public CBaseEntity
{
public:
	bool Holster(CBaseCombatWeapon *other = nullptr)
	{
		return call_vfunc<bool, CBaseCombatWeapon, CBaseCombatWeapon *>(this, CBaseCombatWeaponHolster, other);
	}
	
	bool Deploy()
	{
		return call_vfunc<bool>(this, CBaseCombatWeaponDeploy);
	}
	
	bool CanHolster()
	{
		return call_vfunc<bool>(this, CBaseCombatWeaponCanHolster);
	}
};

class CBasePlayer : public CBaseEntity
{
public:
	EHANDLE &GetUseEntity()
	{
		return *(EHANDLE *)((unsigned char *)this + m_hUseEntityOffset);
	}
	
	void SetUseEntity(CBaseEntity *pEntity)
	{
		GetUseEntity() = pEntity;
	}
	
	CBaseCombatWeapon *GetActiveWeapon()
	{
		return (CBaseCombatWeapon *)(CBaseEntity *)*(EHANDLE *)((unsigned char *)this + m_hActiveWeaponOffset);
	}
	
	Vector Weapon_ShootPosition()
	{
		return call_vfunc<Vector>(this, CBasePlayerWeapon_ShootPosition);
	}
	
	int &GetHideHud()
	{
		return *(int *)((unsigned char *)this + m_iHideHUDOffset);
	}
	
	int GetButtons()
	{
		if(m_nButtonsOffset == -1) {
			datamap_t *map = gamehelpers->GetDataMap(this);
			sm_datatable_info_t info{};
			gamehelpers->FindDataMapInfo(map, "m_nButtons", &info);
			m_nButtonsOffset = info.actual_offset;
		}
		
		return *(int *)((unsigned char *)this + m_nButtonsOffset);
	}
	
	bool SwitchToNextBestWeapon(CBaseCombatWeapon *pCurrent)
	{
		return call_mfunc<bool, CBasePlayer, CBaseCombatWeapon *>(this, CBaseCombatCharacterSwitchToNextBestWeapon, pCurrent);
	}
	
	bool ClearUseEntity()
	{
		if ( ((CBaseEntity *)GetUseEntity()) != NULL )
		{
			// Stop controlling the train/object
			// TODO: Send HUD Update
			GetUseEntity()->Use( this, this, USE_OFF, 0 );
			GetUseEntity() = NULL;
			return true;
		}

		return false;
	}
	
	void EyeVectors( Vector *pForward, Vector *pRight = NULL, Vector *pUp = NULL )
	{
		/*if ( GetVehicle() != NULL )
		{
			// Cache or retrieve our calculated position in the vehicle
			CacheVehicleView();
			AngleVectors( m_vecVehicleViewAngles, pForward, pRight, pUp );
		}
		else*/
		{
			AngleVectors( EyeAngles(), pForward, pRight, pUp );
		}
	}
};

using CPointEntity = CBaseEntity;

#define SF_PHYSBOX_ASLEEP					0x01000
#define SF_PHYSBOX_IGNOREUSE				0x02000
#define SF_PHYSBOX_DEBRIS					0x04000
#define SF_PHYSBOX_MOTIONDISABLED			0x08000
#define SF_PHYSBOX_USEPREFERRED				0x10000
#define SF_PHYSBOX_ENABLE_ON_PHYSCANNON		0x20000
#define SF_PHYSBOX_NO_ROTORWASH_PUSH		0x40000		// The rotorwash doesn't push these
#define SF_PHYSBOX_ENABLE_PICKUP_OUTPUT		0x80000
#define SF_PHYSBOX_ALWAYS_PICK_UP		    0x100000		// Physcannon can always pick this up, no matter what mass or constraints may apply.
#define SF_PHYSBOX_NEVER_PICK_UP			0x200000		// Physcannon will never be able to pick this up.
#define SF_PHYSBOX_NEVER_PUNT				0x400000		// Physcannon will never be able to punt this object.
#define SF_PHYSBOX_PREVENT_PLAYER_TOUCH_ENABLE 0x800000		// If set, the player will not cause the object to enable its motion when bumped into

using CPhysBox = CBaseEntity;

#include <physics_shared.h>
#include <player_pickup.h>

SH_DECL_MANUALHOOK2_void(PickupObject, 0, 0, 0, CBaseEntity *, bool)
SH_DECL_MANUALHOOK1(GetHeldObjectMass, 0, 0, 0, float, IPhysicsObject *)
SH_DECL_MANUALHOOK1_void(ForceDropOfCarriedPhysObjects, 0, 0, 0, CBaseEntity *)

bool CanPickupObject( CBaseEntity *pObject, float massLimit, float sizeLimit )
{
	// UNDONE: Make this virtual and move to HL2 player

	//Must be valid
	if ( pObject == NULL )
		return false;

	//Must move with physics
	if ( pObject->GetMoveType() != MOVETYPE_VPHYSICS )
		return false;

	IPhysicsObject *pList[VPHYSICS_MAX_OBJECT_LIST_COUNT];
	int count = pObject->VPhysicsGetObjectList( pList, ARRAYSIZE(pList) );

	//Must have a physics object
	if (!count)
		return false;

	float objectMass = 0;
	bool checkEnable = false;
	for ( int i = 0; i < count; i++ )
	{
		objectMass += pList[i]->GetMass();
		if ( !pList[i]->IsMoveable() )
		{
			checkEnable = true;
		}
		if ( pList[i]->GetGameFlags() & FVPHYSICS_NO_PLAYER_PICKUP )
			return false;
		if ( pList[i]->IsHinged() )
			return false;
	}


	//Msg( "Target mass: %f\n", pPhys->GetMass() );

	//Must be under our threshold weight
	if ( massLimit > 0 && objectMass > massLimit )
		return false;

	if ( checkEnable )
	{
		// Allowing picking up of bouncebombs.

		// Allow pickup of phys props that are motion enabled on player pickup
		CPhysicsProp *pProp = pObject->IsPhysicsProp();
		CPhysBox *pBox = pObject->IsPhysBox();
		if ( !pProp && !pBox )
			return false;

		if ( pProp && !(pProp->HasSpawnFlags( SF_PHYSPROP_ENABLE_ON_PHYSCANNON )) )
			return false;

		if ( pBox && !(pBox->HasSpawnFlags( SF_PHYSBOX_ENABLE_ON_PHYSCANNON )) )
			return false;
	}

	if ( sizeLimit > 0 )
	{
		const Vector &size = pObject->CollisionProp()->OBBSize();
		if ( size.x > sizeLimit || size.y > sizeLimit || size.z > sizeLimit )
			return false;
	}

	return true;
}

struct game_shadowcontrol_params_t : public hlshadowcontrol_params_t
{
	DECLARE_SIMPLE_DATADESC();
};

class CGrabController : public IMotionEvent
{
	DECLARE_SIMPLE_DATADESC();

public:

	CGrabController( void );
	~CGrabController( void );
	void AttachEntity( CBasePlayer *pPlayer, CBaseEntity *pEntity, IPhysicsObject *pPhys, bool bIsMegaPhysCannon, const Vector &vGrabPosition, bool bUseGrabPosition );
	void DetachEntity( bool bClearVelocity );
	void OnRestore();

	bool UpdateObject( CBasePlayer *pPlayer, float flError );

	void SetTargetPosition( const Vector &target, const QAngle &targetOrientation );
	float ComputeError();
	float GetLoadWeight( void ) const { return m_flLoadWeight; }
	void SetAngleAlignment( float alignAngleCosine ) { m_angleAlignment = alignAngleCosine; }
	void SetIgnorePitch( bool bIgnore ) { m_bIgnoreRelativePitch = bIgnore; }
	QAngle TransformAnglesToPlayerSpace( const QAngle &anglesIn, CBasePlayer *pPlayer );
	QAngle TransformAnglesFromPlayerSpace( const QAngle &anglesIn, CBasePlayer *pPlayer );

	CBaseEntity *GetAttached() { return (CBaseEntity *)m_attachedEntity; }

	IMotionEvent::simresult_e Simulate( IPhysicsMotionController *pController, IPhysicsObject *pObject, float deltaTime, Vector &linear, AngularImpulse &angular );
	float GetSavedMass( IPhysicsObject *pObject );

	bool IsObjectAllowedOverhead( CBaseEntity *pEntity );

private:
	// Compute the max speed for an attached object
	void ComputeMaxSpeed( CBaseEntity *pEntity, IPhysicsObject *pPhysics );

	game_shadowcontrol_params_t	m_shadow;
	float			m_timeToArrive;
	float			m_errorTime;
	float			m_error;
	float			m_contactAmount;
	float			m_angleAlignment;
	bool			m_bCarriedEntityBlocksLOS;
	bool			m_bIgnoreRelativePitch;

	float			m_flLoadWeight;
	float			m_savedRotDamping[VPHYSICS_MAX_OBJECT_LIST_COUNT];
	float			m_savedMass[VPHYSICS_MAX_OBJECT_LIST_COUNT];
	EHANDLE			m_attachedEntity;
	QAngle			m_vecPreferredCarryAngles;
	bool			m_bHasPreferredCarryAngles;
	float			m_flDistanceOffset;

	QAngle			m_attachedAnglesPlayerSpace;
	Vector			m_attachedPositionObjectSpace;

	IPhysicsMotionController *m_controller;

	bool			m_bAllowObjectOverhead; // Can the player hold this object directly overhead? (Default is NO)

	// NVNT player controlling this grab controller
	CBasePlayer*	m_pControllingPlayer;

	friend class CWeaponPhysCannon;
};

const float DEFAULT_MAX_ANGULAR = 360.0f * 10.0f;
const float REDUCED_CARRY_MASS = 1.0f;

CGrabController::CGrabController( void )
{
	m_shadow.dampFactor = 1.0;
	m_shadow.teleportDistance = 0;
	m_errorTime = 0;
	m_error = 0;
	// make this controller really stiff!
	m_shadow.maxSpeed = 1000;
	m_shadow.maxAngular = DEFAULT_MAX_ANGULAR;
	m_shadow.maxDampSpeed = m_shadow.maxSpeed*2;
	m_shadow.maxDampAngular = m_shadow.maxAngular;
	m_attachedEntity = NULL;
	m_vecPreferredCarryAngles = vec3_angle;
	m_bHasPreferredCarryAngles = false;
	m_flDistanceOffset = 0;
	// NVNT constructing m_pControllingPlayer to NULL
	m_pControllingPlayer = NULL;
}

CGrabController::~CGrabController( void )
{
	DetachEntity( false );
}

void CGrabController::OnRestore()
{
	if ( m_controller )
	{
		m_controller->SetEventHandler( this );
	}
}

void CGrabController::SetTargetPosition( const Vector &target, const QAngle &targetOrientation )
{
	m_shadow.targetPosition = target;
	m_shadow.targetRotation = targetOrientation;

	m_timeToArrive = gpGlobals->frametime;

	CBaseEntity *pAttached = GetAttached();
	if ( pAttached )
	{
		IPhysicsObject *pObj = pAttached->VPhysicsGetObject();
		
		if ( pObj != NULL )
		{
			pObj->Wake();
		}
		else
		{
			DetachEntity( false );
		}
	}
}

float CGrabController::ComputeError()
{
	if ( m_errorTime <= 0 )
		return 0;

	CBaseEntity *pAttached = GetAttached();
	if ( pAttached )
	{
		Vector pos;
		IPhysicsObject *pObj = pAttached->VPhysicsGetObject();
		
		if ( pObj )
		{	
			pObj->GetShadowPosition( &pos, NULL );

			float error = (m_shadow.targetPosition - pos).Length();
			if ( m_errorTime > 0 )
			{
				if ( m_errorTime > 1 )
				{
					m_errorTime = 1;
				}
				float speed = error / m_errorTime;
				if ( speed > m_shadow.maxSpeed )
				{
					error *= 0.5;
				}
				m_error = (1-m_errorTime) * m_error + error * m_errorTime;
			}
		}
		else
		{
			DevMsg( "Object attached to Physcannon has no physics object\n" );
			DetachEntity( false );
			return 9999; // force detach
		}
	}
	
	if ( pAttached->IsEFlagSet( EFL_IS_BEING_LIFTED_BY_BARNACLE ) )
	{
		m_error *= 3.0f;
	}

	m_errorTime = 0;

	return m_error;
}

#define MASS_SPEED_SCALE	60
#define MAX_MASS			40

float PhysGetEntityMass( CBaseEntity *pEntity )
{
	IPhysicsObject *pList[VPHYSICS_MAX_OBJECT_LIST_COUNT];
	int physCount = pEntity->VPhysicsGetObjectList( pList, ARRAYSIZE(pList) );
	float otherMass = 0;
	for ( int i = 0; i < physCount; i++ )
	{
		otherMass += pList[i]->GetMass();
	}

	return otherMass;
}

void CGrabController::ComputeMaxSpeed( CBaseEntity *pEntity, IPhysicsObject *pPhysics )
{
	m_shadow.maxSpeed = 1000;
	m_shadow.maxAngular = DEFAULT_MAX_ANGULAR;

	// Compute total mass...
	float flMass = PhysGetEntityMass( pEntity );
	float flMaxMass = physcannon_maxmass.GetFloat();
	if ( flMass <= flMaxMass )
		return;

	float flLerpFactor = clamp( flMass, flMaxMass, 500.0f );
	flLerpFactor = SimpleSplineRemapVal( flLerpFactor, flMaxMass, 500.0f, 0.0f, 1.0f );

	float invMass = pPhysics->GetInvMass();
	float invInertia = pPhysics->GetInvInertia().Length();

	float invMaxMass = 1.0f / MAX_MASS;
	float ratio = invMaxMass / invMass;
	invMass = invMaxMass;
	invInertia *= ratio;

	float maxSpeed = invMass * MASS_SPEED_SCALE * 200;
	float maxAngular = invInertia * MASS_SPEED_SCALE * 360;

	m_shadow.maxSpeed = Lerp( flLerpFactor, m_shadow.maxSpeed, maxSpeed );
	m_shadow.maxAngular = Lerp( flLerpFactor, m_shadow.maxAngular, maxAngular );
}

QAngle CGrabController::TransformAnglesToPlayerSpace( const QAngle &anglesIn, CBasePlayer *pPlayer )
{
	if ( m_bIgnoreRelativePitch )
	{
		matrix3x4_t test;
		QAngle angleTest = pPlayer->EyeAngles();
		angleTest.x = 0;
		AngleMatrix( angleTest, test );
		return TransformAnglesToLocalSpace( anglesIn, test );
	}
	return TransformAnglesToLocalSpace( anglesIn, pPlayer->EntityToWorldTransform() );
}

QAngle CGrabController::TransformAnglesFromPlayerSpace( const QAngle &anglesIn, CBasePlayer *pPlayer )
{
	if ( m_bIgnoreRelativePitch )
	{
		matrix3x4_t test;
		QAngle angleTest = pPlayer->EyeAngles();
		angleTest.x = 0;
		AngleMatrix( angleTest, test );
		return TransformAnglesToWorldSpace( anglesIn, test );
	}
	return TransformAnglesToWorldSpace( anglesIn, pPlayer->EntityToWorldTransform() );
}

void PhysicsImpactSound( CBaseEntity *pEntity, IPhysicsObject *pPhysObject, int channel, int surfaceProps, int surfacePropsHit, float volume, float impactSpeed )
{
	(void_to_func<void (*)(CBaseEntity *, IPhysicsObject *, int, int, int, float, float)>(PhysicsImpactSoundPtr))(pEntity, pPhysObject, channel, surfaceProps, surfacePropsHit, volume, impactSpeed);
}

bool Pickup_GetPreferredCarryAngles( CBaseEntity *pObject, CBasePlayer *pPlayer, matrix3x4_t &localToWorld, QAngle &outputAnglesWorldSpace )
{
	return (void_to_func<bool(*)(CBaseEntity *, CBasePlayer *, matrix3x4_t &, QAngle &)>(Pickup_GetPreferredCarryAnglesPtr))(pObject, pPlayer, localToWorld, outputAnglesWorldSpace);
}

IPhysicsObject *GetRagdollChildAtPosition( CBaseEntity *pTarget, const Vector &position )
{
	// Check for a ragdoll
	if ( pTarget->IsRagdoll() == NULL )
		return NULL;

	// Get the root
	IPhysicsObject *pList[VPHYSICS_MAX_OBJECT_LIST_COUNT];
	int count = pTarget->VPhysicsGetObjectList( pList, ARRAYSIZE( pList ) );
	
	IPhysicsObject *pBestChild = NULL;
	float			flBestDist = 99999999.0f;
	float			flDist;
	Vector			vPos;

	// Find the nearest child to where we're looking
	for ( int i = 0; i < count; i++ )
	{
		pList[i]->GetPosition( &vPos, NULL );
		
		flDist = ( position - vPos ).LengthSqr();

		if ( flDist < flBestDist )
		{
			pBestChild = pList[i];
			flBestDist = flDist;
		}
	}

	// Make this our base now
	pTarget->VPhysicsSwapObject( pBestChild );

	return pTarget->VPhysicsGetObject();
}

static void MatrixOrthogonalize( matrix3x4_t &matrix, int column )
{
	Vector columns[3];
	int i;

	for ( i = 0; i < 3; i++ )
	{
		MatrixGetColumn( matrix, i, columns[i] );
	}

	int index0 = column;
	int index1 = (column+1)%3;
	int index2 = (column+2)%3;

	columns[index2] = CrossProduct( columns[index0], columns[index1] );
	columns[index1] = CrossProduct( columns[index2], columns[index0] );
	VectorNormalize( columns[index2] );
	VectorNormalize( columns[index1] );
	MatrixSetColumn( columns[index1], index1, matrix );
	MatrixSetColumn( columns[index2], index2, matrix );
}

#define SIGN(x) ( (x) < 0 ? -1 : 1 )

static QAngle AlignAngles( const QAngle &angles, float cosineAlignAngle )
{
	matrix3x4_t alignMatrix;
	AngleMatrix( angles, alignMatrix );

	// NOTE: Must align z first
	for ( int j = 3; --j >= 0; )
	{
		Vector vec;
		MatrixGetColumn( alignMatrix, j, vec );
		for ( int i = 0; i < 3; i++ )
		{
			if ( fabs(vec[i]) > cosineAlignAngle )
			{
				vec[i] = SIGN(vec[i]);
				vec[(i+1)%3] = 0;
				vec[(i+2)%3] = 0;
				MatrixSetColumn( vec, j, alignMatrix );
				MatrixOrthogonalize( alignMatrix, j );
				break;
			}
		}
	}

	QAngle out;
	MatrixAngles( alignMatrix, out );
	return out;
}

void CGrabController::AttachEntity( CBasePlayer *pPlayer, CBaseEntity *pEntity, IPhysicsObject *pPhys, bool bIsMegaPhysCannon, const Vector &vGrabPosition, bool bUseGrabPosition )
{
	// play the impact sound of the object hitting the player
	// used as feedback to let the player know he picked up the object
	int hitMaterial = pPhys->GetMaterialIndex();
	int playerMaterial = pPlayer->VPhysicsGetObject() ? pPlayer->VPhysicsGetObject()->GetMaterialIndex() : hitMaterial;
	PhysicsImpactSound( pPlayer, pPhys, CHAN_STATIC, hitMaterial, playerMaterial, 1.0, 64 );
	Vector position;
	QAngle angles;
	pPhys->GetPosition( &position, &angles );
	// If it has a preferred orientation, use that instead.
	Pickup_GetPreferredCarryAngles( pEntity, pPlayer, pPlayer->EntityToWorldTransform(), angles );

//	ComputeMaxSpeed( pEntity, pPhys );

	// If we haven't been killed by a grab, we allow the gun to grab the nearest part of a ragdoll
	if ( bUseGrabPosition )
	{
		IPhysicsObject *pChild = GetRagdollChildAtPosition( pEntity, vGrabPosition );
		
		if ( pChild )
		{
			pPhys = pChild;
		}
	}

	// Carried entities can never block LOS
	m_bCarriedEntityBlocksLOS = pEntity->BlocksLOS();
	pEntity->SetBlocksLOS( false );
	m_controller = physenv->CreateMotionController( this );
	m_controller->AttachObject( pPhys, true );
	// Don't do this, it's causing trouble with constraint solvers.
	//m_controller->SetPriority( IPhysicsMotionController::HIGH_PRIORITY );

	pPhys->Wake();
	PhysSetGameFlags( pPhys, FVPHYSICS_PLAYER_HELD );
	SetTargetPosition( position, angles );
	m_attachedEntity = pEntity;
	IPhysicsObject *pList[VPHYSICS_MAX_OBJECT_LIST_COUNT];
	int count = pEntity->VPhysicsGetObjectList( pList, ARRAYSIZE(pList) );
	m_flLoadWeight = 0;
	float damping = 10;
	float flFactor = count / 7.5f;
	if ( flFactor < 1.0f )
	{
		flFactor = 1.0f;
	}
	for ( int i = 0; i < count; i++ )
	{
		float mass = pList[i]->GetMass();
		pList[i]->GetDamping( NULL, &m_savedRotDamping[i] );
		m_flLoadWeight += mass;
		m_savedMass[i] = mass;

		// reduce the mass to prevent the player from adding crazy amounts of energy to the system
		pList[i]->SetMass( REDUCED_CARRY_MASS / flFactor );
		pList[i]->SetDamping( NULL, &damping );
	}

	// NVNT setting m_pControllingPlayer to the player attached
	m_pControllingPlayer = pPlayer;
	
	// Give extra mass to the phys object we're actually picking up
	pPhys->SetMass( REDUCED_CARRY_MASS );
	pPhys->EnableDrag( false );

	m_errorTime = bIsMegaPhysCannon ? -1.5f : -1.0f; // 1 seconds until error starts accumulating
	m_error = 0;
	m_contactAmount = 0;

	m_attachedAnglesPlayerSpace = TransformAnglesToPlayerSpace( angles, pPlayer );
	if ( m_angleAlignment != 0 )
	{
		m_attachedAnglesPlayerSpace = AlignAngles( m_attachedAnglesPlayerSpace, m_angleAlignment );
	}

	// Ragdolls don't offset this way
	if ( pEntity->IsRagdoll() )
	{
		m_attachedPositionObjectSpace.Init();
	}
	else
	{
		VectorITransform( pEntity->WorldSpaceCenter(), pEntity->EntityToWorldTransform(), m_attachedPositionObjectSpace );
	}
	
#define GCC_LINKER_BEING_STUPID

	// If it's a prop, see if it has desired carry angles
	CPhysicsProp *pProp = pEntity->IsPhysicsProp();
	if ( pProp )
	{
#ifndef GCC_LINKER_BEING_STUPID
		m_bHasPreferredCarryAngles = pProp->GetPropDataAngles( "preferred_carryangles", m_vecPreferredCarryAngles );
		m_flDistanceOffset = pProp->GetCarryDistanceOffset();
#else
		m_bHasPreferredCarryAngles = false;
		m_flDistanceOffset = 0;
#endif
	}
	else
	{
		m_bHasPreferredCarryAngles = false;
		m_flDistanceOffset = 0;
	}

	m_bAllowObjectOverhead = IsObjectAllowedOverhead( pEntity );
}

static void ClampPhysicsVelocity( IPhysicsObject *pPhys, float linearLimit, float angularLimit )
{
	Vector vel;
	AngularImpulse angVel;
	pPhys->GetVelocity( &vel, &angVel );
	float speed = VectorNormalize(vel) - linearLimit;
	float angSpeed = VectorNormalize(angVel) - angularLimit;
	speed = speed < 0 ? 0 : -speed;
	angSpeed = angSpeed < 0 ? 0 : -angSpeed;
	vel *= speed;
	angVel *= angSpeed;
	pPhys->AddVelocity( &vel, &angVel );
}

void PhysForceClearVelocity( IPhysicsObject *pPhys )
{
	return (void_to_func<void(*)(IPhysicsObject *)>(PhysForceClearVelocityPtr))(pPhys);
}

void CGrabController::DetachEntity( bool bClearVelocity )
{
	CBaseEntity *pEntity = GetAttached();
	if ( pEntity )
	{
		// Restore the LS blocking state
		pEntity->SetBlocksLOS( m_bCarriedEntityBlocksLOS );
		IPhysicsObject *pList[VPHYSICS_MAX_OBJECT_LIST_COUNT];
		int count = pEntity->VPhysicsGetObjectList( pList, ARRAYSIZE(pList) );

		for ( int i = 0; i < count; i++ )
		{
			IPhysicsObject *pPhys = pList[i];
			if ( !pPhys )
				continue;

			// on the odd chance that it's gone to sleep while under anti-gravity
			pPhys->EnableDrag( true );
			pPhys->Wake();
			pPhys->SetMass( m_savedMass[i] );
			pPhys->SetDamping( NULL, &m_savedRotDamping[i] );
			PhysClearGameFlags( pPhys, FVPHYSICS_PLAYER_HELD );
			if ( bClearVelocity )
			{
				PhysForceClearVelocity( pPhys );
			}
			else
			{
#ifndef CLIENT_DLL
				ClampPhysicsVelocity( pPhys, hl2_normspeed.GetFloat() * 1.5f, 2.0f * 360.0f );
#endif
			}

		}
	}

	m_attachedEntity = NULL;
	if ( physenv )
	{
		physenv->DestroyMotionController( m_controller );
	}
	m_controller = NULL;
}

bool CGrabController::IsObjectAllowedOverhead( CBaseEntity *pEntity )
{
	// Allow combine balls overhead 

	// Allow props that are specifically flagged as such
	CPhysicsProp *pPhysProp = pEntity->IsPhysicsProp();
	if ( pPhysProp != NULL && pPhysProp->HasInteraction( PROPINTER_PHYSGUN_ALLOW_OVERHEAD ) )
		return true;

	return false;
}

static bool InContactWithHeavyObject( IPhysicsObject *pObject, float heavyMass )
{
	bool contact = false;
	IPhysicsFrictionSnapshot *pSnapshot = pObject->CreateFrictionSnapshot();
	while ( pSnapshot->IsValid() )
	{
		IPhysicsObject *pOther = pSnapshot->GetObject( 1 );
		if ( !pOther->IsMoveable() || pOther->GetMass() > heavyMass )
		{
			contact = true;
			break;
		}
		pSnapshot->NextFrictionData();
	}
	pObject->DestroyFrictionSnapshot( pSnapshot );
	return contact;
}

void PhysComputeSlideDirection( IPhysicsObject *pPhysics, const Vector &inputVelocity, const AngularImpulse &inputAngularVelocity, Vector *pOutputVelocity, Vector *pOutputAngularVelocity, float minMass )
{
	(void_to_func<void(*)(IPhysicsObject *, const Vector &, const AngularImpulse &, Vector *, Vector *, float)>(PhysComputeSlideDirectionPtr))(pPhysics, inputVelocity, inputAngularVelocity, pOutputVelocity, pOutputAngularVelocity, minMass);
}

IMotionEvent::simresult_e CGrabController::Simulate( IPhysicsMotionController *pController, IPhysicsObject *pObject, float deltaTime, Vector &linear, AngularImpulse &angular )
{
	game_shadowcontrol_params_t shadowParams = m_shadow;
	if ( InContactWithHeavyObject( pObject, GetLoadWeight() ) )
	{
		m_contactAmount = Approach( 0.1f, m_contactAmount, deltaTime*2.0f );
	}
	else
	{
		m_contactAmount = Approach( 1.0f, m_contactAmount, deltaTime*2.0f );
	}
	shadowParams.maxAngular = m_shadow.maxAngular * m_contactAmount * m_contactAmount * m_contactAmount;
#ifndef CLIENT_DLL
	m_timeToArrive = pObject->ComputeShadowControl( shadowParams, m_timeToArrive, deltaTime );
#else
	m_timeToArrive = pObject->ComputeShadowControl( shadowParams, (TICK_INTERVAL*2), deltaTime );
#endif
	
	// Slide along the current contact points to fix bouncing problems
	Vector velocity;
	AngularImpulse angVel;
	pObject->GetVelocity( &velocity, &angVel );
	PhysComputeSlideDirection( pObject, velocity, angVel, &velocity, &angVel, GetLoadWeight() );
	pObject->SetVelocityInstantaneous( &velocity, NULL );

	linear.Init();
	angular.Init();
	m_errorTime += deltaTime;

	return SIM_LOCAL_ACCELERATION;
}

float CGrabController::GetSavedMass( IPhysicsObject *pObject )
{
	CBaseEntity *pHeld = m_attachedEntity;
	if ( pHeld )
	{
		if ( pObject->GetGameData() == (void*)pHeld )
		{
			IPhysicsObject *pList[VPHYSICS_MAX_OBJECT_LIST_COUNT];
			int count = pHeld->VPhysicsGetObjectList( pList, ARRAYSIZE(pList) );
			for ( int i = 0; i < count; i++ )
			{
				if ( pList[i] == pObject )
					return m_savedMass[i];
			}
		}
	}
	return 0.0f;
}

SH_DECL_MANUALHOOK0_void(GenericDtor, 0, 0, 0)
SH_DECL_MANUALHOOK4_void(Use, 0, 0, 0, CBaseEntity *, CBaseEntity *, USE_TYPE, float)
SH_DECL_MANUALHOOK1(OnControls, 0, 0, 0, bool, CBaseEntity *)
SH_DECL_MANUALHOOK1_void(VPhysicsUpdate, 0, 0, 0, IPhysicsObject *)
SH_DECL_MANUALHOOK1_void(VPhysicsShadowUpdate, 0, 0, 0, IPhysicsObject *)
SH_DECL_MANUALHOOK0_void(OnRestore, 0, 0, 0)

class CPlayerPickupController : public CBaseEntity
{
public:
	struct vars_t
	{
		CGrabController m_grabController{};
		CBasePlayer *m_pPlayer = nullptr;
	};
	
	unsigned char *vars_ptr()
	{ return (((unsigned char *)this) + sizeofCBaseEntity); }
	vars_t &getvars()
	{ return *(vars_t *)vars_ptr(); }
	
	bool HookOnControls(CBaseEntity *) { RETURN_META_VALUE(MRES_SUPERCEDE, true); }
	void HookVPhysicsUpdate(IPhysicsObject *) { RETURN_META(MRES_SUPERCEDE); }
	void HookVPhysicsShadowUpdate(IPhysicsObject *) { RETURN_META(MRES_SUPERCEDE); }
	void HookOnRestore() { getvars().m_grabController.OnRestore(); RETURN_META(MRES_SUPERCEDE); }
	
	void HookEntityDtor()
	{
		CBaseEntity *pEntity = META_IFACEPTR(CBaseEntity);
		getvars().~vars_t();
		SH_REMOVE_MANUALHOOK(GenericDtor, pEntity, SH_MEMBER(this, &CPlayerPickupController::HookEntityDtor), false);
		SH_REMOVE_MANUALHOOK(Use, pEntity, SH_MEMBER(this, &CPlayerPickupController::HookUse), false);
		SH_REMOVE_MANUALHOOK(OnControls, pEntity, SH_MEMBER(this, &CPlayerPickupController::HookOnControls), false);
		SH_REMOVE_MANUALHOOK(VPhysicsUpdate, pEntity, SH_MEMBER(this, &CPlayerPickupController::HookVPhysicsUpdate), false);
		SH_REMOVE_MANUALHOOK(VPhysicsShadowUpdate, pEntity, SH_MEMBER(this, &CPlayerPickupController::HookVPhysicsShadowUpdate), false);
		SH_REMOVE_MANUALHOOK(OnRestore, pEntity, SH_MEMBER(this, &CPlayerPickupController::HookOnRestore), false);
		RETURN_META(MRES_IGNORED);
	}
	
	static CPlayerPickupController *create()
	{
		CPlayerPickupController *bytes = (CPlayerPickupController *)calloc(1, sizeofCBaseEntity + sizeof(vars_t));
		call_mfunc<void, CBaseEntity, bool>(bytes, CBaseEntityCTOR, true);
		new (bytes->vars_ptr()) vars_t();
		SH_ADD_MANUALHOOK(GenericDtor, bytes, SH_MEMBER(bytes, &CPlayerPickupController::HookEntityDtor), false);
		SH_ADD_MANUALHOOK(Use, bytes, SH_MEMBER(bytes, &CPlayerPickupController::HookUse), false);
		SH_ADD_MANUALHOOK(OnControls, bytes, SH_MEMBER(bytes, &CPlayerPickupController::HookOnControls), false);
		SH_ADD_MANUALHOOK(VPhysicsUpdate, bytes, SH_MEMBER(bytes, &CPlayerPickupController::HookVPhysicsUpdate), false);
		SH_ADD_MANUALHOOK(VPhysicsShadowUpdate, bytes, SH_MEMBER(bytes, &CPlayerPickupController::HookVPhysicsShadowUpdate), false);
		SH_ADD_MANUALHOOK(OnRestore, bytes, SH_MEMBER(bytes, &CPlayerPickupController::HookOnRestore), false);
		return bytes;
	}
	
	bool IsHoldingEntity( CBaseEntity *pEnt );
	CGrabController &GetGrabController() { return getvars().m_grabController; }
	
	void Init( CBasePlayer *pPlayer, CBaseEntity *pObject );
	void Shutdown( bool bThrown = false );
	
	void HookUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
};

bool CPlayerPickupController::IsHoldingEntity( CBaseEntity *pEnt )
{
	return ( getvars().m_grabController.GetAttached() == pEnt );
}

void Pickup_OnPhysGunDrop( CBaseEntity *pDroppedObject, CBasePlayer *pPlayer, PhysGunDrop_t Reason )
{
	(void_to_func<void(*)(CBaseEntity *, CBasePlayer *, PhysGunDrop_t)>(Pickup_OnPhysGunDropPtr))(pDroppedObject, pPlayer, Reason);
}

void CPlayerPickupController::Shutdown( bool bThrown )
{
	CBaseEntity *pObject = getvars().m_grabController.GetAttached();

	bool bClearVelocity = false;
	if ( !bThrown && pObject && pObject->VPhysicsGetObject() && pObject->VPhysicsGetObject()->GetContactPoint(NULL,NULL) )
	{
		bClearVelocity = true;
	}

	getvars().m_grabController.DetachEntity( bClearVelocity );
	// NVNT if we have a player, issue a zero constant force message
#if defined( WIN32 ) && !defined( _X360 )
	if(getvars().m_pPlayer)
		HapticSetConstantForce(m_pPlayer,Vector(0,0,0));
#endif
	if ( pObject != NULL )
	{
		Pickup_OnPhysGunDrop( pObject, getvars().m_pPlayer, bThrown ? THROWN_BY_PLAYER : DROPPED_BY_PLAYER );
	}

	if ( getvars().m_pPlayer )
	{
		getvars().m_pPlayer->SetUseEntity( NULL );
		if ( getvars().m_pPlayer->GetActiveWeapon() )
		{
			if ( !getvars().m_pPlayer->GetActiveWeapon()->Deploy() )
			{
				// We tried to restore the player's weapon, but we couldn't.
				// This usually happens when they're holding an empty weapon that doesn't
				// autoswitch away when out of ammo. Switch to next best weapon.
				
				getvars().m_pPlayer->SwitchToNextBestWeapon( NULL );
			}
		}

		getvars().m_pPlayer->GetHideHud() &= ~HIDEHUD_WEAPONSELECTION;
	}
	Remove();
}

void Pickup_OnPhysGunPickup( CBaseEntity *pPickedUpObject, CBasePlayer *pPlayer, PhysGunPickup_t reason )
{
	(void_to_func<void(*)(CBaseEntity *, CBasePlayer *, PhysGunPickup_t)>(Pickup_OnPhysGunPickupPtr))(pPickedUpObject, pPlayer, reason);
}

void CPlayerPickupController::Init( CBasePlayer *pPlayer, CBaseEntity *pObject )
{
	// Holster player's weapon
	if ( pPlayer->GetActiveWeapon() )
	{
		if ( !pPlayer->GetActiveWeapon()->CanHolster() || !pPlayer->GetActiveWeapon()->Holster() )
		{
			Shutdown();
			return;
		}
	}

	// If the target is debris, convert it to non-debris
	if ( pObject->GetCollisionGroup() == COLLISION_GROUP_DEBRIS )
	{
		// Interactive debris converts back to debris when it comes to rest
		pObject->SetCollisionGroup( COLLISION_GROUP_INTERACTIVE_DEBRIS );
	}

	// done so I'll go across level transitions with the player
	SetParent( pPlayer );
	getvars().m_grabController.SetIgnorePitch( true );
	getvars().m_grabController.SetAngleAlignment( DOT_30DEGREE );
	getvars().m_pPlayer = pPlayer;
	IPhysicsObject *pPhysics = pObject->VPhysicsGetObject();
	
	Pickup_OnPhysGunPickup( pObject, getvars().m_pPlayer, PICKED_UP_BY_PLAYER );
	
	getvars().m_grabController.AttachEntity( pPlayer, pObject, pPhysics, false, vec3_origin, false );
	// NVNT apply a downward force to simulate the mass of the held object.
#if defined( WIN32 ) && !defined( _X360 )
	HapticSetConstantForce(getvars().m_pPlayer,clamp(getvars().m_grabController.GetLoadWeight()*0.1,1,6)*Vector(0,-1,0));
#endif
	
	getvars().m_pPlayer->GetHideHud() |= HIDEHUD_WEAPONSELECTION;
	getvars().m_pPlayer->SetUseEntity( this );
}

const QAngle& CCollisionProperty::GetCollisionAngles() const
{
	if ( IsBoundsDefinedInEntitySpace() )
	{
		return m_pOuter->GetAbsAngles();
	}

	return vec3_angle;
}

const matrix3x4_t& CCollisionProperty::CollisionToWorldTransform() const
{
	return call_mfunc<const matrix3x4_t &>(this, CCollisionPropertyCollisionToWorldTransform);
}

const Vector& CCollisionProperty::GetCollisionOrigin() const
{
	return m_pOuter->GetAbsOrigin();
}

void CCollisionProperty::CollisionAABBToWorldAABB( const Vector &entityMins, 
	const Vector &entityMaxs, Vector *pWorldMins, Vector *pWorldMaxs ) const
{
	if ( !IsBoundsDefinedInEntitySpace() || (GetCollisionAngles() == vec3_angle) )
	{
		VectorAdd( entityMins, GetCollisionOrigin(), *pWorldMins );
		VectorAdd( entityMaxs, GetCollisionOrigin(), *pWorldMaxs );
	}
	else
	{
		TransformAABB( CollisionToWorldTransform(), entityMins, entityMaxs, *pWorldMins, *pWorldMaxs );
	}
}

// when looking level, hold bottom of object 8 inches below eye level
#define PLAYER_HOLD_LEVEL_EYES	-8

// when looking down, hold bottom of object 0 inches from feet
#define PLAYER_HOLD_DOWN_FEET	2

// when looking up, hold bottom of object 24 inches above eye level
#define PLAYER_HOLD_UP_EYES		24

// use a +/-30 degree range for the entire range of motion of pitch
#define PLAYER_LOOK_PITCH_RANGE	30

// player can reach down 2ft below his feet (otherwise he'll hold the object above the bottom)
#define PLAYER_REACH_DOWN_DISTANCE	24

static void ComputePlayerMatrix( CBasePlayer *pPlayer, matrix3x4_t &out )
{
	if ( !pPlayer )
		return;

	QAngle angles = pPlayer->EyeAngles();
	Vector origin = pPlayer->EyePosition();
	
	// 0-360 / -180-180
	//angles.x = init ? 0 : AngleDistance( angles.x, 0 );
	//angles.x = clamp( angles.x, -PLAYER_LOOK_PITCH_RANGE, PLAYER_LOOK_PITCH_RANGE );
	angles.x = 0;

	float feet = pPlayer->GetAbsOrigin().z + pPlayer->WorldAlignMins().z;
	float eyes = origin.z;
	float zoffset = 0;
	// moving up (negative pitch is up)
	if ( angles.x < 0 )
	{
		zoffset = RemapVal( angles.x, 0, -PLAYER_LOOK_PITCH_RANGE, PLAYER_HOLD_LEVEL_EYES, PLAYER_HOLD_UP_EYES );
	}
	else
	{
		zoffset = RemapVal( angles.x, 0, PLAYER_LOOK_PITCH_RANGE, PLAYER_HOLD_LEVEL_EYES, PLAYER_HOLD_DOWN_FEET + (feet - eyes) );
	}
	origin.z += zoffset;
	angles.x = 0;
	AngleMatrix( angles, origin, out );
}

#include <model_types.h>

bool StandardFilterRules( IHandleEntity *pHandleEntity, int fContentsMask )
{
	return (void_to_func<bool(*)(IHandleEntity *, int)>(StandardFilterRulesPtr))(pHandleEntity, fContentsMask);
}

CTraceFilterSkipTwoEntities::CTraceFilterSkipTwoEntities( const IHandleEntity *passentity, const IHandleEntity *passentity2, int collisionGroup ) :
	BaseClass( passentity, collisionGroup ), m_pPassEnt2(passentity2)
{
}

bool PassServerEntityFilter( const IHandleEntity *pTouch, const IHandleEntity *pPass ) 
{
	if ( !pPass )
		return true;

	if ( pTouch == pPass )
		return false;

	const CBaseEntity *pEntTouch = EntityFromEntityHandle( pTouch );
	const CBaseEntity *pEntPass = EntityFromEntityHandle( pPass );
	if ( !pEntTouch || !pEntPass )
		return true;

	// don't clip against own missiles
	if ( pEntTouch->GetOwnerEntity() == pEntPass )
		return false;
	
	// don't clip against owner
	if ( pEntPass->GetOwnerEntity() == pEntTouch )
		return false;	


	return true;
}

CTraceFilterSimple::CTraceFilterSimple( const IHandleEntity *passedict, int collisionGroup,
									   ShouldHitFunc_t pExtraShouldHitFunc )
{
	m_pPassEnt = passedict;
	m_collisionGroup = collisionGroup;
	m_pExtraShouldHitCheckFunction = pExtraShouldHitFunc;
}

bool CTraceFilterSimple::ShouldHitEntity( IHandleEntity *pHandleEntity, int contentsMask )
{
	return call_mfunc<bool, CTraceFilterSimple, IHandleEntity *, int>(this, CTraceFilterSimpleShouldHitEntity, pHandleEntity, contentsMask);
}

bool CTraceFilterSkipTwoEntities::ShouldHitEntity( IHandleEntity *pHandleEntity, int contentsMask )
{
	Assert( pHandleEntity );
	if ( !PassServerEntityFilter( pHandleEntity, m_pPassEnt2 ) )
		return false;

	return BaseClass::ShouldHitEntity( pHandleEntity, contentsMask );
}

bool CGrabController::UpdateObject( CBasePlayer *pPlayer, float flError )
{
 	CBaseEntity *pEntity = GetAttached();
	if ( !pEntity || ComputeError() > flError || pPlayer->GetGroundEntity() == pEntity || !pEntity->VPhysicsGetObject() )
	{
		return false;
	}

	//Adrian: Oops, our object became motion disabled, let go!
	IPhysicsObject *pPhys = pEntity->VPhysicsGetObject();
	if ( pPhys && pPhys->IsMoveable() == false )
	{
		return false;
	}

	Vector forward, right, up;
	QAngle playerAngles = pPlayer->EyeAngles();
	AngleVectors( playerAngles, &forward, &right, &up );
	
	float pitch = AngleDistance(playerAngles.x,0);

	if( !m_bAllowObjectOverhead )
	{
		playerAngles.x = clamp( pitch, -75, 75 );
	}
	else
	{
		playerAngles.x = clamp( pitch, -90, 75 );
	}

	
	
	// Now clamp a sphere of object radius at end to the player's bbox
	
	const QAngle &ent_angles = pEntity->GetAbsAngles();
	Vector radial = physcollision->CollideGetExtent( pPhys->GetCollide(), vec3_origin, ent_angles, -forward );
	Vector player2d = pPlayer->CollisionProp()->OBBMaxs();
	float playerRadius = player2d.Length2D();
	float radius = playerRadius + fabs(DotProduct( forward, radial ));

	float distance = 24 + ( radius * 2.0f );

	// Add the prop's distance offset
	distance += m_flDistanceOffset;

	Vector start = pPlayer->Weapon_ShootPosition();
	Vector end = start + ( forward * distance );

	trace_t	tr;
	CTraceFilterSkipTwoEntities traceFilter( pPlayer, pEntity, COLLISION_GROUP_NONE );
	Ray_t ray;
	ray.Init( start, end );
	enginetrace->TraceRay( ray, MASK_SOLID_BRUSHONLY, &traceFilter, &tr );

	if ( tr.fraction < 0.5 )
	{
		end = start + forward * (radius*0.5f);
	}
	else if ( tr.fraction <= 1.0f )
	{
		end = start + forward * ( distance - radius );
	}
	Vector playerMins, playerMaxs, nearest;
	pPlayer->CollisionProp()->WorldSpaceAABB( &playerMins, &playerMaxs );
	Vector playerLine = pPlayer->CollisionProp()->WorldSpaceCenter();
	CalcClosestPointOnLine( end, playerLine+Vector(0,0,playerMins.z), playerLine+Vector(0,0,playerMaxs.z), nearest, NULL );

	if( !m_bAllowObjectOverhead )
	{
		Vector delta = end - nearest;
		float len = VectorNormalize(delta);
		if ( len < radius )
		{
			end = nearest + radius * delta;
		}
	}

	//Show overlays of radius
	if ( g_debug_physcannon.GetBool() )
	{
		/*NDebugOverlay::Box( end, -Vector( 2,2,2 ), Vector(2,2,2), 0, 255, 0, true, 0 );

		NDebugOverlay::Box( GetAttached()->WorldSpaceCenter(), 
							-Vector( radius, radius, radius), 
							Vector( radius, radius, radius ),
							255, 0, 0,
							true,
							0.0f );*/
	}

	QAngle angles = TransformAnglesFromPlayerSpace( m_attachedAnglesPlayerSpace, pPlayer );
	
	// If it has a preferred orientation, update to ensure we're still oriented correctly.
	Pickup_GetPreferredCarryAngles( pEntity, pPlayer, pPlayer->EntityToWorldTransform(), angles );

	// We may be holding a prop that has preferred carry angles
	if ( m_bHasPreferredCarryAngles )
	{
		matrix3x4_t tmp;
		ComputePlayerMatrix( pPlayer, tmp );
		angles = TransformAnglesToWorldSpace( m_vecPreferredCarryAngles, tmp );
	}

	matrix3x4_t attachedToWorld;
	Vector offset;
	AngleMatrix( angles, attachedToWorld );
	VectorRotate( m_attachedPositionObjectSpace, attachedToWorld, offset );

	SetTargetPosition( end - offset, angles );

	return true;
}

#include <in_buttons.h>

void CPlayerPickupController::HookUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	if ( (CBasePlayer *)pActivator == getvars().m_pPlayer )
	{
		CBaseEntity *pAttached = getvars().m_grabController.GetAttached();

		// UNDONE: Use vphysics stress to decide to drop objects
		// UNDONE: Must fix case of forcing objects into the ground you're standing on (causes stress) before that will work
		if ( !pAttached || useType == USE_OFF || (getvars().m_pPlayer->GetButtons() & IN_ATTACK2) || getvars().m_grabController.ComputeError() > 12 )
		{
			Shutdown();
			RETURN_META(MRES_SUPERCEDE);
		}
		
		//Adrian: Oops, our object became motion disabled, let go!
		IPhysicsObject *pPhys = pAttached->VPhysicsGetObject();
		if ( pPhys && pPhys->IsMoveable() == false )
		{
			Shutdown();
			RETURN_META(MRES_SUPERCEDE);
		}

#if STRESS_TEST
		vphysics_objectstress_t stress;
		CalculateObjectStress( pPhys, pAttached, &stress );
		if ( stress.exertedStress > 250 )
		{
			Shutdown();
			RETURN_META(MRES_SUPERCEDE);
		}
#endif
		// +ATTACK will throw phys objects
		if ( getvars().m_pPlayer->GetButtons() & IN_ATTACK )
		{
			Shutdown( true );
			
			if(pPhys) {
				Vector vecLaunch;
				getvars().m_pPlayer->EyeVectors( &vecLaunch );
				// JAY: Scale this with mass because some small objects really go flying
				float massFactor = clamp( pPhys->GetMass(), 0.5, 15 );
				massFactor = RemapVal( massFactor, 0.5, 15, 0.5, 4 );
				vecLaunch *= player_throwforce.GetFloat() * massFactor;

				pPhys->ApplyForceCenter( vecLaunch );
				AngularImpulse aVel = RandomAngularImpulse( -10, 10 ) * massFactor;
				pPhys->ApplyTorqueCenter( aVel );
			}

			RETURN_META(MRES_SUPERCEDE);
		}

		if ( useType == USE_SET )
		{
			// update position
			getvars().m_grabController.UpdateObject( getvars().m_pPlayer, 12 );
		}
	}
	
	RETURN_META(MRES_SUPERCEDE);
}

CBaseEntity *GetPlayerHeldEntity( CBasePlayer *pPlayer )
{
	CBaseEntity *pObject = NULL;
	CPlayerPickupController *pPlayerPickupController = (CPlayerPickupController *)(CBaseEntity *)(pPlayer->GetUseEntity());

	if ( pPlayerPickupController )
	{
		pObject = pPlayerPickupController->GetGrabController().GetAttached();
	}

	return pObject;
}

float PlayerPickupGetHeldObjectMass( CBaseEntity *pPickupControllerEntity, IPhysicsObject *pHeldObject )
{
	float mass = 0.0f;
	CPlayerPickupController *pController = (CPlayerPickupController *)pPickupControllerEntity->IsPickupController();
	if ( pController )
	{
		CGrabController &grab = pController->GetGrabController();
		mass = grab.GetSavedMass( pHeldObject );
	}
	return mass;
}

bool PlayerPickupControllerIsHoldingEntity( CBaseEntity *pPickupControllerEntity, CBaseEntity *pHeldEntity )
{
	CPlayerPickupController *pController = (CPlayerPickupController *)pPickupControllerEntity->IsPickupController();

	return pController ? pController->IsHoldingEntity( pHeldEntity ) : false;
}

void PlayerPickupObject( CBasePlayer *pPlayer, CBaseEntity *pObject )
{
	//Don't pick up if we don't have a phys object.
	if ( pObject->VPhysicsGetObject() == NULL ) {
		return;
	}

	CPlayerPickupController *pController = (CPlayerPickupController *)servertools->CreateEntityByName( "player_pickup" );
	
	if ( !pController ) {
		return;
	}
	
	servertools->DispatchSpawn(pController);

	pController->Init( pPlayer, pObject );
}

void DoPickupObject(CBasePlayer *pPlayer, CBaseEntity *pObject, bool bLimitMassAndSize)
{
	if ( pPlayer->GetGroundEntity() == pObject ) {
		return;
	}
	
	if ( bLimitMassAndSize == true )
	{
		if ( CanPickupObject( pObject, 35, 128 ) == false ) {
			return;
		}
	}

	// Can't be picked up if NPCs are on me
	if ( pObject->HasNPCsOnIt() ) {
		return;
	}

	PlayerPickupObject( pPlayer, pObject );
}

void HookPickupObject(CBaseEntity *pObject, bool bLimitMassAndSize)
{
	CBasePlayer *pPlayer = META_IFACEPTR(CBasePlayer);
	DoPickupObject(pPlayer, pObject, bLimitMassAndSize);
	RETURN_META(MRES_SUPERCEDE);
}

float HookGetHeldObjectMass(IPhysicsObject *pHeldObject)
{
	CBasePlayer *pPlayer = META_IFACEPTR(CBasePlayer);
	float mass = PlayerPickupGetHeldObjectMass( pPlayer->GetUseEntity(), pHeldObject );
	RETURN_META_VALUE(MRES_SUPERCEDE, mass);
}

void HookForceDropOfCarriedPhysObjects(CBaseEntity *pOnlyIfHoldingThis)
{
	CBasePlayer *pPlayer = META_IFACEPTR(CBasePlayer);
	pPlayer->ClearUseEntity();
	RETURN_META(MRES_SUPERCEDE);
}

void Sample::OnClientPutInServer(int client)
{
	CBaseEntity *pEntity = gamehelpers->ReferenceToEntity(client);
	if(pEntity) {
		SH_ADD_MANUALHOOK(PickupObject, pEntity, SH_STATIC(HookPickupObject), false);
		SH_ADD_MANUALHOOK(GetHeldObjectMass, pEntity, SH_STATIC(HookGetHeldObjectMass), false);
		SH_ADD_MANUALHOOK(ForceDropOfCarriedPhysObjects, pEntity, SH_STATIC(HookForceDropOfCarriedPhysObjects), false);
	}
}

void Sample::OnClientDisconnecting(int client)
{
	CBaseEntity *pEntity = gamehelpers->ReferenceToEntity(client);
	if(pEntity) {
		SH_REMOVE_MANUALHOOK(PickupObject, pEntity, SH_STATIC(HookPickupObject), false);
		SH_REMOVE_MANUALHOOK(GetHeldObjectMass, pEntity, SH_STATIC(HookGetHeldObjectMass), false);
		SH_REMOVE_MANUALHOOK(ForceDropOfCarriedPhysObjects, pEntity, SH_STATIC(HookForceDropOfCarriedPhysObjects), false);
	}
}

IEntityFactoryDictionary *dictionary = nullptr;

static class PickupFactory : public IEntityFactory
{
public:
	virtual IServerNetworkable *Create( const char *pClassName )
	{
		CBaseEntity *pEntity = (CBaseEntity *)CPlayerPickupController::create();
		call_vfunc<void, CBaseEntity, const char *>(pEntity, CBaseEntityPostConstructor, pClassName);
		return pEntity->GetNetworkable();
	}
	virtual void Destroy( IServerNetworkable *pNetworkable )
	{
		if(pNetworkable) {
			pNetworkable->Release();
		}
	}
	virtual size_t GetEntitySize() { return sizeofCBaseEntity; }
} pickfac{};

IPhysics *phys = nullptr;
ConVar *tf_allow_player_use = nullptr;

bool Sample::SDK_OnMetamodLoad(ISmmAPI *ismm, char *error, size_t maxlen, bool late)
{
	gpGlobals = ismm->GetCGlobals();
	GET_V_IFACE_ANY(GetServerFactory, servertools, IServerTools, VSERVERTOOLS_INTERFACE_VERSION)
	GET_V_IFACE_CURRENT(GetEngineFactory, modelinfo, IVModelInfo, VMODELINFO_SERVER_INTERFACE_VERSION)
	GET_V_IFACE_ANY(GetEngineFactory, engine, IVEngineServer, INTERFACEVERSION_VENGINESERVER)
	GET_V_IFACE_ANY(GetEngineFactory, enginetrace, IEngineTrace, INTERFACEVERSION_ENGINETRACE_SERVER)
	GET_V_IFACE_ANY(GetEngineFactory, staticpropmgr, IStaticPropMgrServer, INTERFACEVERSION_STATICPROPMGR_SERVER)
	GET_V_IFACE_CURRENT(GetEngineFactory, icvar, ICvar, CVAR_INTERFACE_VERSION);
	GET_V_IFACE_CURRENT(GetPhysicsFactory, phys, IPhysics, VPHYSICS_INTERFACE_VERSION);
	GET_V_IFACE_CURRENT(GetPhysicsFactory, physcollision, IPhysicsCollision, VPHYSICS_COLLISION_INTERFACE_VERSION);
	g_pCVar = icvar;
	ConVar_Register(0, this);
	dictionary = servertools->GetEntityFactoryDictionary();
	dictionary->InstallFactory(&pickfac, "player_pickup");
	g_SMAPI->AddListener(g_PLAPI, this);
	tf_allow_player_use = g_pCVar->FindVar("tf_allow_player_use");
	return true;
}

bool Sample::RegisterConCommandBase(ConCommandBase *pCommand)
{
	META_REGCVAR(pCommand);
	return true;
}

void Sample::OnLevelInit(const char *pMapName, char const *pMapEntities, char const *pOldLevel, char const *pLandmarkName, bool loadGame, bool background)
{
	physenv = phys->GetActiveEnvironmentByIndex(0);
	tf_allow_player_use->SetValue(true);
}

IGameConfig *g_pGameConf = nullptr;

CDetour *pCanPickupObject = nullptr;

DETOUR_DECL_STATIC3(DetourCanPickupObject, bool, CBaseEntity *, pObject, float, massLimit, float, sizeLimit)
{
	return CanPickupObject(pObject, massLimit, sizeLimit);
}

bool Sample::SDK_OnLoad(char *error, size_t maxlen, bool late)
{
	gameconfs->LoadGameConfigFile("tfpickup", &g_pGameConf, error, maxlen);
	
	CDetourManager::Init(g_pSM->GetScriptingEngine(), g_pGameConf);
	
	int offset = -1;
	g_pGameConf->GetOffset("CBasePlayer::PickupObject", &offset);
	SH_MANUALHOOK_RECONFIGURE(PickupObject, offset, 0, 0);
	
	g_pGameConf->GetOffset("CBaseEntity::Use", &CBaseEntityUse);
	SH_MANUALHOOK_RECONFIGURE(Use, CBaseEntityUse, 0, 0);
	
	g_pGameConf->GetOffset("CBaseEntity::OnControls", &offset);
	SH_MANUALHOOK_RECONFIGURE(OnControls, offset, 0, 0);
	
	g_pGameConf->GetOffset("CBaseEntity::VPhysicsUpdate", &offset);
	SH_MANUALHOOK_RECONFIGURE(VPhysicsUpdate, offset, 0, 0);
	
	g_pGameConf->GetOffset("CBaseEntity::VPhysicsShadowUpdate", &offset);
	SH_MANUALHOOK_RECONFIGURE(VPhysicsShadowUpdate, offset, 0, 0);
	
	g_pGameConf->GetOffset("CBaseEntity::OnRestore", &offset);
	SH_MANUALHOOK_RECONFIGURE(OnRestore, offset, 0, 0);
	
	g_pGameConf->GetOffset("CBasePlayer::GetHeldObjectMass", &offset);
	SH_MANUALHOOK_RECONFIGURE(GetHeldObjectMass, offset, 0, 0);
	
	g_pGameConf->GetOffset("CBasePlayer::ForceDropOfCarriedPhysObjects", &offset);
	SH_MANUALHOOK_RECONFIGURE(ForceDropOfCarriedPhysObjects, offset, 0, 0);
	
	pCanPickupObject = DETOUR_CREATE_STATIC(DetourCanPickupObject, "CBasePlayer::CanPickupObject")
	pCanPickupObject->EnableDetour();
	
	g_pGameConf->GetOffset("CBaseEntity::PostConstructor", &CBaseEntityPostConstructor);
	g_pGameConf->GetOffset("CBaseEntity::VPhysicsGetObjectList", &CBaseEntityVPhysicsGetObjectList);
	g_pGameConf->GetOffset("CBaseEntity::EyeAngles", &CBaseEntityEyeAngles);
	g_pGameConf->GetOffset("CBaseEntity::WorldSpaceCenter", &CBaseEntityWorldSpaceCenter);
	g_pGameConf->GetOffset("CBaseEntity::SetParent", &CBaseEntitySetParent);
	g_pGameConf->GetOffset("CBaseEntity::EyePosition", &CBaseEntityEyePosition);
	g_pGameConf->GetOffset("CBasePlayer::Weapon_ShootPosition", &CBasePlayerWeapon_ShootPosition);
	g_pGameConf->GetOffset("CBaseCombatWeapon::Holster", &CBaseCombatWeaponHolster);
	g_pGameConf->GetOffset("CBaseCombatWeapon::Deploy", &CBaseCombatWeaponDeploy);
	g_pGameConf->GetOffset("CBaseCombatWeapon::CanHolster", &CBaseCombatWeaponCanHolster);
	g_pGameConf->GetOffset("CBreakableProp::HasInteraction", &CBreakablePropHasInteraction);
	g_pGameConf->GetOffset("sizeof(CBaseEntity)", &sizeofCBaseEntity);
	
	g_pGameConf->GetMemSig("CBaseEntity::CBaseEntity", &CBaseEntityCTOR);
	g_pGameConf->GetMemSig("CBaseEntity::HasNPCsOnIt", &CBaseEntityHasNPCsOnIt);
	g_pGameConf->GetMemSig("CBaseEntity::CalcAbsolutePosition", &CBaseEntityCalcAbsolutePosition);
	g_pGameConf->GetMemSig("Pickup_GetPreferredCarryAngles", &Pickup_GetPreferredCarryAnglesPtr);
	g_pGameConf->GetMemSig("Pickup_OnPhysGunDrop", &Pickup_OnPhysGunDropPtr);
	g_pGameConf->GetMemSig("Pickup_OnPhysGunPickup", &Pickup_OnPhysGunPickupPtr);
	g_pGameConf->GetMemSig("PhysComputeSlideDirection", &PhysComputeSlideDirectionPtr);
	g_pGameConf->GetMemSig("PhysForceClearVelocity", &PhysForceClearVelocityPtr);
	g_pGameConf->GetMemSig("CBaseCombatCharacter::SwitchToNextBestWeapon", &CBaseCombatCharacterSwitchToNextBestWeapon);
	g_pGameConf->GetMemSig("StandardFilterRules", &StandardFilterRulesPtr);
	g_pGameConf->GetMemSig("CTraceFilterSimple::ShouldHitEntity", &CTraceFilterSimpleShouldHitEntity);
	g_pGameConf->GetMemSig("CCollisionProperty::CollisionToWorldTransform", &CCollisionPropertyCollisionToWorldTransform);
	g_pGameConf->GetMemSig("PhysicsImpactSound", &PhysicsImpactSoundPtr);
	g_pGameConf->GetMemSig("PhysRemoveShadow", &PhysRemoveShadowPtr);
	
	sm_sendprop_info_t info{};
	gamehelpers->FindSendPropInfo("CBaseEntity", "m_hGroundEntity", &info);
	m_hGroundEntityOffset = info.actual_offset;
	
	gamehelpers->FindSendPropInfo("CBasePlayer", "m_hUseEntity", &info);
	m_hUseEntityOffset = info.actual_offset;
	
	gamehelpers->FindSendPropInfo("CBaseEntity", "m_hOwnerEntity", &info);
	m_hOwnerEntityOffset = info.actual_offset;
	
	gamehelpers->FindSendPropInfo("CBasePlayer", "m_iHideHUD", &info);
	m_iHideHUDOffset = info.actual_offset;
	
	gamehelpers->FindSendPropInfo("CBasePlayer", "m_hActiveWeapon", &info);
	m_hActiveWeaponOffset = info.actual_offset;
	
	g_pEntityList = reinterpret_cast<CBaseEntityList *>(gamehelpers->GetGlobalEntityList());
	
	playerhelpers->AddClientListener(this);
	
	return true;
}

void Sample::SDK_OnUnload()
{
	if(pCanPickupObject) {
		pCanPickupObject->Destroy();
	}
	gameconfs->CloseGameConfigFile(g_pGameConf);
	playerhelpers->RemoveClientListener(this);
}
