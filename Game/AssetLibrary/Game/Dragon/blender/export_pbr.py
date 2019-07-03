import bpy
import os
import uuid;

import mathutils
from math import radians

g_pass = "Forward"
g_shader_guid = "36B48296-CF38-4FA1-8F5E-9B53AD69C75F"
g_envmap_guid = "46E817C6-2BCA-4354-8CA6-C3E006A6309B"
g_diffuse_ibl_map_guid = "8DDAFB8A-180F-4EB0-A6F8-B2720A4630DA"
g_brdf_lut_guid = "B3AB969F-DA7E-4C91-9B75-4B30FDE48ADC"

g_scene_template_file = "E:\\git\\mcferront\\anttrap-engine\\Game\\AssetLibrary\\Game\\Dragon\\blender\\template.lua"
g_material_template_file = "E:\\git\\mcferront\\anttrap-engine\\Game\\AssetLibrary\\Game\\Dragon\\blender\\template.material"
g_output_path = "E:\\git\\mcferront\\anttrap-engine\\Game\\AssetLibrary\\Game\\Dragon\\"
g_scene_name = "dragon"

g_scene = bpy.context.scene
g_objects = bpy.context.scene.objects

at_objects = []
at_materials = {}

def create_engine_transform(v, q):
	output = "Transform_Set(Vector_Set("
	output += str(v[0]) + "," + str(v[2]) + "," + str(v[1]) + "), Quaternion_Set("
	output += str(q[1]) + "," + str(q[2]) + "," + str(q[3]) + "," + str(q[0]) + "))"
	return output
	
def parse_type(t):
	if isinstance(t, bpy.types.NodeSocketColor):
		return str(t.default_value[0]) + "," + str(t.default_value[1]) + "," + str(t.default_value[2]) + "," + str(t.default_value[3]);
	elif isinstance(t, bpy.types.NodeSocketFloatFactor) or isinstance(t, bpy.types.NodeSocketFloat):
		return str(t.default_value);
	elif isinstance(t, bpy.types.NodeSocketVector):
		return str(t.default_value[0]) + "," + str(t.default_value[1]) + "," + str(t.default_value[2]);
	else:
		return "undefined: " + str(type(t));

class AT_Material:
	def __init__(self, _material):
		self.material = _material;
		self.name = _material.name;
		self.guid = str(uuid.uuid4()).upper()
		
	def export(self, template):
		node = self.material.node_tree.nodes["Principled BSDF"]
		template = template.replace("$PASS", g_pass)
		template = template.replace("$SHADER", g_shader_guid)
		template = template.replace("$ENVMAP", g_envmap_guid)
		template = template.replace("$DIF_IBL_MAP", g_diffuse_ibl_map_guid)
		template = template.replace("$BRDF_LUT", g_brdf_lut_guid)
		for input in node.inputs:
			#print(input.name.upper().replace(" ", "_"));
			macro_name = "$" + input.name.upper().replace(" ", "_") + "\n"
			template = template.replace(macro_name, parse_type(input) + "\n");
		return template
		
	def print_desc(self):
		print(self.name + "...");
		node = self.material.node_tree.nodes["Principled BSDF"]
		for input in node.inputs:
			print("\t" + input.name + ' = ' + parse_type(input));
			
		
def create_materials(object):
	materials = []
	for m, material in enumerate(object.data.materials):
		at_material = AT_Material(material)
		materials.append(at_material)

	return materials

	
class AT_Mesh:
	def __init__(self, _path, _mesh):
		self.mesh = _mesh;
		self.guid = str(uuid.uuid4()).upper()
		self.fullPath = _path + "/" + _mesh.name
		self.materials = []
		
	def export(self):
		mesh_blob  = "   node = Node_Create(Id_Create(), \"" + self.mesh.name + "\");\n"
		mesh_blob += "   node:SetWorldTransform(" + create_engine_transform(self.mesh.location, self.mesh.rotation_quaternion) + ");\n"
		mesh_blob += "   mesh = node:AddComponent(Id_Create(), \"MeshRendererComponent\");\n"
		mesh_blob += "   mesh:Create(\"" + self.mesh.name.split('.')[0] + ".mesh\",\""
		for m, material in enumerate(self.mesh.data.materials):
			mesh_blob += material.name + ".material,"
		mesh_blob = mesh_blob[:-1]
		mesh_blob += "\", \"Geometry\");\n"
		mesh_blob += "   node:SetParent(self._node);\n"
		mesh_blob += "   node:AddToScene();\n"
		mesh_blob += "   node:Bind();\n"
			
		return mesh_blob;
		
	def print_desc(self):
		print(self.mesh.name + "(" + self.mesh.type + ")...");
		print("\tpos: " + str(self.mesh.location));
		print("\trot: " + str(self.mesh.rotation_quaternion));
		
