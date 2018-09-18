#pragma once

#include "EngineGlobal.h"
#include "Asset.h"

class Node;

class Scene : public Asset
{
    friend class SceneSerializer;

private:
    List<Node *> m_Nodes;
    MemoryStream m_Stream;

public:
    void Create( );

    void Destroy( void );
    void Reload( void );

    virtual void Bind( void );

    virtual void AddToScene( void );
    virtual void RemoveFromScene( void );

    void AddNode(
        Node *pNode
        );

    void RemoveNode(
        Node *pNode
        );

private:
    void DestroyNodes( void );
    void Deserialize( void );

public:
    DeclareResourceType( Scene );
};

class SceneSerializer : public ISerializer
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

    virtual ISerializable *Instantiate( ) const { return new Scene; }

    virtual const SerializableType &GetSerializableType( void ) const { return Scene::StaticSerializableType( ); }

    virtual uint32 GetVersion( void ) const { return 1; }
};
