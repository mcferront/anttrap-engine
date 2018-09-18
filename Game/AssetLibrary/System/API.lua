Time = {};
Time.deltaTime = 0;

Screen = {};
Screen.width = Core.GetRegistryInt( "Window/width", 1920 );
Screen.height = Core.GetRegistryInt( "Window/height", 1080 );

function get_low_8_bits(key)
    --new value with high bits shifted down
    local high_value = math.floor( key / 256 );
    --move high bits back up, lower bits will be 00
    high_value = high_value * 256;
    --subtract original value from high bits + 00 
    --which leads only original low bits 
    return key - high_value
end

function get_high_8_bits(key)
    --shift high bits down
    return math.floor( key / 256 );
end

--Simple math
function LerpFloat(a, b, t)
   return (b - a) * t + a; 
end

function LoadResources(name)
    return Core.LoadResources(name);
end

function UnloadResources(handle)
    return Core.UnloadResources(handle);
end

function OnError(e)
    print("Lua Error: "..e);
end

function PrintQuaternion(label, v)
    print(label..v.quaternion.x..","..v.quaternion.y..","..v.quaternion.z..","..v.quaternion.w);
end

function PrintVector(label, v)
    print(label..v.x..","..v.y..","..v.z..","..v.w);
end

function PrintTransform(label, t)
    PrintQuaternion(label.."rotation: ", t:GetOrientation());
    PrintVector(label.."position: ", t:GetTranslation());
    PrintVector(label.."scale:    ", t:GetScale());
end

-- registry
function GetRegistryInt(path, default)
   return Core.GetRegistryInt(path, default);
end 

function GetRegistryFloat(path, default)
   return Core.GetRegistryFloat(path, default);
end 

function GetRegistryBool(path, default)
   return Core.GetRegistryBool(path, default);
end 

function SetRegistryInt(path, value)
   return Core.SetRegistryInt(path, value);
end 

function SetRegistryFloat(path, value)
   return Core.SetRegistryFloat(path, value);
end 

function SetRegistryBool(path, value)
   return Core.SetRegistryBool(path, value);
end 

-- lua
function ExecuteLuaAsset(assetAlias)
    Core.ExecuteLuaAsset(assetAlias);
end

--Coroutines
do
s_Coroutines = {};
s_CoroutineObj = nil;

function StartCoroutine(o, f)
   local co = coroutine.create(CoroutineObj_Internal);
   local r = {};

   local index   = string.find(f, ":");
   local luaTable= string.sub(f, 0, index - 1);
   local luaFunc = string.sub(f, index + 1);

   r.co = co;
   r.o = o;
   r.t = luaTable;
   r.f = luaFunc;
   r.delay = 0;
   
   table.insert(s_Coroutines, r);
end

function StopCoroutine(o, f)
    if (true == s_Executing) then
		-- Because the coroutine table is temporarily copied
		print("Cannot call StopCoroutine from a coroutine");
		return;
	end

    local index   = string.find(f, ":");
    local luaTable= string.sub(f, 0, index - 1);
    local luaFunc = string.sub(f, index + 1);
   
    for i = 1, #s_Coroutines do
        local v = s_Coroutines[i];
        if (v ~= nil and v.o == o and v.f == luaFunc and v.t == luaTable) then
            s_Coroutines[i] = nil;
        end
    end   
end

function StopAllCoroutines(o)
    if (true == s_Executing) then
		-- Because the coroutine table is temporarily copied
		print("Cannot call StopAllCoroutines from a coroutine");
		return;
	end
	
	for i = 1, #s_Coroutines do
        local v = s_Coroutines[i];
        if (v ~= nil and v.o == o) then
            s_Coroutines[i] = nil;
        end
    end   
end

