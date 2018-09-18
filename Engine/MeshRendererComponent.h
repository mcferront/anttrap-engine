#pragma once

#include "EngineGlobal.h"
#include "Component.h"
#include "RenderableMesh.h"

class MeshRendererComponent : public Component
{
public:
    DeclareComponentType(MeshRendererComponent);

private:
    RenderableMesh m_Model;
    ResourceHandleList m_Materials;

public:
    void Create(
        ResourceHandle model,
        const ResourceHandleList &materials,
        const IdList &renderGroups
        );

    virtual void Destroy( void );

    virtual void Bind( void );
    
    void AddMaterial(
        int surface, 
        ResourceHandle material
        );

    virtual void AddToScene( void );
    virtual void RemoveFromScene( void );

    void CreateNodesFromBones( void );

    ResourceHandle GetMesh( void ) const { return m_Model.GetModel(); }
};

class MeshRendererComponentSerializer : public ISerializer
{
public:
    virtual bool Serialize(
        Serializer *pSerializer,
        const ISerializable *pSerializable
        )
    {
        return false;
    }

    virtual ISerializable *Deserialize(
        Serializer *pSerializer,
        ISerializable *pSerializable
        );

    virtual ISerializable *Instantiate() const { return new MeshRendererComponent; }

    virtual const SerializableType &GetSerializableType( void ) const { return MeshRendererComponent::StaticSerializableType( ); }

    virtual uint32 GetVersion( void ) const { return 1; }
};
