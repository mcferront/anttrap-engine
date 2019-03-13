if (dragon == nil) then
   dragon = LuaComponent:New{};
   dragon.__index = dragon;   
end

function dragon:New()
   local newObject = { };
   setmetatable(newObject, dragon);
   newObject._native = Core.GetContextComponent();
   newObject._node = CreateNode(newObject._native:GetParent());
   return newObject;
end

function dragon:_Create()
   print("dragon: HELLO WORLD!\n");

   self._node:RemoveFromScene();

   node = Node_Get("MainCamera");
   node:SetWorldTransform(Transform_Set(Vector_Set(1.1539392471313477,7.7523579597473145,-54.99597930908203), Quaternion_Set(0.0,0.0,0.0,1.0)));
   camera = node:GetComponent("CameraComponent");
   camera:SetFov(0.8575560450553894);
   camera:SetNearClip(0.10000000149011612);
   camera:SetFarClip(100.0);
   script = node:AddComponent(Id_Create(), "ScriptComponent");
   script:Create("FirstPersonController.lua");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();
   script:GetScript().pitch = -0.00;
   script:GetScript().yaw = -0.00;
   script:GetScript().roll = -0.00;


   node = Node_Create(Id_Create(), "Sun");
   node:SetWorldTransform(Transform_Set(Vector_Set(35.35108947753906,34.549835205078125,14.000901222229004), Quaternion_Set(0.0,0.0,0.0,1.0)));
   light = node:AddComponent(Id_Create(), "DirectionalLightComponent");
   light:Create("Geometry", Vector_Set(1.0,1.0,1.0),1.0);
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();



   node = Node_Create(Id_Create(), "dragon_vrip");
   node:SetWorldTransform(Transform_Set(Vector_Set(0.0,0.0,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("dragon_vrip.mesh","dragon_mat.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();



   self._node:AddToScene();
end

function dragon:_Destroy()
end


