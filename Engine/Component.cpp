#include "EnginePch.h"

#include "Component.h"

DefineComponentType(Component, NULL);

HashTable<Id, Component*> Component::s_ActiveComponents;