function ExecuteCoroutines(deltaSeconds)
    s_Executing = true;
	Time.deltaTime = deltaSeconds;

	--copy table because we don't want coroutines started by existing coroutines
	--to execute until the next tick.  otherwise we could get in an infinite
	--coroutine loop
    local activeRoutines = s_Coroutines;
    s_Coroutines = {};

    for i = 1, #activeRoutines do
        local v = activeRoutines[i];
        if (v ~= nil) then
            s_CoroutineObj = v;
            s_CoroutineObj.delay = s_CoroutineObj.delay - deltaSeconds;
            if (s_CoroutineObj.delay <= 0) then
                local r, m, d = coroutine.resume(s_CoroutineObj.co);
                if (r == false or m ~= 1) then
                    activeRoutines[i] = nil;
                    if (r == false) then print("Coroutine "..v.t..":"..v.f.." Error: "..m); end
                else
                    s_CoroutineObj.delay = d;
                end
            end
        end
    end   
    
    --rebuild table
    for i = 1, #activeRoutines do
        local v = activeRoutines[i];
        if (v ~= nil) then
            table.insert(s_Coroutines, v);
        end
    end
	s_Executing = false;	
end

function WaitForSeconds(s)
   --always yield 1 (which is return value for our resume call)
   --this way if our resume doesn't receive a 1 it knows to terminate
   --the coroutine (error or a natural end)
   coroutine.yield(1, s);
end

function CoroutineObj_Internal()
   return _G[s_CoroutineObj.t][s_CoroutineObj.f](s_CoroutineObj.o);
end
end

function Vector_Create()
    return Math.Vector();
end

function Transform_Create()
    return Math.Transform();
end

function Math_RotateY(degrees)
    local result = Math.Transform();
    Math.Math_RotateY(result, Math.Math_DegreesToRadians(degrees));
    return result;
end

function Math_RotateX(degrees)
    local result = Math.Transform();
    Math.Math_RotateX(result, Math.Math_DegreesToRadians(degrees));
    return result;
end

function Math_TransformPosition(position, transform)
    local result = Math.Vector();
    Math.Math_TransformPosition(result, position, transform);
    return result;
end 

if (InputMap == nil) then
    InputMap = {};
    InputMap.__index = InputMap;
end

function CreateInputMap(keymap)
    local newObject = {};
    setmetatable(newObject, InputMap);
    newObject._native = Core.CreateInputMap(keymap);
    return newObject;
end

function InputMap_Create(keymap)
    return CreateInputMap(keymap);
end

function InputMap:IsNewPress()
    return self._native:IsNewPress();
end

function InputMap:IsNewRelease()
    return self._native:IsNewRelease();
end

function InputMap:IsPressed()
    return self._native:IsPressed();
end

function InputMap:IsReleased()
    return self._native:IsReleased();
end

function InputMap:GetValue()
    return self._native:GetValue();
end

function InputMap:Press()
    return self._native:Press();
end

function InputMap:Release()
    return self._native:Release();
end

--id
if (Id == nil) then
   Id = {};
   Id.__index = Id;
end

function CreateId(native)
   local newObject = {};
   setmetatable(newObject, Id);
   newObject._native = native;
   return newObject;
end

function Id_Create(id)
    if (nil == id) then
        return CreateId(Core.Id_Create());
    else
        return CreateId(Core.Id(id));
    end
end

function Id:ToString()
    return self._native:ToString();
end

--lua node
if (LuaNode == nil) then
   LuaNode = {};
   LuaNode.__index = LuaNode;
end

function CreateNode(native)
   local newObject = {};
   setmetatable(newObject, LuaNode);
   newObject._native = native;
   return newObject;
end

function Node_Create(id, alias)
    local node = Core.CreateNode(id._native, alias);
    return CreateNode(node);
end

function Node_Get(alias)
   local n = Core.GetNodeFromAlias(alias);
   
   if n ~= nil then
      return CreateNode(n);
   end
   
   return nil;
end

function LuaNode:RemoveFromScene()
    self._native:RemoveFromScene();
end

function LuaNode:AddToScene()
    self._native:AddToScene();
end

function LuaNode:Bind()
    self._native:Bind();
end

function LuaNode:AddComponent(id, type)
    local c = Core.CreateComponent(self._native, id._native, type);
    return CreateComponent(self, c);
end

function LuaNode:GetComponent(type)
   local c = self._native:GetComponent(type);
   return CreateComponent(self, c);
end

function LuaNode:DeleteComponent(id)
    Core.DeleteComponent(self._native, id._native);
end

function LuaNode:SetWorldPosition(p)
    self._native:SetWorldPosition(p);
