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
   node:SetWorldTransform(Transform_Set(Vector_Set(35.664039611816406,50.99458312988281,-290.7057189941406), Quaternion_Set(0.0,0.0,0.0,1.0)));
   camera = node:GetComponent("CameraComponent");
   camera:SetFov(0.8575560450553894);
   camera:SetNearClip(0.10000000149011612);
   camera:SetFarClip(1000.0);
   script = node:AddComponent(Id_Create(), "ScriptComponent");
   script:Create("FirstPersonController.lua");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();
   script:GetScript().pitch = -0.00;
   script:GetScript().yaw = -0.00;
   script:GetScript().roll = -0.00;


   node = Node_Create(Id_Create(), "Sun");
   node:SetWorldTransform(Transform_Set(Vector_Set(33.343448638916016,49.658790588378906,-479.474365234375), Quaternion_Set(0.0,0.0,0.0,1.0)));
   light = node:AddComponent(Id_Create(), "DirectionalLightComponent");
   light:Create("Geometry", Vector_Set(1.0,1.0,1.0),1.0);
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();



   node = Node_Create(Id_Create(), "brdf_sphere.024");
   node:SetWorldTransform(Transform_Set(Vector_Set(-18.293560028076172,-9.32406997680664,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.011.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.023");
   node:SetWorldTransform(Transform_Set(Vector_Set(7.529406547546387,-9.32406997680664,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.012.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.012");
   node:SetWorldTransform(Transform_Set(Vector_Set(32.887184143066406,-9.32406997680664,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.022.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.011");
   node:SetWorldTransform(Transform_Set(Vector_Set(58.16260528564453,-9.32406997680664,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.023.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.001");
   node:SetWorldTransform(Transform_Set(Vector_Set(83.97831726074219,-9.32406997680664,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.029.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.029");
   node:SetWorldTransform(Transform_Set(Vector_Set(83.97831726074219,110.16429138183594,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.028.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.028");
   node:SetWorldTransform(Transform_Set(Vector_Set(83.97831726074219,85.81858825683594,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.027.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.027");
   node:SetWorldTransform(Transform_Set(Vector_Set(83.97831726074219,61.10085678100586,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.026.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.026");
   node:SetWorldTransform(Transform_Set(Vector_Set(83.97831726074219,37.17118835449219,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.025.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.025");
   node:SetWorldTransform(Transform_Set(Vector_Set(83.97831726074219,13.96291732788086,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.024.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.022");
   node:SetWorldTransform(Transform_Set(Vector_Set(58.16260528564453,13.96291732788086,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.021.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.021");
   node:SetWorldTransform(Transform_Set(Vector_Set(58.16260528564453,37.17118835449219,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.020.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.020");
   node:SetWorldTransform(Transform_Set(Vector_Set(58.16260528564453,61.10085678100586,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.019.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.019");
   node:SetWorldTransform(Transform_Set(Vector_Set(58.16260528564453,85.81858825683594,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.018.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.018");
   node:SetWorldTransform(Transform_Set(Vector_Set(58.16260528564453,110.16429138183594,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.017.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.017");
   node:SetWorldTransform(Transform_Set(Vector_Set(32.887184143066406,110.16429138183594,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.016.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.016");
   node:SetWorldTransform(Transform_Set(Vector_Set(32.887184143066406,85.81858825683594,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.015.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.015");
   node:SetWorldTransform(Transform_Set(Vector_Set(32.887184143066406,61.10085678100586,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.001.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.014");
   node:SetWorldTransform(Transform_Set(Vector_Set(32.887184143066406,37.17118835449219,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.014.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.013");
   node:SetWorldTransform(Transform_Set(Vector_Set(32.887184143066406,13.96291732788086,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.013.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.010");
   node:SetWorldTransform(Transform_Set(Vector_Set(7.529406547546387,13.96291732788086,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.010.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.009");
   node:SetWorldTransform(Transform_Set(Vector_Set(7.529406547546387,37.17118835449219,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.009.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.008");
   node:SetWorldTransform(Transform_Set(Vector_Set(7.529406547546387,61.10085678100586,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.008.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.007");
   node:SetWorldTransform(Transform_Set(Vector_Set(7.529406547546387,85.81858825683594,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.007.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.006");
   node:SetWorldTransform(Transform_Set(Vector_Set(7.529406547546387,110.16429138183594,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.006.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.005");
   node:SetWorldTransform(Transform_Set(Vector_Set(-18.293560028076172,110.16429138183594,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.005.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.004");
   node:SetWorldTransform(Transform_Set(Vector_Set(-18.293560028076172,85.81858825683594,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.004.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.003");
   node:SetWorldTransform(Transform_Set(Vector_Set(-18.293560028076172,61.10085678100586,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.003.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.002");
   node:SetWorldTransform(Transform_Set(Vector_Set(-18.293560028076172,37.17118835449219,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.002.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere");
   node:SetWorldTransform(Transform_Set(Vector_Set(-18.293560028076172,13.96291732788086,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();



   self._node:AddToScene();
end

function dragon:_Destroy()
end


