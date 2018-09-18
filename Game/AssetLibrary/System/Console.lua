if (Console == nil) then
   Console = LuaComponent:New{};
   Console.__index = Console;
end

function Console:New()
    local newObject = { };
    setmetatable(newObject, Console);
    newObject._native = Core.GetContextComponent();
    newObject._node = CreateNode(newObject._native:GetParent());
    newObject.string = "";
    newObject.history_index = 0;
    newObject.label = nil;
    newObject.find_next_is_valid = false;
    newObject.find_next_string_index = 0;
    newObject.history_table = {};
    return newObject;
end

function Console:_Create()
    self.label = self._node:AddComponent(Id_Create(), "LabelComponent");
    print("Creating "..Screen.width.."x"..Screen.height.."\n")
    self.label:Create(Screen.width, 56, 255, 255, 255, 255, "Grey.material", "DefaultFont.material", "DefaultFont.fontmap", "DefaultFont.bmp", "UI");
    self.label:SetAlign(TextArea.AlignLeft, TextArea.AlignBottom);

    self._node:SetWorldPosition(Math.Vector(0, Screen.height / 2 - 56 / 2, 0));
    InputSystem_AddEventListener("KeyDown", "OnKeyDown");    
    
    self._node:RemoveFromScene();
end

function Console:UpperSymbol(key)
    if ( '1'  == key ) then return '!'; end
    if ( '2'  == key ) then return '@'; end
    if ( '3'  == key ) then return '#'; end
    if ( '4'  == key ) then return '$'; end
    if ( '5'  == key ) then return '%'; end
    if ( '6'  == key ) then return '^'; end
    if ( '7'  == key ) then return '&'; end
    if ( '8'  == key ) then return '*'; end
    if ( '9'  == key ) then return '('; end
    if ( '0'  == key ) then return ')'; end
    if ( '-'  == key ) then return '_'; end
    if ( '='  == key ) then return '+'; end
    if ( '\'' == key ) then return '\"'; end
    if ( '\\' == key ) then return '|'; end
    if ( ','  == key ) then return '<'; end
    if ( '.'  == key ) then return '>'; end
    if ( '/'  == key ) then return '?'; end
    if ( '`'  == key ) then return '~'; end
    if ( '['  == key ) then return '{'; end
    if ( ']'  == key ) then return '}'; end
    if ( ';'  == key ) then return ':'; end

    return key;
end

do
local in_scene = false;
function Console:OnKeyDown(sender, event, key, modifiers)
    
    keyAscii = get_low_8_bits(key, 8);
    keyScan = get_high_8_bits(key, 8);

    --self.string = keyAscii.." "..keyScan;
    
    if (keyAscii == 96) then --'~' 
        if (false == in_scene ) then 
            Focus_Push(self, true);
            self._node:AddToScene();
            self._node:Bind();
            StartCoroutine(self, "Console:Active");
        else 
            Focus_Pop(self);
            self._node:RemoveFromScene();
            StopCoroutine(self, "Console:Active");
        end
        in_scene = false == in_scene;
        return;
    end
    
    if (false == in_scene) then
        return;
    end
    
   if (keyAscii == 8) then --backspace
       self:resetFindString();
       if (string.len(self.string) > 0) then
           self.string = string.sub(self.string, 0, string.len(self.string) - 1)
       end
   elseif (keyAscii == 27) then --escape
       self:resetFindString();
       self.string = "";
   elseif (keyAscii == 13) then --enter
       self:resetFindString();
       local result = self:execute_string();
       table.insert(self.history_table, { self.string, result });
       self.string = "";
       self.history_index = #self.history_table + 1;
   elseif (keyAscii == 9) then --tab
        if (string.sub(modifiers, 1, 1) == "1" or string.sub(modifiers, 2, 2) == '1') then
           self.string = self:preview_string(false);
        else
           self.string = self:preview_string(true);
        end
   elseif (keyAscii >= 32 and keyAscii <= 126) then
       self:resetFindString();
       if (string.sub(modifiers, 1, 1) == "1" or string.sub(modifiers, 2, 2) == '1') then   --shift
           self.string = self.string..self:UpperSymbol(string.char(keyAscii));
       else
           self.string = self.string..string.lower(string.char(keyAscii));
       end
   elseif (keyScan == 38 and self.history_index > 1) then --up arrow
       self:resetFindString();
       self.history_index = self.history_index - 1;
       self.string = self.history_table[self.history_index][1];
   elseif (keyScan == 40 and self.history_index < #self.history_table) then --down arrow
       self:resetFindString();
       self.history_index = self.history_index + 1;
       self.string = self.history_table[self.history_index][1];
   end
end
end

function Console_Error_Handler(err)
    print(err);
end

function Console:resetFindString()
   self.find_next_string_index = 0;
   self.find_next_is_valid = false;
   self.find_string = nil;
end

function Console:preview_string(forward)   
   if self.find_string == nil then
      self.find_string = self.string
   end
   
   if forward == true and self.find_next_is_valid == true then
      self.find_next_string_index = self.find_next_string_index + 1;
   elseif forward == false and self.find_next_string_index > 0 then
      self.find_next_string_index = self.find_next_string_index - 1;
   elseif forward == false then  -- if we're going backwards and we can't go back any further - return the original string
      return self.find_string;
   end
   
	local s = Core.FindClosestCommand(self.find_string:gsub( "%s$", ""), self.find_next_string_index)

   if s ~= nil then
      self.find_next_is_valid = true;
      s = s.." ";
   else
      if forward == true and self.find_next_string_index > 0 then --back the index up one, we've reached the end
         self.find_next_string_index = self.find_next_string_index - 1;
      end

      self.find_next_is_valid = false;
      s = self.string;      
   end

   return s;
end

function Console:execute_string()
	local result = Core.HandleConsoleCommand(self.string);
   
   if result == nil then
		local f, s = xpcall(loadstring(self.string), Console_Error_Handler);
		if (s == true) then 
         result = f(); 
      else
         result = "undefined command";
      end
	end
   
   if result == nil then
      result = "";
   end
   
   return result;
end

function Console:Active()
    local blink = {"_", ""};
    local i = 1;
    
    local prev_t = 0;
    local acc_t = 0;
    
    while true do
        self.label:Clear();

        for i = 1, #self.history_table do
            local v = self.history_table[i];
            self.label:Print(v[1].." => "..v[2]);
        end
        
        self.label:Print(">"..self.string..blink[i]);
        
        acc_t = acc_t + Time.deltaTime;
        if acc_t - prev_t > .25 then
            i = i + 1;
            if ( i == 3 ) then i = 1; end
            prev_t = acc_t;
        end

        WaitForSeconds(0);
    end
end
