import bpy
import os
import uuid;

g_pass = "Forward"
g_shader = "36B48296-CF38-4FA1-8F5E-9B53AD69C75F #blender_principled_bsdf"

g_scene_template_file = "C:\\Users\\trapper.mcferron\\Desktop\\pbr\\template.lua"
g_material_template_file = "C:\\Users\\trapper.mcferron\\Desktop\\pbr\\template.material"
g_output_path = "C:\\Users\\trapper.mcferron\\Desktop\\pbr\\"
g_scene_name = "dragon"			

g_scene = bpy.context.scene
g_objects = bpy.context.scene.objects

at_objects = []
at_materials = {}

def create_engine_transform(v, q):
	output = "Transform_Set(Vector_Set("
	output += str(v[0]) + "," + str(v[1]) + "," + str(v[2]) + "), Quaternion_Set("
	output += str(q[3]) + "," + str(q[0]) + "," + str(q[1]) + "," + str(q[2]) + "))"
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
		
	def export(self, template):
		node = self.material.node_tree.nodes["Principled BSDF"]
		template = template.replace("$PASS", g_pass)
		template = template.replace("$SHADER", g_shader)
		for input in node.inputs:
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
		mesh_blob += "   mesh:Create(\"" + self.mesh.name + ".mesh\",\""
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
		camera_blob += "   camera:SetFov(" + str(self.camera.data.angle) + ");\n"
		camera_blob += "   camera:SetNearClip(" + str(self.camera.data.clip_start) + ");\n"
		camera_blob += "   camera:SetFarClip(" + str(self.camera.data.clip_end) + ");\n"
		camera_blob += "   node:SetParent(self._node);\n"
		camera_blob += "   node:AddToScene();\n"
		camera_blob += "   node:Bind();\n"
		return camera_blob;
		
	def print_desc(self):
		print(self.camera.name + "(" + self.camera.type + ")...");
		print("\tpos: " + str(self.camera.location));
		print("\trot: " + str(self.camera.rotation_quaternion));
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
		if root.type == 'MESH':
			at_object = AT_Mesh(path, root)
		elif root.type == 'LAMP':
			at_object = AT_Lamp(path, root)
		elif root.type == 'CAMERA':
			at_object = AT_Camera(path, root)

		if at_object is not None:
			at_objects.append(at_object);
		
			if isinstance(at_object, AT_Mesh):
				materials = create_materials(at_object.mesh)

				if len(materials) > 0:
					for i, material in enumerate(materials):
						if material.name not in at_materials:
							at_materials[material.name] = material

						material = at_materials[material.name]
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

print("done");
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	