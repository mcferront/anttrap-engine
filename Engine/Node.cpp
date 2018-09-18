#include "EnginePch.h"

#include "Node.h"
#include "ResourceWorld.h"
#include "Component.h"
#include "Sprite.h"
#include "SceneAsset.h"
#include "LuaVM.h"

DefineResourceType(Node, Resource, new NodeSerializer);

HashTable<Id, Node*> Node::s_ActiveNodes;
HashTable<Id, Node*> Node::s_AllNodes;

void Node::Create( void )
{
    m_pScene = NULL;
    m_StartActive = true;
    m_FirstAdd = true;

    m_Components.Create( 4, 4, IdHash, IdCompare );
    m_ComponentList.Create( );
    m_ComponentCopy.Create( );

    m_LocalTransform = Math::IdentityTransform( );
    m_WorldTransform = Math::IdentityTransform( );

    m_ChildrenIds.Create( );
    m_Children.Create( );

    m_CollisionLayer = Id::Empty;
    m_SceneId = Id::Empty;

    m_pParent = NULL;

    m_ParentFlags.SetDefault( );

    m_Flags.SetTransformDirty( true );
    PropogateFlags( );
}

void Node::Bind( void )
{
    Resource::Bind( );

    Node *pParent;

    // Make sure the parent knows about us 
    if ( s_AllNodes.Get(m_ParentId, &pParent) )
        SetParent( pParent );

    //Attach our children and attach me to the parent
    //We have to do both fixups because a live update
    //might be modifying a single node in a hierarchy
    BindChildren( );

    //copy the list because they might add or remove components
    //during tick and it would cause our list order to blow up
    m_ComponentCopy.CopyFrom( m_ComponentList );

    List<ComponentDesc *>::Enumerator e = m_ComponentCopy.GetEnumerator( );

    while ( e.EnumNext() )
        if ( NULL != e.Data()->pComponent ) e.Data()->pComponent->Bind( );
}

void Node::Destroy( void )
{
    //Detach ourselves from our parent
    SetParent( NULL );

    //Make sure we're out of the scene
    RemoveFromScene( );

    // Remove from global list
    s_AllNodes.Remove( GetId() );

    // Destroy the components
    int i, size = m_ComponentList.GetSize( );

    for ( i = 0; i < size; i++ )
    {
        ComponentDesc *pDesc = m_ComponentList.GetAt( i );
        if ( pDesc->pComponent ) pDesc->pComponent->Destroy( );

        delete m_ComponentList.GetAt( i );
    }

    // Destroy all children
    if ( m_Children.GetSize() > 0 )
    {
        NodeList children;
        children.CopyFrom( m_Children );

        m_Children.Clear( );

        //Copy list because calling destroy
        //on these children will cause them to attempt
        //to remove themselves from us
        {
            List<Node*>::Enumerator e = children.GetEnumerator( );

            while ( e.EnumNext() )
            {
                e.Data( )->Destroy( );
                delete e.Data( );
            }
        }
    }

    m_ComponentList.Destroy( );
    m_ComponentCopy.Destroy( );
    m_Components.Destroy( );

    m_ChildrenIds.Destroy( );

    if ( NULL != m_pScene )
        m_pScene->RemoveNode( this );

    Resource::Destroy( );
}

void Node::BindChildren( void )
{
    DetachChildren( );

    // Make sure all our children know about us
    uint32 size = m_ChildrenIds.GetSize();

    for (uint32 i = 0; i < size; i++)
    {
        Node *pChild;
        Id id = m_ChildrenIds.GetAt( i );

        if ( s_AllNodes.Get(id, &pChild) )
            pChild->SetParent( this );
    }

    {
        List<Node*>::Enumerator e = m_Children.GetEnumerator( );

        while ( e.EnumNext() )
            e.Data( )->Bind( );
    }
}

void Node::DetachChildren( void )
{
    if ( m_Children.GetSize() == 0 ) return;

    NodeList children;
    children.CopyFrom( m_Children );

    m_Children.Clear( );

    //Copy list because calling destroy
    //on these children will cause them to attempt
    //to remove themselves from us
    {
        List<Node*>::Enumerator e = children.GetEnumerator( );

        while ( e.EnumNext() )
            e.Data( )->SetParent( NULL );
    }
}

void Node::AddToScene( void )
{
    Resource::AddToScene( );

    if ( m_FirstAdd )
    {
        s_AllNodes.Add( GetId(), this );

        if ( NULL == m_pScene )
        {
            ResourceHandle handle(m_SceneId);

            if ( true == IsResourceLoaded(handle) )
            {
                Scene *pScene = GetResource(handle, Scene);
                AttachToScene( pScene );
            }
        }

        m_FirstAdd = false;
    
        m_Flags.SetTransformDirty( true );
        PropogateFlags( );

        if ( false == m_StartActive )
            return;
    }

    m_Flags.SetInScene( true );
    PropogateFlags( );

    UpdateSceneState( );
}

