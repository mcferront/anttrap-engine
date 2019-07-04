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

   self.GameHandle = LoadResources("environment.resources");

   self._node:RemoveFromScene();

   node = Node_Get("MainCamera");
   node:SetWorldTransform(Transform_Set(Vector_Set(102.5252685546875,102.11091613769531,-598.2654418945312), Quaternion_Set(0.0,0.0,0.0,1.0)));
   camera = node:GetComponent("CameraComponent");
   camera:SetFov(0.8575560450553894);
   camera:SetNearClip(0.10000000149011612);
   camera:SetFarClip(2000.0);
   script = node:AddComponent(Id_Create(), "ScriptComponent");
   script:Create("FirstPersonController.lua");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();
   script:GetScript().pitch = -0.00;
   script:GetScript().yaw = -0.00;
   script:GetScript().roll = -0.00;


   node = Node_Create(Id_Create(), "Sun");
   node:SetWorldTransform(Transform_Set(Vector_Set(33.343448638916016,151.0697021484375,-479.474365234375), Quaternion_Set(0.3826834559440613,0.0,0.0,0.9238795042037964)));
   light = node:AddComponent(Id_Create(), "DirectionalLightComponent");
   light:Create("Geometry", Vector_Set(1.0,1.0,1.0),1.0);
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "Sky");
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","sky.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   local t = Transform_Create();
   node:GetWorldTransform(t);
   t:SetTranslation(Vector_Set(75,75,50));
   t:SetScale(Vector_Set(100,100,100));
   node:SetWorldTransform(t);

   node = Node_Create(Id_Create(), "brdf_sphere.131");
   node:SetWorldTransform(Transform_Set(Vector_Set(83.97831726074219,-33.81973648071289,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.029.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.130");
   node:SetWorldTransform(Transform_Set(Vector_Set(58.16260528564453,-33.81973648071289,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.023.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.129");
   node:SetWorldTransform(Transform_Set(Vector_Set(32.887184143066406,-33.81973648071289,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.022.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.128");
   node:SetWorldTransform(Transform_Set(Vector_Set(7.529406547546387,-33.81973648071289,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.012.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.127");
   node:SetWorldTransform(Transform_Set(Vector_Set(-18.293560028076172,-33.81973648071289,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.011.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.126");
   node:SetWorldTransform(Transform_Set(Vector_Set(108.99468994140625,-33.81973648071289,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.055.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.125");
   node:SetWorldTransform(Transform_Set(Vector_Set(134.81765747070312,-33.81973648071289,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.056.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.124");
   node:SetWorldTransform(Transform_Set(Vector_Set(160.17544555664062,-33.81973648071289,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.057.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.123");
   node:SetWorldTransform(Transform_Set(Vector_Set(185.45086669921875,-33.81973648071289,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.058.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.122");
   node:SetWorldTransform(Transform_Set(Vector_Set(211.26657104492188,-33.81973648071289,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.059.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.121");
   node:SetWorldTransform(Transform_Set(Vector_Set(-44.80034637451172,-33.81973648071289,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.126.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.120");
   node:SetWorldTransform(Transform_Set(Vector_Set(-18.293560028076172,134.56396484375,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.105.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.119");
   node:SetWorldTransform(Transform_Set(Vector_Set(-18.293560028076172,157.77223205566406,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.102.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.118");
   node:SetWorldTransform(Transform_Set(Vector_Set(-18.293560028076172,181.701904296875,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.083.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.117");
   node:SetWorldTransform(Transform_Set(Vector_Set(-18.293560028076172,206.4196319580078,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.080.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.116");
   node:SetWorldTransform(Transform_Set(Vector_Set(-18.293560028076172,230.7653350830078,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.060.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.115");
   node:SetWorldTransform(Transform_Set(Vector_Set(7.529406547546387,230.7653350830078,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.062.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.114");
   node:SetWorldTransform(Transform_Set(Vector_Set(7.529406547546387,206.4196319580078,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.079.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.113");
   node:SetWorldTransform(Transform_Set(Vector_Set(7.529406547546387,181.701904296875,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.084.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.112");
   node:SetWorldTransform(Transform_Set(Vector_Set(7.529406547546387,157.77223205566406,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.101.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.111");
   node:SetWorldTransform(Transform_Set(Vector_Set(7.529406547546387,134.56396484375,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.106.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.110");
   node:SetWorldTransform(Transform_Set(Vector_Set(32.887184143066406,134.56396484375,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.107.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.109");
   node:SetWorldTransform(Transform_Set(Vector_Set(32.887184143066406,157.77223205566406,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.100.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.108");
   node:SetWorldTransform(Transform_Set(Vector_Set(32.887184143066406,181.701904296875,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.085.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.107");
   node:SetWorldTransform(Transform_Set(Vector_Set(32.887184143066406,206.4196319580078,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.078.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.106");
   node:SetWorldTransform(Transform_Set(Vector_Set(32.887184143066406,230.7653350830078,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.063.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.105");
   node:SetWorldTransform(Transform_Set(Vector_Set(58.16260528564453,230.7653350830078,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.064.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.104");
   node:SetWorldTransform(Transform_Set(Vector_Set(58.16260528564453,206.4196319580078,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.077.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.103");
   node:SetWorldTransform(Transform_Set(Vector_Set(58.16260528564453,181.701904296875,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.086.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.102");
   node:SetWorldTransform(Transform_Set(Vector_Set(58.16260528564453,157.77223205566406,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.099.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.101");
   node:SetWorldTransform(Transform_Set(Vector_Set(58.16260528564453,134.56396484375,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.108.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.100");
   node:SetWorldTransform(Transform_Set(Vector_Set(83.97831726074219,134.56396484375,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.109.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.099");
   node:SetWorldTransform(Transform_Set(Vector_Set(83.97831726074219,157.77223205566406,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.098.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.098");
   node:SetWorldTransform(Transform_Set(Vector_Set(83.97831726074219,181.701904296875,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.087.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.097");
   node:SetWorldTransform(Transform_Set(Vector_Set(83.97831726074219,206.4196319580078,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.076.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.096");
   node:SetWorldTransform(Transform_Set(Vector_Set(83.97831726074219,230.7653350830078,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.065.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.095");
   node:SetWorldTransform(Transform_Set(Vector_Set(211.26657104492188,230.7653350830078,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.070.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.094");
   node:SetWorldTransform(Transform_Set(Vector_Set(211.26657104492188,206.4196319580078,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.071.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.093");
   node:SetWorldTransform(Transform_Set(Vector_Set(211.26657104492188,181.701904296875,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.092.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.092");
   node:SetWorldTransform(Transform_Set(Vector_Set(211.26657104492188,157.77223205566406,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.093.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.091");
   node:SetWorldTransform(Transform_Set(Vector_Set(211.26657104492188,134.56396484375,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.114.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.090");
   node:SetWorldTransform(Transform_Set(Vector_Set(185.45086669921875,134.56396484375,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.113.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.089");
   node:SetWorldTransform(Transform_Set(Vector_Set(185.45086669921875,157.77223205566406,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.094.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.088");
   node:SetWorldTransform(Transform_Set(Vector_Set(185.45086669921875,181.701904296875,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.091.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.087");
   node:SetWorldTransform(Transform_Set(Vector_Set(185.45086669921875,206.4196319580078,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.072.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.086");
   node:SetWorldTransform(Transform_Set(Vector_Set(185.45086669921875,230.7653350830078,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.069.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.085");
   node:SetWorldTransform(Transform_Set(Vector_Set(160.17544555664062,230.7653350830078,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.068.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.084");
   node:SetWorldTransform(Transform_Set(Vector_Set(160.17544555664062,206.4196319580078,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.073.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.083");
   node:SetWorldTransform(Transform_Set(Vector_Set(160.17544555664062,181.701904296875,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.090.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.082");
   node:SetWorldTransform(Transform_Set(Vector_Set(160.17544555664062,157.77223205566406,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.095.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.081");
   node:SetWorldTransform(Transform_Set(Vector_Set(160.17544555664062,134.56396484375,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.112.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.080");
   node:SetWorldTransform(Transform_Set(Vector_Set(134.81765747070312,134.56396484375,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.111.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.079");
   node:SetWorldTransform(Transform_Set(Vector_Set(134.81765747070312,157.77223205566406,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.096.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.078");
   node:SetWorldTransform(Transform_Set(Vector_Set(134.81765747070312,181.701904296875,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.089.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.077");
   node:SetWorldTransform(Transform_Set(Vector_Set(134.81765747070312,206.4196319580078,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.074.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.076");
   node:SetWorldTransform(Transform_Set(Vector_Set(134.81765747070312,230.7653350830078,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.067.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.075");
   node:SetWorldTransform(Transform_Set(Vector_Set(108.99468994140625,230.7653350830078,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.066.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.074");
   node:SetWorldTransform(Transform_Set(Vector_Set(108.99468994140625,206.4196319580078,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.075.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.073");
   node:SetWorldTransform(Transform_Set(Vector_Set(108.99468994140625,181.701904296875,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.088.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.072");
   node:SetWorldTransform(Transform_Set(Vector_Set(108.99468994140625,157.77223205566406,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.097.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.071");
   node:SetWorldTransform(Transform_Set(Vector_Set(108.99468994140625,134.56396484375,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.110.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.070");
   node:SetWorldTransform(Transform_Set(Vector_Set(-44.80034637451172,230.7653350830078,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.061.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.069");
   node:SetWorldTransform(Transform_Set(Vector_Set(-44.80034637451172,206.4196319580078,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.081.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.068");
   node:SetWorldTransform(Transform_Set(Vector_Set(-44.80034637451172,181.701904296875,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.082.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.067");
   node:SetWorldTransform(Transform_Set(Vector_Set(-44.80034637451172,157.77223205566406,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.103.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.066");
   node:SetWorldTransform(Transform_Set(Vector_Set(-44.80034637451172,134.56396484375,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.104.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.065");
   node:SetWorldTransform(Transform_Set(Vector_Set(-44.80034637451172,13.96291732788086,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.064");
   node:SetWorldTransform(Transform_Set(Vector_Set(-44.80034637451172,37.17118835449219,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.002.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.063");
   node:SetWorldTransform(Transform_Set(Vector_Set(-44.80034637451172,61.10085678100586,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.003.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.062");
   node:SetWorldTransform(Transform_Set(Vector_Set(-44.80034637451172,85.81858825683594,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.004.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.061");
   node:SetWorldTransform(Transform_Set(Vector_Set(-44.80034637451172,110.16429138183594,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.005.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.060");
   node:SetWorldTransform(Transform_Set(Vector_Set(-44.80034637451172,-9.32406997680664,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.115.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.059");
   node:SetWorldTransform(Transform_Set(Vector_Set(108.99468994140625,13.96291732788086,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.050.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.058");
   node:SetWorldTransform(Transform_Set(Vector_Set(108.99468994140625,37.17118835449219,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.045.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.057");
   node:SetWorldTransform(Transform_Set(Vector_Set(108.99468994140625,61.10085678100586,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.040.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.056");
   node:SetWorldTransform(Transform_Set(Vector_Set(108.99468994140625,85.81858825683594,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.035.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.055");
   node:SetWorldTransform(Transform_Set(Vector_Set(108.99468994140625,110.16429138183594,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.030.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.054");
   node:SetWorldTransform(Transform_Set(Vector_Set(134.81765747070312,110.16429138183594,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.031.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.053");
   node:SetWorldTransform(Transform_Set(Vector_Set(134.81765747070312,85.81858825683594,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.036.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.052");
   node:SetWorldTransform(Transform_Set(Vector_Set(134.81765747070312,61.10085678100586,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.041.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.051");
   node:SetWorldTransform(Transform_Set(Vector_Set(134.81765747070312,37.17118835449219,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.046.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.050");
   node:SetWorldTransform(Transform_Set(Vector_Set(134.81765747070312,13.96291732788086,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.051.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.049");
   node:SetWorldTransform(Transform_Set(Vector_Set(160.17544555664062,13.96291732788086,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.052.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.048");
   node:SetWorldTransform(Transform_Set(Vector_Set(160.17544555664062,37.17118835449219,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.047.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.047");
   node:SetWorldTransform(Transform_Set(Vector_Set(160.17544555664062,61.10085678100586,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.042.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.046");
   node:SetWorldTransform(Transform_Set(Vector_Set(160.17544555664062,85.81858825683594,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.037.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.045");
   node:SetWorldTransform(Transform_Set(Vector_Set(160.17544555664062,110.16429138183594,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.032.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.044");
   node:SetWorldTransform(Transform_Set(Vector_Set(185.45086669921875,110.16429138183594,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.033.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.043");
   node:SetWorldTransform(Transform_Set(Vector_Set(185.45086669921875,85.81858825683594,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.038.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.042");
   node:SetWorldTransform(Transform_Set(Vector_Set(185.45086669921875,61.10085678100586,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.043.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.041");
   node:SetWorldTransform(Transform_Set(Vector_Set(185.45086669921875,37.17118835449219,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.048.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.040");
   node:SetWorldTransform(Transform_Set(Vector_Set(185.45086669921875,13.96291732788086,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.053.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.039");
   node:SetWorldTransform(Transform_Set(Vector_Set(211.26657104492188,13.96291732788086,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.054.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.038");
   node:SetWorldTransform(Transform_Set(Vector_Set(211.26657104492188,37.17118835449219,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.049.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.037");
   node:SetWorldTransform(Transform_Set(Vector_Set(211.26657104492188,61.10085678100586,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.044.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.036");
   node:SetWorldTransform(Transform_Set(Vector_Set(211.26657104492188,85.81858825683594,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.039.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.035");
   node:SetWorldTransform(Transform_Set(Vector_Set(211.26657104492188,110.16429138183594,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.034.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.034");
   node:SetWorldTransform(Transform_Set(Vector_Set(211.26657104492188,-9.32406997680664,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.125.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.033");
   node:SetWorldTransform(Transform_Set(Vector_Set(185.45086669921875,-9.32406997680664,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.124.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.032");
   node:SetWorldTransform(Transform_Set(Vector_Set(160.17544555664062,-9.32406997680664,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.123.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.031");
   node:SetWorldTransform(Transform_Set(Vector_Set(134.81765747070312,-9.32406997680664,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.122.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.030");
   node:SetWorldTransform(Transform_Set(Vector_Set(108.99468994140625,-9.32406997680664,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.121.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.024");
   node:SetWorldTransform(Transform_Set(Vector_Set(-18.293560028076172,-9.32406997680664,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.116.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.023");
   node:SetWorldTransform(Transform_Set(Vector_Set(7.529406547546387,-9.32406997680664,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.117.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.012");
   node:SetWorldTransform(Transform_Set(Vector_Set(32.887184143066406,-9.32406997680664,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.118.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.011");
   node:SetWorldTransform(Transform_Set(Vector_Set(58.16260528564453,-9.32406997680664,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.119.material", "Geometry");
   node:SetParent(self._node);
   node:AddToScene();
   node:Bind();

   node = Node_Create(Id_Create(), "brdf_sphere.001");
   node:SetWorldTransform(Transform_Set(Vector_Set(83.97831726074219,-9.32406997680664,0.0), Quaternion_Set(0.0,0.0,0.0,1.0)));
   mesh = node:AddComponent(Id_Create(), "MeshRendererComponent");
   mesh:Create("brdf_sphere.mesh","brdf_sphere.120.material", "Geometry");
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


