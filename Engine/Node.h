#pragma once

#include "EngineGlobal.h"
#include "Resource.h"

class Component;
class Scene;

struct ComponentType;

typedef List<Component*> ComponentList;

class AnttrapNode;

class NodeList
{
private:
    List<Node*> m_Nodes;
    bool m_Created;

public:
    NodeList( void ) { m_Created = false; }
    ~NodeList( void )
    {
        if ( false == m_Created )
            return;

        m_Nodes.Destroy( );
        m_Created = false;
    }

    void CopyFrom( const NodeList &list ) { Create( ); m_Nodes.CopyFrom( list.m_Nodes ); }

    void Clear( void ) { Create( ); m_Nodes.Clear( ); }
    void Add( Node *pNode ) { Create( ); m_Nodes.Add( pNode ); }
    void Remove( Node *pNode ) { Create( ); m_Nodes.Remove( pNode ); }
    void AddUnique( Node *pNode ) { Create( ); m_Nodes.AddUnique( pNode ); }

    int GetSize( void ) const { return (int) m_Nodes.GetSize( ); }
    Node *Get( int i ) const { return m_Nodes.GetAt( i ); }
    List<Node*>::Enumerator GetEnumerator( void ) const { return m_Nodes.GetEnumerator( ); }

    void Create( void )
    {
        if ( true == m_Created ) return;

        m_Nodes.Create( );
        m_Created = true;
    }

    NodeList & operator = ( const NodeList &rhs )
    {
        CopyFrom( rhs );

        return *this;
    }
};

class AnttrapNode : public Resource
{
private:
    struct Flags
    {
        Flags( )
        {
            flags = 0;
        }

        uint32 flags;

        static const uint32 Flags_InScene = 1;
        static const uint32 Flags_TransformDirty = 2;
        static const uint32 Flags_Selected = 4;

        void SetInScene( bool value )         { value ? flags |= Flags_InScene : flags &= ~Flags_InScene; }
        void SetTransformDirty( bool value )  { value ? flags |= Flags_TransformDirty : flags &= ~Flags_TransformDirty; }
        void SetSelected( bool value )        { value ? flags |= Flags_Selected : flags &= ~Flags_Selected; }

        bool IsInScene( )        const { return 0 != ( flags & Flags_InScene ); }
        bool IsTransformDirty( ) const { return 0 != ( flags & Flags_TransformDirty ); }
        bool IsSelected( )       const { return 0 != ( flags & Flags_Selected ); }

        void SetDefault( void )
        {
            SetInScene( true );
            SetTransformDirty( false );
            SetSelected( false );
        }

        Flags Combine(
            const Flags &other
            ) const
        {
            Flags f;
            f.flags = ( ( other.flags & Flags_InScene ) & ( flags & Flags_InScene ) ) |
                ( ( other.flags & Flags_TransformDirty ) | ( flags & Flags_TransformDirty ) ) |
                ( ( other.flags & Flags_Selected ) | ( flags & Flags_Selected ) );

            return f;
        }

        bool operator == ( const Flags &rhs ) const
        {
            return flags == rhs.flags;
        }

        bool operator != ( const Flags &rhs ) const
        {
            return flags != rhs.flags;
        }
    };

    struct ComponentDesc
    {
        Id id;
        Component *pComponent;
    };

    typedef List<ComponentDesc*> ComponentDescList;

    friend class NodeSerializer;

private:
    //hash table for quick name lookup
    //and a list to guarantee order
    HashTable<Id, ComponentDesc*> m_Components;
    ComponentDescList m_ComponentList;
    ComponentDescList m_ComponentCopy;

    NodeList m_Children;
    Node  *m_pParent;

    Transform m_LocalTransform;
    IdList m_ChildrenIds;
    Id m_ParentId;
    Id m_SceneId;
    Id m_CollisionLayer;

    Scene *m_pScene;
    bool m_StartActive;
    bool m_FirstAdd;

    mutable Transform m_WorldTransform;
    mutable Flags     m_Flags;
    mutable Flags     m_ParentFlags;
    mutable Flags     m_CombinedFlags;

public:
    DeclareResourceType( Node );

    void Create( void );

    void Bind( void );

    void Destroy( void );

    void BindChildren( void );
    void DetachChildren( void );

    void AddToScene( void );
    void RemoveFromScene( void );

    void Tick(
        float deltaSeconds
        );

    virtual void PostTick( void );
    virtual void Final( void );

    virtual void EditorRender( void );

    void SetWorldTransform(
        const Transform &transform
        );

