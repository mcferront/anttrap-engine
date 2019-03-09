if (Bistro == nil) then
   Bistro = LuaComponent:New{};
   Bistro.__index = Bistro;   
end

function Bistro:New()
   local newObject = { };
   setmetatable(newObject, Bistro);
   newObject._native = Core.GetContextComponent();
   newObject._node = CreateNode(newObject._native:GetParent());
   newObject._resources = nil;
   newObject.activeIndex = 1;
   newObject.controlItems = {};
   newObject._camera_script = nil;
   newObject._camera_node = nil;
   newObject._lights = {};
   return newObject;
end

function Bistro:_Create()
   print("BISTRO: HELLO WORLD!\n");

   self.AssetsHandles = {};
   self.AssetsHandles[0] = LoadResources("BistroAssets1.resources");
   self.AssetsHandles[1] = LoadResources("BistroAssets2.resources");
   self.AssetsHandles[2] = LoadResources("BistroAssets3.resources");
   self.AssetsHandles[3] = LoadResources("BistroAssets4.resources");
   self.AssetsHandles[4] = LoadResources("BistroAssets5.resources");
   self.AssetsHandles[5] = LoadResources("BistroAssets6.resources");
   self.AssetsHandles[6] = LoadResources("BistroAssets7.resources");
   self.AssetsHandles[7] = LoadResources("BistroAssets8.resources");
   self.AssetsHandles[8] = LoadResources("BistroAssets9.resources");
   self.AssetsHandles[9] = LoadResources("BistroAssetsA.resources");
   self.AssetsHandles[10] = LoadResources("BistroAssetsB.resources");

   SetRegistryInt( "Debug/show_stats", 3 );
   
   self._node:RemoveFromScene();

   -- on screen commands
   self.label = Node_Create(Id_Create(), "Key Label");
   self.label.component = self.label:AddComponent(Id_Create(), "LabelComponent");
   self.label.component:Create(512, 256, 255, 255, 255, 255, "NULL", "DefaultFont.material", "DefaultFont.fontmap", "DefaultFont.bmp", "UI");
   self.label.component:SetAlign(TextArea.AlignLeft, TextArea.AlignTop);
   self.label.component:Print("[~] - Open Console");
   self.label.component:Print("[i] - Screenshot");
   self.label.component:Print(" ");
   self.label.component:Print("[W,A,S,D | Mouse] - Move/Rotate Camera");
   self.label.component:Print("[SPACE] - Cycle Views");
   self.label.component:Print("[TAB] - Toggle This");
   self.label.component:Print("[ESC] - Quit");
   self.label.in_scene = true;
   self.label:SetParent(self._node);
   self.label:AddToScene();
   self.label:SetWorldPosition(Math.Vector(- Screen.width / 2 + 512 / 2 + 16, Screen.height / 2 - 256 / 2 - 56, 0));
   self.label:Bind();

   -- AMBIENT LIGHT
   local nits = .25;
   
   color = Vector_Create();
   color.x = 1.0;
   color.y = 0.5;
   color.z = 0.5;
   color.w = 1;

   light = Node_Create(Id_Create(), "Ambient Light");
   light_properties  = light:AddComponent(Id_Create(), "AmbientLightComponent");
   light_properties:Create("Geometry", color, nits);
   light:AddToScene();
   light:Bind();

   local positions_x  = { 500, 800, 424, 611, 773, 722, 880,1037, 662 };
   local positions_y  = { 311, 311, 311, 311, 243, 261, 261, 261, 261,};
   local positions_z  = { 51, -100, 493, 850, 463, 269, 192, 105, 421 };
   local inner_angles = { 1.0, 1.0, 1.0, 1.0, .95, 1.5, 1.5, 1.5, 1.5 };
   local outer_angles = { 1.6, 1.6, 1.6, 1.6, 1.3, 2.7, 2.7, 2.7, 2.7 };
   local ranges       = { 500, 500, 500, 500, 200, 400, 400, 400, 400 };
   local reds         = { 1.0,    1.0,  1.0,  1.0, 0.35,  1.0, 1.0, 1.0, 1.0 };
   local greens       = { 0.75,  0.75, 0.75, 0.75, 0.25,  1.0, 1.0, 1.0, 1.0 };
   local blues        = { 0.75,  0.75, 0.75, 0.75, 0.25,  1.0, 1.0, 1.0, 1.0 };
   local nit_values   = { 32000,  32000, 32000, 32000, 5000, 5000,5000,5000,5000 };
   local yaws         = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
   local pitches      = { 90,   90,  90,  90,  90, 90,  90,   90,  90 };
   
   local num_spot_lights = #positions_x;
   
   for i = 1, num_spot_lights do
      local x = positions_x[((i - 1) % #positions_x) + 1];
      local y = positions_y[((i - 1) % #positions_y) + 1];
      local z = positions_z[((i - 1) % #positions_z) + 1];
      local inner_angle = inner_angles[((i - 1) % #inner_angles) + 1] * 2.0;
      local outer_angle = outer_angles[((i - 1) % #outer_angles) + 1] * 2.0;
      local red = reds[((i - 1) % #reds) + 1];
      local green = greens[((i - 1) % #greens) + 1];
      local blue = blues[((i - 1) % #blues) + 1];
      local range = ranges[((i - 1) % #ranges) + 1];
      local pitch = pitches[((i - 1) % #pitches) + 1];
      local yaw = yaws[((i - 1) % #yaws) + 1];
      local nits = nit_values[((i - 1) % #nit_values) + 1];
      
      color = Vector_Create();
      color.x = red;
      color.y = green;
      color.z = blue;
      color.w = 1.0;

      position = Vector_Create();
      position.x = x;
      position.y = y;
      position.z = z;
      position.w = 1;
            
      transform = Transform_Create();
      pitchTransform = Math_RotateX(pitch);
      yawTransform  = Math_RotateY(yaw);
      Math.Math_Multiply(transform, pitchTransform, yawTransform);
      transform:SetTranslation(position);

      light = Node_Create(Id_Create(), "Ceiling Light");
      light_properties  = light:AddComponent(Id_Create(), "SpotLightComponent");
      light_properties:Create("Geometry", color, nits, inner_angle, outer_angle, range);
      light:SetWorldTransform(transform);
      light:AddToScene();

      if i == 0 then
         script = light:AddComponent(Id_Create(), "ScriptComponent");
         script:Create("FirstPersonController.lua");    
         script:SetActive(false);
         table.insert(self.controlItems, script);
         script:GetScript().pitch = Math.Math_DegreesToRadians(pitch);
         script:GetScript().yaw = Math.Math_DegreesToRadians(yaw);
      end
      
      light:Bind();
      table.insert(self._lights, light_properties);
   end
   
   --SHADOW CAMERA
   light = Node_Get("ShadowCamera");
   if light ~= nil then
      color = Vector_Create();
      color.x = 0;
      color.y = 0;
      color.z = 0;
      color.w = 0;

      position = Vector_Create();
      position.x = 611;
      position.y = 396;
      position.z = 347;
      position.w = 1;

      yaw = 6;
      pitch = 83;
      
      transform = Transform_Create();
      pitchTransform = Math_RotateX(pitch);
      yawTransform  = Math_RotateY(yaw);
      Math.Math_Multiply(transform, pitchTransform, yawTransform);
      transform:SetTranslation(position);
      
      light:RemoveFromScene();
      light_properties  = light:AddComponent(Id_Create(), "DirectionalLightComponent");
      light_properties:Create("Geometry", color, 1.5);
      light:SetWorldTransform(transform);
      script = light:AddComponent(Id_Create(), "ScriptComponent");
      script:Create("FirstPersonController.lua");    
      script:SetActive(false);
      table.insert(self.controlItems, script);
       
      light:AddToScene();
      light:Bind();
      script:GetScript().pitch = Math.Math_DegreesToRadians(pitch);
      script:GetScript().yaw = Math.Math_DegreesToRadians(yaw);
   end
   
    --CAMERA
   cameraNode = Node_Get("MainCamera");

   position = Vector_Create();
   position.x = 686;
   position.y = 193;
   position.z = -232;
   position.w = 1;
	yaw = 26;
   pitch = 7;
   
   position.x = 0;
   position.y = 0;
   position.z = -10;
   position.w = 1;
	yaw = 0;
   pitch = 0;

   transform = Math_RotateY(yaw);
   transform:SetIdentity();
    
   position = Math_TransformPosition(position, transform);
   transform:SetTranslation(position);

   cameraNode:SetWorldTransform(transform);
   cameraNode:SetParent(self._node);   
   cameraNode:AddToScene();
   self.cameraNode = cameraNode;
    
   script = cameraNode:AddComponent(Id_Create(), "ScriptComponent");
   script:Create("FirstPersonController.lua");    
   script:SetActive(false);
   table.insert(self.controlItems, script);
   cameraNode:Bind(); 
	script:GetScript().yaw = Math.Math_DegreesToRadians(yaw);
	script:GetScript().pitch = Math.Math_DegreesToRadians(pitch);

   self._camera_script = script:GetScript();
   self._camera_node = cameraNode;
    
   Core.AddMeshes(self.AssetsHandles[0]); 
   Core.AddMeshes(self.AssetsHandles[1]); 
   Core.AddMeshes(self.AssetsHandles[2]); 
   Core.AddMeshes(self.AssetsHandles[3]); 
   Core.AddMeshes(self.AssetsHandles[4]); 
   Core.AddMeshes(self.AssetsHandles[5]); 
   Core.AddMeshes(self.AssetsHandles[6]); 
   Core.AddMeshes(self.AssetsHandles[7]); 
   Core.AddMeshes(self.AssetsHandles[8]); 
   Core.AddMeshes(self.AssetsHandles[9]); 
   Core.AddMeshes(self.AssetsHandles[10]); 
   Core.PlaceMeshes(self:GetParent()._native, "Bistro_Mappings.data");
   SetRegistryBool("hemisphere.enable", false);

   --first node to be controlled
   self.activeIndex = #self.controlItems;
   self.controlItems[self.activeIndex]:SetActive(true);

   --listen for input
   InputSystem_AddEventListener("KeyDown", "OnKeyDown");

   self._node:AddToScene();
   
   self:JumpToShot(6);
end

function Bistro:_Destroy()
   UnloadResources(self.AssetsHandles[0]);
   UnloadResources(self.AssetsHandles[1]);
end

do
local scene_index = 0;
local scenes = 
{
   {  -- cookies
      yaw = 17,
      pitch = 10,
      x = 667,
      y = 196,
      z = 238,
      dof_far_kernel = 10,
      dof_far_plane = 150,
   },
   {  -- taps
      yaw = 44,
      pitch = 7,
      x = 703,
      y = 212,
      z = 46,
      dof_far_kernel = 5,
      dof_far_plane = 300,
   },
   {  -- corner looking out
      yaw = 510,
      pitch = 12,
      x = 602,
      y = 234,
      z = 413,
      dof_far_kernel = 21,
      dof_far_plane = 140,
   },
   {  -- sitting at table
      yaw = 387,
      pitch = 4,
      x = 164,
      y = 163,
      z = 552,
      dof_far_kernel = 2,
      dof_far_plane = 200,
   },
   {  -- plant
      yaw = -33,
      pitch = 12,
      x = 1102,
      y = 257,
      z = 50,
      dof_far_kernel = 15,
      dof_far_plane = 600,
   },
   {  -- table facing out
      yaw = 133,
      pitch = 6,
      x = 100,
      y = 147,
      z = 715,
      dof_far_kernel = 20,
      dof_far_plane = 150,
   },
}

local res_index = 0;
local resolutions = 
{
   -- no upres
   {
      upres = 0,
      width = 0,
   },
   -- checkerboard corner
   {
      upres = 1,
      width = 0,
   },
   -- ddr taa
   {
      upres = 2,
      width = 0,
   },
   -- no upres @ 4k
   {
      upres = 0,
      width = 3840,
   },
   -- checkerboard @ 4k
   {
      upres = 1,
      width = 3840,
   },
   -- ddr taa @ 4k
   {
      upres = 2,
      width = 3840,
   },
}

local floor_index = 0;
local floors = 
{
   "floor_wet",
   "floor_tile",
}

function Bistro:JumpToShot(index)

   if index < 1 then 
      index = #scenes; 
   else
      index = ((index - 1) % #scenes) + 1;
   end
   
   scene_index = index;

   local position = Vector_Create();
   local yaw, pitch;
   
   position.x = scenes[ scene_index ].x;
   position.y = scenes[ scene_index ].y;
   position.z = scenes[ scene_index ].z;
   yaw = scenes[ scene_index ].yaw;
   pitch = scenes[ scene_index ].pitch;

   self._camera_script.yaw = Math.Math_DegreesToRadians(yaw);
   self._camera_script.pitch = Math.Math_DegreesToRadians(pitch);
   
   SetRegistryFloat("dof.focal_plane_end", scenes[ scene_index ].dof_far_plane);
   SetRegistryFloat("dof.far_kernel", scenes[ scene_index ].dof_far_kernel);
   
   
   local transform = Transform_Create();
   transform:SetIdentity();
   transform:SetTranslation(position);
   self._camera_node:SetWorldTransform(transform);
end

function Bistro:OnKeyDown(sender, event, key, modifiers)
    if Focus_Acquire(self) then
       keyAscii = get_low_8_bits(key, 8);
       keyScan = get_high_8_bits(key, 8);

       local shift_down = (string.sub(modifiers, 1, 1) == "1" or string.sub(modifiers, 2, 2) == '1');

       if (keyAscii == 27) then --'esc'
            Core.HandleConsoleCommand("quit");
       elseif (keyAscii == 9) then --'tab'
         if self.label.in_scene then 
            self.label:RemoveFromScene();
            self.label.stats = GetRegistryInt("Debug/show_stats", 0);
            SetRegistryInt("Debug/show_stats", 0);
         else
            self.label:AddToScene();
            SetRegistryInt("Debug/show_stats", self.label.stats);
         end            
         self.label.in_scene = self.label.in_scene ~= true;         
       elseif (keyAscii == 32) then --'space'
         if shift_down then
            self:JumpToShot(scene_index - 1);
         else
            self:JumpToShot(scene_index + 1);
         end
       elseif (keyAscii == 73) then --'i'
         SetRegistryBool( "App/screenshot", true );
      end

      Focus_Pop(self);
    end    
end
end