end

function LuaNode:GetWorldPosition(p)
    return self._native:GetWorldPosition(p);
end

function LuaNode:GetWorldTransform(p)
    return self._native:GetWorldTransform(p);
end

function LuaNode:SetWorldTransform(p)
    return self._native:SetWorldTransform(p);
end

function LuaNode:SetTickable(tickable)
    self._native:SetTickable(tickable);
end

function LuaNode:SetParent(node)
    if (node ~= nil) then 
        self._native:SetParent(node._native);
    else
        self._native:SetParent(nil);
    end
end

function LuaNode:Destroy()
    self._native:Destroy();
end

function InputSystem_AddEventListener(eventName, method)
   Core.AddEventListener(Core.Id("InputSystem"), eventName, method);
end

function InputSystem_RemoveEventListener(eventName, method)
   Core.RemoveEventListener(Core.Id("InputSystem"), eventName, method);
end

do
local focusStack = {};
local focusLock = nil;
function Focus_Acquire(node)
    if focusStack[#focusStack] == node then
        return true
    else
        return Focus_Push(node);
    end
end

function Focus_Has(node)
    return node == focusStack[#focusStack];
end
    
function Focus_Push(node, lock)
    if focusLock ~= true then
        focusStack[#focusStack + 1] = node;
        focusLock = lock
        return true
    end
    
    return false
end

function Focus_Pop(node)
    if focusStack[#focusStack] == node then
        focusLock = false;
        focusStack[#focusStack] = nil;
    else
        OnError("Node being popped is not at the top");
    end
end
end

-- Lua Component
if (LuaComponent == nil) then
   LuaComponent = {};
   LuaComponent.__index = LuaComponent;
end

function LuaComponent:New()
   local newObject = { };
   setmetatable(newObject, LuaComponent);
   return newObject;
end

function LuaComponent:SetActive(active)
    self._native:SetActive(active);
end

function LuaComponent:GetParent()
    return self._node;
end

if (TextArea == nil) then
    TextArea = {};
    TextArea.__index = TextArea;
    TextArea.AlignLeft = Core.TextArea_AlignLeft;
    TextArea.AlignRight = Core.TextArea_AlignRight;
    TextArea.AlignCenter = Core.TextArea_AlignCenter;
    TextArea.AlignTop = Core.TextArea_AlignTop;
    TextArea.AlignBottom = Core.TextArea_AlignBottom;
    TextArea.AlignCenter = Core.TextArea_AlignCenter;
end

--Label Component
if (LabelComponent == nil) then 
    LabelComponent = LuaComponent:New{};
    LabelComponent.__index = LabelComponent;
end  

function CreateLabelComponent(node, native_component)
    local newObject = { }; 	
    setmetatable(newObject, LabelComponent); 	

    newObject._native = Core.ToLabelComponent(native_component);
    newObject._node =  node;

    return newObject; 
 end
 
function LabelComponent:Clear()
    self._native:Clear();
end
 
function LabelComponent:SetAlign(align, valign)
    self._native:SetAlign(align, valign);
end
 
function LabelComponent:Print(s)
    self._native:Print(s);
end

function LabelComponent:Create(width, height, r, g, b, a, back_material, front_material, font_map, font_texture, render_layers)
    local list = Core.CreateIdList(render_layers);
    
    self._native:Create(Math.Vector2(width, height), Math.Vector(r / 255, g / 255, b / 255, a / 255), 
                                           Core.GetResourceHandleFromAlias(back_material), Core.GetResourceHandleFromAlias(front_material), 
                                           Core.GetResourceHandleFromAlias(font_map), Core.GetResourceHandleFromAlias(font_texture), list);
    Core.DestroyIdList(list);
                                           
end

--MeshRenderer Component
if (MeshRendererComponent == nil) then 
    MeshRendererComponent = LuaComponent:New{};
    MeshRendererComponent.__index = MeshRendererComponent;
end  

function CreateMeshRendererComponent(node, native_component)
    local newObject = { }; 	
    setmetatable(newObject, MeshRendererComponent); 	
    
    newObject._native = Core.ToMeshRendererComponent(native_component);
    newObject._node =  node;
    
    return newObject; 
end
  
function MeshRendererComponent:Create(model, materials, renderGroups)
    local materialList = Core.CreateResourceHandleList_FromAlias(materials);
    local renderGroupList = Core.CreateIdList(renderGroups);

    self._native:Create(Core.GetResourceHandleFromAlias(model), materialList, renderGroupList);

    Core.DestroyResourceHandleList(materialList);
    Core.DestroyIdList(renderGroupList);
end

function MeshRendererComponent:Destroy()
    self._native:Destroy();
    self._native = nil;
end

function MeshRendererComponent:AddMaterial(index, materialAlias)
    self._native:AddMaterial(index, Core.GetResourceHandleFromAlias(materialAlias));
end

function MeshRendererComponent:CreateNodesFromBones()
    self._native:CreateNodesFromBones();
end

--DirectionalLightComponent Component
if (DirectionalLightComponent == nil) then 
    DirectionalLightComponent = LuaComponent:New{};
    DirectionalLightComponent.__index = DirectionalLightComponent;
end  

function CreateDirectionalLightComponent(node, native_component)
    local newObject = { }; 	
    setmetatable(newObject, DirectionalLightComponent); 	
    
    newObject._native = Core.ToDirectionalLightComponent(native_component);
    newObject._node =  node;
    
    return newObject; 
end
  
function DirectionalLightComponent:Create(renderGroups, color, intensity)
    local renderGroupList = Core.CreateIdList(renderGroups);

    self._native:Create(renderGroupList, color, intensity);

    Core.DestroyIdList(renderGroupList);
end

function DirectionalLightComponent:Destroy()
    self._native:Destroy();
    self._native = nil;
end

--PointLightComponent Component
if (PointLightComponent == nil) then 
    PointLightComponent = LuaComponent:New{};
    PointLightComponent.__index = PointLightComponent;
end  

function CreatePointLightComponent(node, native_component)
    local newObject = { }; 	
    setmetatable(newObject, PointLightComponent); 	
    
    newObject._native = Core.ToPointLightComponent(native_component);
    newObject._node =  node;
    
    return newObject; 
end
  
function PointLightComponent:Create(renderGroups, color, intensity, range)
    local renderGroupList = Core.CreateIdList(renderGroups);

    self._native:Create(renderGroupList, color, intensity, range);

    Core.DestroyIdList(renderGroupList);
end

function PointLightComponent:SetColor(red, green, blue)
    local vector = Vector_Create();
    vector.x = red;
    vector.y = green;
    vector.z = blue;
    vector.w = 1;
    self._native:SetColor(vector);
end

function PointLightComponent:Destroy()
    self._native:Destroy();
    self._native = nil;
end

--SpotLightComponent Component
if (SpotLightComponent == nil) then 
    SpotLightComponent = LuaComponent:New{};
    SpotLightComponent.__index = SpotLightComponent;
end  

function CreateSpotLightComponent(node, native_component)
    local newObject = { }; 	
    setmetatable(newObject, SpotLightComponent); 	
    
    newObject._native = Core.ToSpotLightComponent(native_component);
    newObject._node =  node;
    
    return newObject; 
end
  
function SpotLightComponent:Create(renderGroups, color, intensity, inner_angle, outer_angle, range)
    local renderGroupList = Core.CreateIdList(renderGroups);

    self._native:Create(renderGroupList, color, intensity, inner_angle, outer_angle, range);

    Core.DestroyIdList(renderGroupList);
end

function SpotLightComponent:SetColor(red, green, blue)
    local vector = Vector_Create();
    vector.x = red;
    vector.y = green;
    vector.z = blue;
    vector.w = 1;
    self._native:SetColor(vector);
end

function SpotLightComponent:Destroy()
    self._native:Destroy();
    self._native = nil;
end

--AmbientLightComponent Component
if (AmbientLightComponent == nil) then 
    AmbientLightComponent = LuaComponent:New{};
    AmbientLightComponent.__index = AmbientLightComponent;
end  

function CreateAmbientLightComponent(node, native_component)
    local newObject = { }; 	
    setmetatable(newObject, AmbientLightComponent); 	

    newObject._native = Core.ToAmbientLightComponent(native_component);
    newObject._node =  node;
    
    return newObject; 
end
  
function AmbientLightComponent:Create(renderGroups, color, intensity)
    local renderGroupList = Core.CreateIdList(renderGroups);

    self._native:Create(renderGroupList, color, intensity);

    Core.DestroyIdList(renderGroupList);
end

function AmbientLightComponent:Destroy()
    self._native:Destroy();
    self._native = nil;
end

--Animation Component
if (AnimationComponent == nil) then 
    AnimationComponent = LuaComponent:New{};
    AnimationComponent.__index = AnimationComponent;
end  

function CreateAnimationComponent(node, native_component)
    local newObject = { }; 	
    setmetatable(newObject, AnimationComponent); 	
    
    newObject._native = Core.ToAnimationComponent(native_component);
    newObject._node =  node;
    
    return newObject; 
end
  
function AnimationComponent:Create(animation, start_playing)
    self._native:Create(Core.GetResourceHandleFromAlias(animation), start_playing);
end

function AnimationComponent:Destroy()
    self._native:Destroy();
    self._native = nil;
end

--Script Component
if (ScriptComponent == nil) then 
    ScriptComponent = LuaComponent:New{};
    ScriptComponent.__index = ScriptComponent;
end  

function CreateScriptComponent(node, native_component)
    local newObject = { }; 	
    setmetatable(newObject, ScriptComponent); 	
    
    newObject._native = Core.ToScriptComponent(native_component);
    newObject._node =  node;
    
    return newObject; 
end
  
function ScriptComponent:Create(script)
    self._native:Create(Core.GetResourceHandleFromAlias(script));
end

function ScriptComponent:GetScript(script_name)
    return _G[Core.GetLuaObjectName(self._native)];
end

--base component
function CreateComponent(node, native)
    local stype = native:GetType():ToString();    
    if (stype == "LabelComponent") then
        return CreateLabelComponent(node, native);
    elseif (stype == "ScriptComponent") then
        return CreateScriptComponent(node, native);
    elseif (stype == "MeshRendererComponent") then
        return CreateMeshRendererComponent(node, native);
    elseif (stype == "AnimationComponent") then
        return CreateAnimationComponent(node, native);
    elseif (stype == "DirectionalLightComponent") then
        return CreateDirectionalLightComponent(node, native);
    elseif (stype == "PointLightComponent") then
        return CreatePointLightComponent(node, native);
    elseif (stype == "SpotLightComponent") then
        return CreateSpotLightComponent(node, native);
    elseif (stype == "AmbientLightComponent") then
        return CreateAmbientLightComponent(node, native);
    else
        OnError("Component type unknown: "..stype)
    end

    return nil;
end

   --[[
   
    local lua = Core.GetLuaObjectName(component);
    
    --Already has a lua object associated with it?
    if (lua ~= nil) then
        return _G[lua];
    end
    
    -- Create a lua object to associate with it
    if (component:GetType():ToString() == "LabelComponent") then
        return CreateLabelComponent(component);
    elseif (component:GetType():ToString() == "ButtonComponent") then
        return CreateButtonComponent(component);
    elseif (component:GetType():ToString() == "SplineComponent") then
        return CreateSplineComponent(component);
    else
        return CreateLuaComponent(component);
    end
    
    return nil;
]]

--[[
function LuaNode:GetComponent(type)
    local c = self._native:GetComponent(type);
    
    if (nil == c) then
        c = Core.GetScriptComponent(self._native, type);
    end
    
    if (nil ~= c) then
        return GetComponentFromId(c:GetId());
    end
    
    return nil;
end

function LuaNode:GetId()
    return self._native:GetId();
end

function LuaNode:GetName()
    return self._native:GetName();
end

function LuaNode:GetWorldTransform()
    return self._native:GetWorldTransform();
end

function LuaNode:GetLocalTransform()
    return self._native:GetLocalTransform();
end

function LuaNode:FindChildByName(name)
    local n = self._native:FindChildByName(name);
    if (nil ~= n) then
        n = GetNodeFromNode(n);
    end
    
    return n;
end

function LuaNode:GetChildren()
    local nodeList = Core.NodeList();
    self._native:GetChildren(nodeList);

    local nodes = {};
    
    for i = 0, nodeList:GetSize() - 1 do
        local n = GetNodeFromId(nodeList:Get(i):GetId());
        table.insert(nodes, n);
    end
        
    return nodes;
end

function LuaNode:GetScene()
    return self._native:GetScene();
end

function LuaNode:GetParent()
    local p = self._native:GetParent();
    
    if (nil ~= p) then
        p = GetNodeFromNode(p);
    end
    
    return p;
end

function LuaNode:SetWorldTransform(t)
    self._native:SetWorldTransform(t);
end

--base component
if (LuaComponent == nil) then
   LuaComponent = {};
   LuaComponent.__index = LuaComponent;
end

function CreateLuaComponent(native)
    local newObject = { }; 	
    setmetatable(newObject, LuaComponent); 	
    newObject:_InternalCreate(native);
    return newObject; 
 end

 function LuaComponent:New()
   local newObject = { };
   setmetatable(newObject, LuaComponent);
   return newObject;
end

function LuaComponent:_InternalCreate(native)
   self._native = native;
   self._node = nil;
end

function LuaComponent:_Create()
    self:_InternalCreate(Core.GetContextComponent());
end

function LuaComponent:_Destroy()
    StopAllCoroutines(self);
end

function LuaComponent:GetType()
    return self._native:GetType();
end

function LuaComponent:AddToScene()
    return self._native:AddToScene();
end

function LuaComponent:RemoveFromScene()
    return self._native:RemoveFromScene();
end

function LuaComponent:GetNode()
    if (self._node == nil) then
        self._node = GetNodeFromNode(self._native:GetParent());
    end
    return self._node;
end

function LuaComponent:GetId()
    return self._native:GetId();
end

function CreateButtonComponent(native)
    local newObject = { }; 	
    setmetatable(newObject, ButtonComponent); 	
    newObject:_InternalCreate(Core.ToButtonComponent(native));
    return newObject; 
 end
 
 --Spline Component
if (SplineComponent == nil) then 
    SplineComponent = LuaComponent:New{};
    SplineComponent.__index = SplineComponent;
end  

function CreateSplineComponent(native)
    local newObject = { }; 	
    setmetatable(newObject, SplineComponent); 	
    newObject:_InternalCreate(Core.ToSplineComponent(native));
    return newObject; 
 end
 
 function SplineComponent:GetPositionOnSpline(position)
    return self._native:GetPositionOnSpline(position);
 end
 
 function SplineComponent:GetTransform(p)
    return self._native:GetTransform(p);
 end

 function SplineComponent:GetLength()
    return self._native:GetLength();
 end

 function SplineComponent:GetClosestParam(position)
    return self._native:GetClosestParam(position);
 end

 function SplineComponent:GetClosestTransform(position)
    return self._native:GetClosestTransform(position);
 end

 --Wrapper and Base routines
g_Nodes = {};
g_Components = {};

function RemoveComponent(c)
    g_Components[c] = nil;
end

function RemoveNode(n)
    g_Nodes[n] = nil;
end

function GetNodeFromId(id)
    local key = id._native:ToString();
    
    if (nil == g_Nodes[key]) then
        local native = Core.GetNodeFromId(id._native);
        if (nil ~= native) then
            g_Nodes[key] = CreateNode(native);
        end
    end
    
    return g_Nodes[key];
end

function GetNodeFromNode(native)
    local key = native:GetId():ToString();
    
    if (nil == g_Nodes[key]) then
        if (nil ~= native) then
            g_Nodes[key] = CreateNode(native);
        end
    end
    
    return g_Nodes[key];
end

function GetComponentFromId(id)
    local key = id:ToString();

    if (nil == g_Components[key]) then
        local native = Core.GetComponentFromId(id);
        if (nil ~= native) then
            g_Components[key] = CreateComponent(native);
        end
    end

    return g_Components[key];
end

function GetNodeByName(name)
    for k, v in g_Nodes do
        if (v:GetName() == name) then
            return v;
        end
    end

    local n = Core.GetNodeByName(name);

    if (n ~= nil) then
        return GetNodeFromNode(n);
    else
        return nil;
    end   
end
]]