class AT_Camera:
	def __init__(self, _path, _camera):
		self.camera = _camera;
		self.guid = str(uuid.uuid4()).upper()
		self.fullPath = _path + "/" + _camera.name
		self.materials = []
		
	def export(self):
		camera_blob  = "   node = Node_Get(\"MainCamera\");\n"
		camera_blob += "   node:SetWorldTransform(" + create_engine_transform(self.camera.location, self.camera.rotation_quaternion) + ");\n"
		camera_blob += "   camera = node:GetComponent(\"CameraComponent\");\n"
		camera_blob += "   camera:SetFov(" + str(self.camera.data.angle_x) + ");\n"
		camera_blob += "   camera:SetNearClip(" + str(self.camera.data.clip_start) + ");\n"
		camera_blob += "   camera:SetFarClip(" + str(self.camera.data.clip_end) + ");\n"

		camera_blob += "   script = node:AddComponent(Id_Create(), \"ScriptComponent\");\n"
		camera_blob += "   script:Create(\"FirstPersonController.lua\");\n"

		camera_blob += "   node:SetParent(self._node);\n"
		camera_blob += "   node:AddToScene();\n"
		camera_blob += "   node:Bind();\n"

		camera_blob += "   script:GetScript().pitch = {:.2f}".format(radians(90)  - self.camera.rotation_euler[0]) + ";\n"
		camera_blob += "   script:GetScript().yaw = {:.2f}".format(- self.camera.rotation_euler[2]) + ";\n"
		camera_blob += "   script:GetScript().roll = {:.2f}".format(- self.camera.rotation_euler[1]) + ";\n"
   
		return camera_blob;
		
	def print_desc(self):
		print(self.camera.name + "(" + self.camera.type + ")...");
		print("\tpos: " + str(self.camera.location));
		print("\trot: " + str(self.camera.rotation_quaternion));
		print("\teul: " + str(self.camera.rotation_euler)); 
		print("\tfov: " + str(self.camera.data.angle));
		print("\tnear: " + str(self.camera.data.clip_start));
		print("\tfar: " + str(self.camera.data.clip_end));
		
class AT_Lamp:
	def __init__(self, _path, _lamp):
		self.lamp = _lamp;
		self.guid = str(uuid.uuid4()).upper()
		self.fullPath = _path + "/" + _lamp.name
		self.materials = []
	
	def export(self):
		light_blob  = "   node = Node_Create(Id_Create(), \"" + self.lamp.name + "\");\n"
		light_blob += "   node:SetWorldTransform(" + create_engine_transform(self.lamp.location, self.lamp.rotation_quaternion) + ");\n"
		light_blob += "   light = node:AddComponent(Id_Create(), \"DirectionalLightComponent\");\n"
		light_blob += "   light:Create(\"Geometry\", Vector_Set("
		light_blob += 		str(self.lamp.data.color[0]) + "," + str(self.lamp.data.color[1]) + "," + str(self.lamp.data.color[2]) + "),"
		light_blob += 		str(self.lamp.data.energy) + ");\n"
		light_blob += "   node:SetParent(self._node);\n"
		light_blob += "   node:AddToScene();\n"
		light_blob += "   node:Bind();\n"
		return light_blob;
		
	def print_desc(self):
		print(self.lamp.name + "(" + self.lamp.type + ")...");
		print("\tpos: " + str(self.lamp.location));
		print("\trot: " + str(self.lamp.rotation_quaternion));
		print("\tcolor: " + str(self.lamp.data.color));
		print("\tfalloff: " + str(self.lamp.data.distance));
		print("\tenergy: " + str(self.lamp.data.energy));