void Node::RemoveFromScene( void )
{
    if ( false == m_Flags.IsInScene() )
        return;

    m_Flags.SetInScene( false );
    PropogateFlags( );

    UpdateSceneState( );

    Resource::RemoveFromScene( );
}

void Node::Tick( 
    float deltaSeconds
)
{
    Resource::Tick( deltaSeconds );

    //copy the list because they might add or remove components
    //during tick and it would cause our list order to blow up
    m_ComponentCopy.CopyFrom( m_ComponentList );

    List<ComponentDesc *>::Enumerator e = m_ComponentCopy.GetEnumerator( );

    while ( e.EnumNext() )
    {
        if ( NULL != e.Data()->pComponent && e.Data()->pComponent->IsActive() ) e.Data()->pComponent->Tick( deltaSeconds );
    }
}

void Node::PostTick( void )
{
    Resource::PostTick( );

    //copy the list because they might add or remove components
    //during tick and it would cause our list order to blow up
    m_ComponentCopy.CopyFrom( m_ComponentList );

    List<ComponentDesc *>::Enumerator e = m_ComponentCopy.GetEnumerator( );

    while ( e.EnumNext() )
    {
        if ( NULL != e.Data()->pComponent && e.Data()->pComponent->IsActive() ) e.Data()->pComponent->PostTick( );
    }
}

void Node::Final( void )
{
    Resource::Final( );

    //copy the list because they might add or remove components
    //during tick and it would cause our list order to blow up
    m_ComponentCopy.CopyFrom( m_ComponentList );

    List<ComponentDesc *>::Enumerator e = m_ComponentCopy.GetEnumerator( );

    while ( e.EnumNext() )
    {
        if ( NULL != e.Data()->pComponent && e.Data()->pComponent->IsActive() ) e.Data()->pComponent->Final( );
    }
}

void Node::EditorRender( void )
{
    Resource::EditorRender( );

    //copy the list because they might add or remove components
    //during tick and it would cause our list order to blow up
    m_ComponentCopy.CopyFrom( m_ComponentList );

    List<ComponentDesc *>::Enumerator e = m_ComponentCopy.GetEnumerator( );

    while ( e.EnumNext() )
    {
        if ( NULL != e.Data()->pComponent && e.Data()->pComponent->IsActive() ) e.Data()->pComponent->EditorRender( );
    }
}

void Node::SetWorldTransform(
    const Transform &transform
)
{
    Transform parent;
    GetParentTransform( &parent );

    Math::Invert( &parent, parent );

    Transform local;
    Math::Multiply( &local, transform, parent );

    SetLocalTransform( local );
}

void Node::SetLocalTransform(
    const Transform &transform
)
{
    m_LocalTransform = transform;

    m_Flags.SetTransformDirty( true );
    PropogateFlags( );
}

void Node::GetLocalTransform(
    Transform *pTransform
) const
{
    *pTransform = m_LocalTransform;
}

void Node::GetWorldTransform(
    Transform *pTransform
) const
{
    *pTransform = GetWorldTransform( );
}

Transform Node::GetLocalTransform( void ) const
{
    return m_LocalTransform;
}

Transform Node::GetWorldTransform( void ) const
{
    UpdateWorldTransform( );

    return m_WorldTransform;
}

void Node::AddComponent(
    Component *pComponent
)
{
    ComponentDesc *pDesc;

    if ( false == m_Components.Get(pComponent->GetId(), &pDesc) )
    {
        pDesc = new ComponentDesc;
        pDesc->pComponent = NULL;
        pDesc->id = pComponent->GetId();

        m_Components.Add( pComponent->GetId(), pDesc );
        m_ComponentList.Add( pDesc );
    }
    
    pDesc->pComponent = pComponent;
    pDesc->pComponent->m_pParent = this;
}


void Node::DeleteComponent(
   Id id
)
{
    ComponentDesc *pComponent;

    if ( true == m_Components.Get(id, &pComponent) )
    {
        if ( NULL != pComponent->pComponent )
        {
            pComponent->pComponent->Destroy( );

            delete pComponent->pComponent;
            pComponent->pComponent = NULL;
        }
    }
}

void Node::AttachToScene(
    Scene *pScene
)
{
    if ( m_pScene )
        m_pScene->RemoveNode( this );

    m_pScene = pScene;

    if ( m_pScene )
        m_pScene->AddNode( this );
}

void Node::GetComponents(
   ComponentList *pList
) const
{
    int i, size = m_ComponentList.GetSize( );

    for ( i = 0; i < size; i++ )
    {
        ComponentDesc *pDesc = m_ComponentList.GetAt( i );

        if ( NULL != pDesc->pComponent ) 
        {
            pList->Add( pDesc->pComponent );
        }
    }
}

Node *Node::FindChildByName(
    const char *pName
) const
{
    uint32 size = m_Children.GetSize();

    Node *pNode = NULL;

    for (uint32 i = 0; i < size; i++)
    {
        pNode = m_Children.Get(i);
        if ( StringRefEqual(pNode->GetName(), pName) )
            break;

       pNode = pNode->FindChildByName( pName );
       if ( pNode != NULL )
           break;

    }

    return pNode;
}

