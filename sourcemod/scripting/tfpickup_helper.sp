#include <sourcemod>
#include <sdkhooks>
#include <vphysics>

//TODO!!! pickup tf_dropped_weapon, lunchboxes

public void OnEntityCreated(int entity, const char[] classname)
{
	if(StrEqual(classname, "tf_projectile_pipe_remote")) {
		SDKHook(entity, SDKHook_Use, OnPipeUse);
	}
}

Action OnPipeUse(int entity, int activator, int caller, UseType type, float value)
{
	if(type == Use_Toggle) {
		Phys_EnableMotion(entity, true);
	}
	return Plugin_Continue;
}