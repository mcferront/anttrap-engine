#include "EnginePch.h"

#include "CameraComponent.h"
#include "Node.h"

DefineComponentType(CameraComponent, new CameraComponentSerializer);

void CameraComponent::Create(
    Id id
    )
{
    SetId( id );
    m_Enabled = false;
}

void CameraComponent::Destroy( void )
{
    Component::Destroy( );
}

void CameraComponent::AddToScene( void )
{
    if ( false == GetParent()->IsInScene() ) return;

    m_Enabled = true;

    Component::AddToScene( );
}

void CameraComponent::RemoveFromScene( void )
{
    m_Enabled = false;

    Component::RemoveFromScene( );
}

Camera *CameraComponent::GetCamera( void ) 
{ 
    m_Camera.SetWorldTransform( GetParent()->GetWorldTransform() );
    return &m_Camera;
}

ISerializable *CameraComponentSerializer::Deserialize(
    Serializer *pSerializer,
    ISerializable *pSerializable
    )
{
    return NULL;

    //if ( NULL == pSerializable ) pSerializable = new CameraComponent; 

    //CameraComponent *pCameraComponent = (CameraComponent *) pSerializable;

    //Id id = Id::Deserialize( pSerializer->GetInputStream() );
    //Id renderTree = Id::Deserialize( pSerializer->GetInputStream() );

    //pCameraComponent->Create( id, renderTree );

    //Color color;
    //int clear, order;
    //float nearClip, farClip;
    //byte perspective, gameCamera;

    //Vector viewport;

    //pSerializer->GetInputStream()->Read( &color, sizeof(color) );
    //pSerializer->GetInputStream()->Read( &clear, sizeof(clear) );
    //pSerializer->GetInputStream()->Read( &nearClip, sizeof(nearClip) );
    //pSerializer->GetInputStream()->Read( &farClip, sizeof(farClip) );
    //pSerializer->GetInputStream()->Read( &order, sizeof(order) );
    //pSerializer->GetInputStream()->Read( &perspective, sizeof(perspective) );
    //pSerializer->GetInputStream()->Read( &gameCamera, sizeof(gameCamera) );
    //pSerializer->GetInputStream()->Read( &viewport, sizeof(viewport) );

    //if ( 0 != perspective )
    //{
    //    float hfov, aspectRatio;

    //    pSerializer->GetInputStream()->Read( &hfov, sizeof(hfov) );
    //    pSerializer->GetInputStream()->Read( &aspectRatio, sizeof(aspectRatio) );
    //
    //    pCameraComponent->m_Camera.CreateAsPerspective( hfov, aspectRatio, nearClip, farClip );
    //}
    //else
    //{
    //    float scale;

    //    pSerializer->GetInputStream()->Read( &scale, sizeof(scale) );

    //    pCameraComponent->m_Camera.CreateAsOrtho( scale, nearClip, farClip );
    //}

    //pCameraComponent->m_Order = order;

    //pCameraComponent->m_Camera.SetViewportRect( viewport );
    //pCameraComponent->m_Camera.SetClearColor( color );
    //pCameraComponent->m_Camera.SetClearFlags( clear);

    //return pSerializable;
}