def gather_hierarchies(path, root):
	for index, child in enumerate(root.children, 1):
		gather_hierarchies(path + "/" + root.name, child);
#"LAMP", "CAMERA"
	#print(root.type);
	if len(root.children) == 0:
		at_object = None;
		if root.type == 'MESH' and root.hide_render == False:
			at_object = AT_Mesh(path, root)
		elif root.type == 'LAMP':
			at_object = AT_Lamp(path, root)
		elif root.type == 'CAMERA':
			at_object = AT_Camera(path, root)

		if at_object is not None:
			#at_object.print_desc();
			at_objects.append(at_object);
		
			if isinstance(at_object, AT_Mesh):
				materials = create_materials(at_object.mesh)

				if len(materials) > 0:
					for i, material in enumerate(materials):
						if material.name not in at_materials:
							at_materials[material.name] = material

						material = at_materials[material.name]
						#material.print_desc();
						at_object.materials.append(material)
	
def gather_materials():
	for obj in g_objects:
		if obj.parent == None:
			gather_hierarchies("", obj);


print("")
print("Principled BSDF exporter:");

gather_materials();

print("loading templates...")
file = open(g_scene_template_file, "r")
scene_template = file.read()
file.close()

file = open(g_material_template_file, "r")
material_template = file.read()
file.close()

print("exporting materials...")
for key, m in at_materials.items():
	file = open(g_output_path + m.name + ".material", "w")
	file.write(m.export(material_template))
	file.close()
	
light_creation = "";
camera_creation = "";
mesh_creation = "";

print("exporting scene...")
for o in at_objects:
	if isinstance(o, AT_Lamp):
		light_creation += o.export() + "\n";
	elif isinstance(o, AT_Mesh):
		mesh_creation += o.export() + "\n";
	elif isinstance(o, AT_Camera):
		camera_creation = o.export();

		
scene = scene_template
scene = scene.replace("$NAME", g_scene_name)
scene = scene.replace("$LIGHTS", light_creation)
scene = scene.replace("$CAMERA", camera_creation)
scene = scene.replace("$MESHES", mesh_creation)

print("writing scene...")
file = open(g_output_path + g_scene_name + ".lua", "w")
file.write(scene)
file.close()

unique_meshes = {}
for o in at_objects:
	if isinstance(o, AT_Mesh):
		root_name = o.mesh.name.split('.')[0] + ".mesh"
		unique_meshes[root_name] = o;

scene_guid = str(uuid.uuid4()).upper();

print("writing ids...")
file = open(g_output_path + g_scene_name + ".ids", "w")

for k,v in unique_meshes.items():
	file.write(v.guid + "," + k + "," + k + "\n")

for k,m in at_materials.items():
	file.write(m.guid + "," + k + ".material," + k + ".material\n")

file.write(scene_guid + "," + g_scene_name + ".lua" + "," + g_scene_name + ".lua" + "\n")
file.write(g_shader_guid + ",,principled_bsdf.shader" + "\n")
file.write(g_envmap_guid + ",,env_map.tif" + "\n")
file.write(g_diffuse_ibl_map_guid + ",,diff_map.tif" + "\n")
file.write(g_brdf_lut_guid + ",,brdf_lut.tif" + "\n")
file.close()

print("writing packagedesc...")
file = open(g_output_path + g_scene_name + ".packagedesc", "w")
file.write(scene_guid + "\t#" + g_scene_name + ".lua" + "\n")
file.write(g_shader_guid + "\t#principled_bsdf.shader" + "\n")
file.write(g_envmap_guid + "\t#env_map.tif" + "\n")
file.write(g_diffuse_ibl_map_guid + "\t#dif_map.tif" + "\n")
file.write(g_brdf_lut_guid + "\t#brdf_lut.tif" + "\n")

for k,m in at_materials.items():
	file.write(m.guid + "\t#" + k + "\n")

for k,v in unique_meshes.items():
	file.write(v.guid + "\t#" + k + "\n")
file.close()


print("done");
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	