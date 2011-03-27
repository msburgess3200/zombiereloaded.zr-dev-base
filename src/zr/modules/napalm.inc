/*
 * ============================================================================
 *
 *  Zombie:Reloaded
 *
 *  File:          napalm.inc
 *  Type:          Module
 *  Description:   Light players on fire with "napalm grenades"
 *
 *  Copyright (C) 2009-2010  Greyscale, Richard Helgeby
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * ============================================================================
 */

/**
 * CHECK CLASS SETTINGS
 */

#include "zr/libraries/damagelib"
#include "zr/libraries/offsetlib"

/**
 * This module's identifier.
 */
new Module:g_moduleNapalm;

/**
 * Napalm cvars.
 */
new Handle:g_hCvarNapalm;
new Handle:g_hCvarNapalmFF;
new Handle:g_hCvarNapalmIgnite;
new Handle:g_hCvarNapalmTimeReset;
new Handle:g_hCvarNapalmDouse;

/**
 * Register this module.
 */
Napalm_Register()
{
    // Define all the module's data as layed out by enum ModuleData in project.inc.
    new moduledata[ModuleData];
    
    moduledata[ModuleData_Disabled] = false;
    moduledata[ModuleData_Hidden] = false;
    strcopy(moduledata[ModuleData_FullName], MM_DATA_FULLNAME, "Napalm");
    strcopy(moduledata[ModuleData_ShortName], MM_DATA_SHORTNAME, "napalm");
    strcopy(moduledata[ModuleData_Description], MM_DATA_DESCRIPTION, "Light players on fire with \"napalm grenades\"");
    //moduledata[ModuleData_Dependencies][0] = ;
    // End the last one with INVALID_MODULE.
    //moduledata[ModuleData_Dependencies][x] = INVALID_MODULE;
    
    // Send this array of data to the module manager.
    g_moduleNapalm = ModuleMgr_Register(moduledata);
    
    // Register the OnEventsRegister event to register all events in it.
    EventMgr_RegisterEvent(g_moduleNapalm, "Event_OnEventsRegister",     "Napalm_OnEventsRegister");
    
    // Register config file(s) that this module will use.
    // ConfigMgr_Register(g_moduleNapalm, "Napalm_OnConfigReload", "configs/<projectshortname>/<config>.txt");
}

/**
 * Register all events here.
 */
public Napalm_OnEventsRegister()
{
    // Register all the events needed for this module.
    //EventMgr_RegisterEvent(g_moduleNapalm, Event_OnEventsReady,          "Napalm_OnEventsReady");
    //EventMgr_RegisterEvent(g_moduleNapalm, Event_OnAllPluginsLoaded,      "Napalm_OnAllPluginsLoaded");
    //EventMgr_RegisterEvent(g_moduleNapalm, Event_OnPluginEnd,             "Napalm_OnPluginEnd");
    //EventMgr_RegisterEvent(g_moduleNapalm, Event_OnAllModulesLoaded,      "Napalm_OnAllModulesLoaded");
    //EventMgr_RegisterEvent(g_moduleNapalm, Event_OnModuleEnable,          "Napalm_OnModuleEnable");
    //EventMgr_RegisterEvent(g_moduleNapalm, Event_OnMyModuleEnable,        "Napalm_OnMyModuleEnable");
    //EventMgr_RegisterEvent(g_moduleNapalm, Event_OnModuleDisable,         "Napalm_OnModuleDisable");
    //EventMgr_RegisterEvent(g_moduleNapalm, Event_OnMyModuleDisable,       "Napalm_OnMyModuleDisable");
    //EventMgr_RegisterEvent(g_moduleNapalm, Event_OnMapStart,              "Napalm_OnMapStart");
    //EventMgr_RegisterEvent(g_moduleNapalm, Event_OnMapEnd,                "Napalm_OnMapEnd");
    //EventMgr_RegisterEvent(g_moduleNapalm, Event_OnAutoConfigsBuffered,   "Napalm_OnAutoConfigsBuffered");
    //EventMgr_RegisterEvent(g_moduleNapalm, Event_OnConfigsExecuted,       "Napalm_OnConfigsExecuted");
    //EventMgr_RegisterEvent(g_moduleNapalm, Event_OnClientPutInServer,     "Napalm_OnClientPutInServer");
    //EventMgr_RegisterEvent(g_moduleNapalm, Event_OnClientDisconnect,      "Napalm_OnClientDisconnect");
    
    #if defined PROJECT_GAME_CSS
    
    EventMgr_RegisterEvent(g_moduleNapalm, "Event_PlayerHurt", "Napalm_PlayerHurt");
    EventMgr_RegisterEvent(g_moduleNapalm, "Event_WeaponFire", "Napalm_WeaponFire");
    EventMgr_RegisterEvent(g_moduleNapalm, "Hook_OnTakeDamage", "Napalm_OnTakeDamage");
    
    #endif
}

/**
 * Plugin is loading.
 */
Napalm_OnPluginStart()
{
    // Register the module.
    Napalm_Register();
    
    // Create cvars.
    g_hCvarNapalm =             Project_CreateConVar("napalm", "1", "Enable napalm grenades.");
    g_hCvarNapalmFF =           Project_CreateConVar("napalm_ff", "0", "Allow humans to light teammates on fire. [Dependency: mp_friendlyfire]");
    g_hCvarNapalmIgnite =       Project_CreateConVar("napalm_ignite", "1", "Ignite grenade in mid-air after player throws it. [Dependency: Human Attribute 'has_napalm']");
    g_hCvarNapalmTimeReset =    Project_CreateConVar("napalm_time_reset", "1", "The burn timer is reset when being naded multiple times. [0: Original burn timer is used.]");
    g_hCvarNapalmDouse =        Project_CreateConVar("napalm_douse", "0", "Minimum water-saturation before any burning player is extinguished. ['0' = Disabled | '1' = Feet | '2' = Waist | '3' = Full submersion]");
}

