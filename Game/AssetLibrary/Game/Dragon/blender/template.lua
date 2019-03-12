if ($NAME == nil) then
   $NAME = LuaComponent:New{};
   $NAME.__index = $NAME;   
end

function $NAME:New()
   local newObject = { };
   setmetatable(newObject, $NAME);
   newObject._native = Core.GetContextComponent();
   newObject._node = CreateNode(newObject._native:GetParent());
   return newObject;
end

function $NAME:_Create()
   print("$NAME: HELLO WORLD!\n");

   self._node:RemoveFromScene();

$CAMERA

$LIGHTS

$MESHES

   self._node:AddToScene();
end

function $NAME:_Destroy()
end