    void SetLocalTransform(
        const Transform &transform
        );

    void GetWorldTransform(
        Transform *pTransform
        ) const;

    void GetLocalTransform(
        Transform *pTransform
        ) const;

    Transform GetWorldTransform( void ) const;
    Transform GetLocalTransform( void ) const;

    void GetComponents(
        ComponentList *pList
        ) const;

    Node *FindChildByName(
        const char *pName
        ) const;

    Component *GetComponent(
        const ComponentType &type
        ) const;

    Component *GetComponent(
        const char *pType
        ) const;

    template <typename t_item>
    t_item *GetComponent( ) const
    {
        return (t_item *) GetComponent( t_item::StaticType( ) );
    }

    Component *GetComponent(
        Id id
        ) const
    {

        ComponentDesc *pComponent;

        if ( m_Components.Get( id, &pComponent ) )
        {
            return pComponent->pComponent;
        }

        return NULL;
    }

    void AddComponent(
        Component *pComponent
        );

    void DeleteComponent(
        Id id
        );

    void AttachToScene(
        Scene *pScene
        );

    void SetSelected(
        bool selected
        )
    {
        m_Flags.SetSelected( selected );
        PropogateFlags( );
    }

    bool IsSelected( void ) const
    {
        return m_CombinedFlags.IsSelected( );
    }

    bool IsInScene( void ) const
    {
        return m_CombinedFlags.IsInScene( );
    }

    void SetParent(
        Node *pNode
        )
    {
        if ( NULL != m_pParent )
            m_pParent->m_Children.Remove( this );

        m_pParent = pNode;

        if ( NULL != m_pParent )
        {
            m_pParent->m_Children.AddUnique( this );
            m_pParent->m_ChildrenIds.AddUnique( this->GetId() );
            m_ParentFlags = m_pParent->m_CombinedFlags;
        }
        else
            m_ParentFlags.SetDefault( );

        m_Flags.SetTransformDirty( true );
        PropogateFlags( );
    }

    Node *GetParent( void ) const
    {
        return m_pParent;
    }

    void GetChildren(
        NodeList *pList
        ) const
    {
        pList->CopyFrom( m_Children );
    }

    void SetCollisionLayer( Id layer ) { m_CollisionLayer = layer; }
    Id GetCollisionLayer( void ) const { return m_CollisionLayer; }

    void SetWorldPosition(
        const Vector &position
        )
    {
        Transform t = GetWorldTransform( );
        t.SetTranslation( position );

        SetWorldTransform( t );
    }

    void GetWorldPosition(
        Vector *pPosition
        ) const
    {
        GetWorldTransform( ).GetTranslation( pPosition );
    }

    Scene *GetScene( void ) const { return m_pScene; }

private:
    void GetParentTransform(
        Transform *pParent
        ) const;

    void UpdateWorldTransform( void ) const;
    void UpdateSceneState( void );

    void PropogateFlags( void ) const
    {
        // Any dirty parent transform will
        // also dirty us until we recompute.
        // This is different than purely a combined state because a combined state
        // would allow the parent to undirty the transform
        // without us ever having updated ours
        if ( m_ParentFlags.IsTransformDirty( ) )
            m_Flags.SetTransformDirty( true );

        Flags combined = m_Flags.Combine( m_ParentFlags );

        if ( combined != m_CombinedFlags )
        {
            m_CombinedFlags = combined;

            uint32 size = m_Children.GetSize( );

            for ( uint32 i = 0; i < size; i++ )
                m_Children.Get( i )->PropogateFlags( m_CombinedFlags );
        }
    }

    void PropogateFlags(
        const Flags &parentFlags
        ) const
    {
        m_ParentFlags = parentFlags;
        PropogateFlags( );
    }

public:
    static Node *GetNode( Id id )
    {
        Node *pNode;

        if ( true == s_ActiveNodes.Get( id, &pNode ) )
            return pNode;

        return NULL;
    }

    static Enumerator<Id, Node *> GetEnumerator( void )
    {
        return s_ActiveNodes.GetEnumerator( );
    }

    static HashTable<Id, Node *> s_ActiveNodes;

    // only to be used internally to look up disabled parents
    static HashTable<Id, Node *> s_AllNodes;
};

class NodeSerializer : public ISerializer
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

    virtual ISerializable *Instantiate( ) const { return new Node; }

    virtual const SerializableType &GetSerializableType( void ) const { return Node::StaticSerializableType( ); }

    virtual uint32 GetVersion( void ) const { return 1; }
};
