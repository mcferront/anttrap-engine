if (FirstPersonController == nil) then
   FirstPersonController = LuaComponent:New{};
   FirstPersonController.__index = FirstPersonController;
   FirstPersonController.z_speed = 5;
   FirstPersonController.x_speed = 5;
   FirstPersonController.y_speed = 5;
   FirstPersonController.yaw_speed = .05;
   FirstPersonController.pitch_speed = .05;
   FirstPersonController.invert_y = true;
   FirstPersonController.child = nil;   
end

function FirstPersonController:New()
    local newObject = { };
    setmetatable(newObject, FirstPersonController);
    newObject._native = Core.GetContextComponent();
    newObject._parent = CreateNode(newObject._native:GetParent());
    newObject.yaw = 0;
    newObject.pitch = 0;
    newObject.roll = 0;
    return newObject;
end

function FirstPersonController:_Create()
    self._parent:SetTickable(true);
end

do
local pos_z_map;
local neg_z_map;
local pos_x_map;
local neg_x_map;
local pos_y_map;
local neg_y_map;
local yaw_map;
local pitch_map;
local transform = Transform_Create();
local position = Vector_Create();
local delta_position = Vector_Create();
local yaw_transform = Transform_Create();
local roll_transform = Transform_Create();
local pitch_transform = Transform_Create();
function FirstPersonController:Tick(delta_seconds)
   if (nil == pos_z_map) then
      pos_z_map = InputMap_Create("Forward");
      pos_s_map = InputMap_Create("Slow");
      neg_z_map = InputMap_Create("Backward");
      pos_x_map = InputMap_Create("Right");
      neg_x_map = InputMap_Create("Left");
      pos_y_map = InputMap_Create("Up");
      neg_y_map = InputMap_Create("Down");
      yaw_map = InputMap_Create("Yaw");
      pitch_map = InputMap_Create("Pitch");
   end

   self.x_speed = GetRegistryFloat("FPC/speed/x", self.x_speed) * delta_seconds;
   self.y_speed = GetRegistryFloat("FPC/speed/y", self.y_speed) * delta_seconds;
   self.z_speed = GetRegistryFloat("FPC/speed/z", self.z_speed) * delta_seconds;
   
   if Focus_Acquire(self) == false then
       return
   end
    
   local values_changed = false;
   local slow_pressed = pos_s_map:IsPressed();
    
   local speed_multiplier = GetRegistryFloat("FPC/speed/multiplier", 100);
   
   if slow_pressed then
      speed_multiplier = 1.0
   end
    
    self._parent:GetWorldTransform(transform);
    
    delta_position.x = 0;
    delta_position.y = 0;
    delta_position.z = 0;
    
    if (pos_z_map:IsPressed()) then
        delta_position.z = self.z_speed * speed_multiplier;
        values_changed = true
    elseif (neg_z_map:IsPressed()) then
        delta_position.z = - self.z_speed * speed_multiplier;
        values_changed = true
    end
    
    if (pos_y_map:IsPressed()) then
        delta_position.y = self.y_speed * speed_multiplier;
        values_changed = true
    elseif (neg_y_map:IsPressed()) then
        delta_position.y = - self.y_speed * speed_multiplier;
        values_changed = true
    end

    if (pos_x_map:IsPressed()) then
        delta_position.x = self.x_speed * speed_multiplier;
        values_changed = true
    elseif (neg_x_map:IsPressed()) then
        delta_position.x = - self.x_speed * speed_multiplier;
        values_changed = true
    end
    
   local yaw_value = yaw_map:GetValue();
   local pitch_value = pitch_map:GetValue();

   if yaw_value ~= 0 then
     self.yaw = self.yaw + yaw_value * self.yaw_speed * delta_seconds;
     values_changed = true
   end

   if pitch_value ~= 0 then
      if (false == self.invert_y) then
         self.pitch = self.pitch - pitch_value * self.pitch_speed * delta_seconds;
      else
         self.pitch = self.pitch + pitch_value * self.pitch_speed * delta_seconds;
      end
      values_changed = true;
   end
    
    position = transform:GetTranslation();

    Math.Math_RotateX(pitch_transform, self.pitch);
    Math.Math_RotateY(yaw_transform, self.yaw);
    Math.Math_RotateZ(roll_transform, self.roll);

    Math.Math_Multiply(transform, pitch_transform, roll_transform);
    Math.Math_Multiply(transform, transform, yaw_transform );
    Math.Math_Rotate(delta_position, delta_position, transform);

    position.x = position.x + delta_position.x;
    position.y = position.y + delta_position.y;
    position.z = position.z + delta_position.z;
    
   if values_changed == true and GetRegistryBool("FPC/log_values", false) then
      print("yaw = "..(self.yaw * 57.295779513)..", pitch = "..(self.pitch * 57.295779513).."\n")
      PrintVector("position = ", position)
      print("\r\n");
   end
   
    transform:SetTranslation(position);
    self._parent:SetWorldTransform(transform);
end
end