Component *Node::GetComponent(
    const ComponentType &type
    ) const
{
    int i, size = m_ComponentList.GetSize( );

    for ( i = 0; i < size; i++ )
    {
        ComponentDesc *pDesc = m_ComponentList.GetAt( i );

        if ( NULL != pDesc->pComponent ) 
        {
            if (pDesc->pComponent->GetType() == type)
                return pDesc->pComponent;
        }
    }

    return NULL;
}

Component *Node::GetComponent(
    const char *pType
    ) const
{
    int i, size = m_ComponentList.GetSize( );

    for ( i = 0; i < size; i++ )
    {
        ComponentDesc *pDesc = m_ComponentList.GetAt( i );

        if ( NULL != pDesc->pComponent ) 
        {
            if (0 == strcmp(pDesc->pComponent->GetType().ToString(), pType))
                return pDesc->pComponent;
        }
    }

    return NULL;
}

void Node::GetParentTransform(
    Transform *pParent
) const
{
    if ( NULL != m_pParent )
        m_pParent->GetWorldTransform( pParent );
    else
        *pParent = Math::IdentityTransform( );
}

void Node::UpdateWorldTransform( void ) const
{
    if ( m_CombinedFlags.IsTransformDirty() )
    {
        Transform parent;
        GetParentTransform( &parent );

        Math::Multiply( &m_WorldTransform, m_LocalTransform, parent );

        m_Flags.SetTransformDirty( false );
        PropogateFlags( );
    }
}

void Node::UpdateSceneState( void )
{
    if ( m_CombinedFlags.IsInScene() )
    {
        if ( false == Node::s_ActiveNodes.Contains(GetId()) )
        {
            Resource::AddToScene( );
    
            Node::s_ActiveNodes.Add( GetId(), this );

            List<ComponentDesc *>::Enumerator e = m_ComponentList.GetEnumerator( );

            while ( e.EnumNext() )
                if ( NULL != e.Data()->pComponent && e.Data()->pComponent->IsActive() ) e.Data()->pComponent->AddToScene( );

            m_Flags.SetTransformDirty( true );
            PropogateFlags( );
        }
    }
    else
    {
        if ( true == Node::s_ActiveNodes.Contains(GetId()) )
        {
            List<ComponentDesc *>::Enumerator e = m_ComponentList.GetEnumerator( );

            while ( e.EnumNext() )
                if ( NULL != e.Data()->pComponent ) e.Data()->pComponent->RemoveFromScene( );
    
            Node::s_ActiveNodes.Remove( GetId() );

            Resource::RemoveFromScene( );
        }
    }

    uint32 size = m_Children.GetSize();

    for (uint32 i = 0; i < size; i++)
        m_Children.Get( i )->UpdateSceneState( );
}

ISerializable *NodeSerializer::Deserialize(
   Serializer *pSerializer,
   ISerializable *pSerializable
)
{
    if ( NULL == pSerializable ) pSerializable = new Node; 

    Node *pNode = (Node *) pSerializable;
    pNode->Create();

    Id id = Id::Deserialize( pSerializer->GetInputStream() );
    Id name = Id::Deserialize( pSerializer->GetInputStream() );
    Id tickLayer = Id::Deserialize( pSerializer->GetInputStream() );
    Id collisionLayer = Id::Deserialize( pSerializer->GetInputStream() );

    pNode->m_CollisionLayer = collisionLayer;

    Quaternion rotation;
    Vector position, scale;

    pSerializer->GetInputStream( )->Read( &rotation, sizeof(rotation), NULL );
    pSerializer->GetInputStream( )->Read( &position, sizeof(position), NULL );
    pSerializer->GetInputStream( )->Read( &scale,    sizeof(scale),    NULL );

    pNode->m_LocalTransform = Transform(rotation, position, scale);

    byte selected;
    pSerializer->GetInputStream( )->Read( &selected, sizeof(selected), NULL );

    byte active;
    pSerializer->GetInputStream( )->Read( &active, sizeof(active), NULL );

    pNode->m_ParentId = Id::Deserialize( pSerializer->GetInputStream() );
    pNode->m_SceneId = Id::Deserialize( pSerializer->GetInputStream() );

    int childCount;
    pSerializer->GetInputStream( )->Read( &childCount, sizeof(childCount), NULL );

    for (int i = 0; i < childCount; i++)
    {
        id = Id::Deserialize( pSerializer->GetInputStream() );
        pNode->m_ChildrenIds.Add( id );
    }

    int componentCount;
    pSerializer->GetInputStream( )->Read( &componentCount, sizeof(componentCount), NULL );

    for (int i = 0; i < componentCount; i++)
    {
        ISerializable *pComponent = pSerializer->Deserialize( NULL );
        pNode->AddComponent( (Component *) pComponent );
    }

    pNode->m_Flags.SetSelected(selected != 0);
    pNode->m_StartActive = active != 0;
    pNode->SetTickable( true );

    return pSerializable;
}
