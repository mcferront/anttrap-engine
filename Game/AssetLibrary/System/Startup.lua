do
local handle;
local console;
local game;
function Startup()

   ExecuteLuaAsset("Console.lua")
   console = Node_Create(Id_Create(), "Console");
   local lua = console:AddComponent(Id_Create(), "ScriptComponent");
   lua:Create("Console.lua");
   console:AddToScene();
   console:Bind();   

   handle = LoadResources("Game.resources");
   ExecuteLuaAsset("Game.lua");
   game = Node_Create(Id_Create(), "Game");
   lua = game:AddComponent(Id_Create(), "ScriptComponent");
   lua:Create("Game.lua");
   game:AddToScene();
   game:Bind();
end

function Shutdown()
    console:Destroy();
    game:Destroy();
    UnloadResources(handle);
end
end

--[[
if (Startup == nil) then
   Startup = {
      m_ParentResourceHandle = nil,
   };
end

function Startup:New(object)
   newObject = object or { };
   setmetatable(newObject, self);
   self.__index = self;

   return newObject;
end

do -- static variable scope
function Startup:Create()
   print("Startup:Create");

   --for some reason the first random call is always the same
   --so call it here when it doesn't matter
   Engine.Random(0, 1024);

   self.m_Database = Engine.Database( );
   self.m_Database:Create( );
   
   RequestResource( "AIBrain.lua", self.m_Database );
   RequestResource( "MWDroneAI.lua", self.m_Database );
   RequestResource( "MWTurretAI.lua", self.m_Database );
   RequestResource( "MWSoldierAI.lua", self.m_Database );
   RequestResource( "SoundEmitter.lua", self.m_Database );
   RequestResource( "Timeline.lua", self.m_Database );
   RequestResource( "GlobalScene.lua", self.m_Database );
   RequestResource( "Default.bmp", self.m_Database );
   RequestResource( "DefaultFont.bmp", self.m_Database );
   RequestResource( "DefaultFont.fontmap", self.m_Database );
   
   g_PhysicsWorld:SetGravity( Engine.Vector(0,-9.8 * 10,0) );
   
   --we don't need pickups checking for collision
   g_PhysicsWorld:DisableCollisionLayers(GetCollisionLayer("Pickup"), GetCollisionLayer("InnerWall"));
   g_PhysicsWorld:DisableCollisionLayers(GetCollisionLayer("Pickup"), GetCollisionLayer("OuterWall"));
   g_PhysicsWorld:DisableCollisionLayers(GetCollisionLayer("Pickup"), GetCollisionLayer("Pickup"));
   g_PhysicsWorld:DisableCollisionLayers(GetCollisionLayer("Pickup"), GetCollisionLayer("Hazard"));
   g_PhysicsWorld:DisableCollisionLayers(GetCollisionLayer("Pickup"), GetCollisionLayer("Default"));

   g_PhysicsWorld:DisableCollisionLayers(GetCollisionLayer("Hazard"), GetCollisionLayer("InnerWall"));
   g_PhysicsWorld:DisableCollisionLayers(GetCollisionLayer("Hazard"), GetCollisionLayer("OuterWall"));
   g_PhysicsWorld:DisableCollisionLayers(GetCollisionLayer("Hazard"), GetCollisionLayer("Default"));

   --player can pass inner wall for a 'safe' zone
   g_PhysicsWorld:DisableCollisionLayers(GetCollisionLayer("PlayerProxy"), GetCollisionLayer("InnerWall"));
   g_PhysicsWorld:DisableCollisionLayers(GetCollisionLayer("DroneProxy"), GetCollisionLayer("InnerWall"));
   g_PhysicsWorld:DisableCollisionLayers(GetCollisionLayer("DroneProxy"), GetCollisionLayer("OuterWall"));

   --projectile should just shoot out of the world
   g_PhysicsWorld:DisableCollisionLayers(GetCollisionLayer("Projectile"), GetCollisionLayer("OuterWall"));
   g_PhysicsWorld:DisableCollisionLayers(GetCollisionLayer("Projectile"), GetCollisionLayer("InnerWall"));
   g_PhysicsWorld:DisableCollisionLayers(GetCollisionLayer("Projectile"), GetCollisionLayer("Pickup"));

   g_PhysicsWorld:DisableCollisionLayers(GetCollisionLayer("ProjectileEasterEgg"), GetCollisionLayer("Default"));
   g_PhysicsWorld:DisableCollisionLayers(GetCollisionLayer("ProjectileEasterEgg"), GetCollisionLayer("PlayerProxy"));
   g_PhysicsWorld:DisableCollisionLayers(GetCollisionLayer("ProjectileEasterEgg"), GetCollisionLayer("EnemyProxy"));
   g_PhysicsWorld:DisableCollisionLayers(GetCollisionLayer("ProjectileEasterEgg"), GetCollisionLayer("DroneProxy"));
   g_PhysicsWorld:DisableCollisionLayers(GetCollisionLayer("ProjectileEasterEgg"), GetCollisionLayer("Pickup"));
   g_PhysicsWorld:DisableCollisionLayers(GetCollisionLayer("ProjectileEasterEgg"), GetCollisionLayer("OuterWall"));
   g_PhysicsWorld:DisableCollisionLayers(GetCollisionLayer("ProjectileEasterEgg"), GetCollisionLayer("Hazard"));
   
   
   --higher values preceed lower values
   --values greater than 10 and less than 1 are reserved for engine use
   g_TouchWorld:AddTouchStage( "Foreground", 3.0 );
   g_TouchWorld:AddTouchStage( "Midground",  2.0 );
   g_TouchWorld:AddTouchStage( "Background", 1.0 );
   
   g_AudioWorld:AddSoundGroup("SFX",   1.0);
   g_AudioWorld:AddSoundGroup("Music", 0.5);
   g_AudioWorld:AddSoundGroup("Voice", 1.0);
   
   local path = Engine.BuildPathToFile( "Data.resources" );
   self.m_Database:LoadResources( path );
end
end

function Startup:Destroy()
   self.m_Database:UnloadResources( );
   self.m_Database:Destroy( );
   self.m_Database = nil;

   g_AudioWorld:RemoveSoundGroup("SFX");
   g_AudioWorld:RemoveSoundGroup("Music");
   g_AudioWorld:RemoveSoundGroup("Voice");   

   print("Startup:Destroy");
end

function Startup:AddToScene()
   print("Startup:AddToScene");
   self.m_ParentResourceHandle = Engine.GetCurrentResourceHandle();
   print("Adding "..self.m_ParentResourceHandle:GetSystemId():ToString());
   
   Engine.LoadScene( "GlobalScene.lua" );
end

function Startup:RemoveFromScene()
   Engine.UnloadScene( "GlobalScene.lua" );

   print("Removing "..self.m_ParentResourceHandle:GetSystemId():ToString());
   self.m_ParentResourceHandle = nil;
end

function Startup:AddEventListeners()
end

function Startup:RemoveEventListeners()
end

function Startup:Tick(deltaSeconds)
   -- Called first in the engine tick process when enabled
end

function Startup:PostTick(deltaSeconds)
   -- Always called last in current engine tick
end

function Startup:Final(deltaSeconds)
   -- Always called last in current engine tick
end

function Startup:OnEvent(channelName, eventName)
end
]]






