local STARTUP = "dragon";

if (Game == nil) then
   Game = LuaComponent:New{};
   Game.__index = Game;   
end

function Game:New()
    local newObject = { };
    setmetatable(newObject, Game);
    newObject._native = Core.GetContextComponent();
    newObject._node = CreateNode(newObject._native:GetParent());
    return newObject;
end

function Game:_Create()
    print("GAME: HELLO WORLD!\n");
   
   self.GameHandle = LoadResources(STARTUP..".resources");
   if self.GameHandle ~= 0 then
      ExecuteLuaAsset(STARTUP..".lua");
      self.scene = Node_Create(Id_Create(), STARTUP);
      self.script = self.scene:AddComponent(Id_Create(), "ScriptComponent");
      self.script:Create(STARTUP..".lua");
      
      self.scene:AddToScene();
      self.scene:Bind();
   end
end

function Game:_Destroy()
   if self.scene ~= nil then
      self.scene:Destroy();
      UnloadResources(self.GameHandle);
   end
end