/**
 * Client is being shot.  Damage is not inflicted yet, but blood is emitted from the victim at this point.
 * 
 * @param victim        The client being attacked.
 * @param attacker      The client attacking the victim.
 * @param damage        The amount of damage inflicted.
 * @param damagetype    A bit indicating type of damage that was being inflicted.
 * 
 * @return              Hook action.  See include/core.inc.
 */
public Action:Napalm_OnTakeDamage(victim, &attacker, &Float:damage, &damagetype)
{
    if (!GetConVarBool(g_hCvarNapalm))
        return Plugin_Continue;
    
    // Client was damaged by fire.
    if (damagetype & UTILS_DMG_BURN)
    {
        // Only take action if it isn't disabled, or the option is valid.
        new douse = GetConVarInt(g_hCvarNapalmDouse);
        if (douse > OFFS_WATERLEVEL_DRY && douse <= OFFS_WATERLEVEL_FULL)
        {
            // If the victim water-level is equal or higher than the given, then we want to extinguish the flame.
            if (OffsLib_GetClientWaterLevel(victim) >= douse)
            {
                // Put the fire out.
                
                //ExtinguishEntity(victim);  <-- Don't use this.  Takes off the FL_ONFIRE flag, but flame doesn't get extinguished.
                
                // This works.
                new fire = GetEntPropEnt(victim, Prop_Data, "m_hEffectEntity");
                if (IsValidEntity(fire))
                {
                    // Make sure the entity is a flame, so we can extinguish it.
                    decl String:classname[64];
                    GetEdictClassname(fire, classname, sizeof(classname));
                    if (StrEqual(classname, "entityflame", false))
                    {
                        SetEntPropFloat(fire, Prop_Data, "m_flLifetime", 0.0);
                    }
                    // Log what entity was in that property, for future reference.
                    else
                    {
                        LogMgr_Print(g_moduleNapalm, LogType_Normal, "Douse", "Found unexpected entity in prop \"m_flLifetime\": \"%s\"", classname);
                    }
                }
            }
        }
    }
    
    // Let the damage happen.
    return Plugin_Continue;
}

/**
 * Client has been damaged.
 * 
 * @param victim        The index of the hurt client.
 * @param attacker      The index of the attacking client.
 * @param health        How much health the client has after the damage.
 * @param armor         How much armor the client has after the damage.
 * @param weapon        The weapon classname used to hurt the victim. (No weapon_ prefix)
 * @param dmg_health    The amount of health the victim lost.
 * @param dmg_armor     The amount of armor the victim lost.
 * @param hitgroup      The hitgroup index of the victim that was damaged.
 */
public Napalm_PlayerHurt(victim, attacker, health, armor, const String:weapon[], dmg_health, dmg_armor, hitgroup)
{
    if (!GetConVarBool(g_hCvarNapalm))
        return;
    
    if (attacker == 0)
        return;
    
    // Check friendlyfire rules.
    if (!GetConVarBool(g_hCvarNapalmFF) && !TLib_IsClientZombie(victim))
        return;
    
    // If napalm time is invalid or 0, then stop.
    //new Float:napalm_time = ClassGetNapalmTime(victim);
    new Float:burntime = 10.0;
    //if (napalm_time <= 0.0)
        //return;
    // TODO
    // If the attacker can't throw napalm grenades, then stop.
    //if (!ClassGetHasNapalm(attacker))
        //return;
    
    #if defined PROJECT_GAME_CSS
    
    // If weapon is a grenade, then ignite player.
    if (StrEqual(weapon, "hegrenade", false))
    {
        new bool:reset = GetConVarBool(g_hCvarNapalmTimeReset);
        new flags = GetEntityFlags(victim);
        
        if (reset || ~flags & FL_ONFIRE)
        {
            // This stops the fire before re-ignition.
            ExtinguishEntity(victim);
            
            // Ignite client.
            IgniteEntity(victim, burntime);
        }
    }
    
    #endif
}

/**
 * Client has fired a weapon.
 * 
 * @param client        The client index.
 * @param weapon        The weapon classname fired. (No weapon_ prefix)
 */
public Napalm_WeaponFire(client, const String:weapon[])
{
    if (!GetConVarBool(g_hCvarNapalm))
        return;
    
    // Check if a grenade is being thrown.
    if (!StrEqual(weapon, "hegrenade", false))
        return;
    
    // If grenade fire is disabled, then stop.
    new bool:ignite = GetConVarBool(g_hCvarNapalmIgnite);
    if (!ignite)
        return;
    
    // If human class can't throw napalm grenades, then stop.
    //if (!ClassGetHasNapalm(client))
        //return;
    
    CreateTimer(0.2, NapalmIgniteGrenade);
}

/**
 * Timer callback, ignite's all hegrenade projectiles.
 * 
 * @param timer     The timer handle.
 */   
public Action:NapalmIgniteGrenade(Handle:timer)
{
    decl String:classname[64];
    
    // Get max entities.
    new maxentities = GetMaxEntities();
    
    // x = entity index.
    for (new x = 0; x <= maxentities; x++)
    {
        // If entity is invalid, then stop.
        if(!IsValidEntity(x) || !IsValidEdict(x))
        {
            continue;
        }
        
        GetEdictClassname(x, classname, sizeof(classname));
        if(StrEqual(classname, "hegrenade_projectile"))
            IgniteEntity(x, 3.0);
    }
}