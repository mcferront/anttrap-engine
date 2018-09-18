#include "EnginePch.h"

#include "LabelComponent.h"
#include "RenderWorld.h"
#include "MaterialAsset.h"

DefineComponentType(LabelComponent, new LabelComponentSerializer);

void LabelComponent::Create(
    const Vector2 &size,
    const Vector &color,
    ResourceHandle back_material,
    ResourceHandle front_material,
    ResourceHandle font_map,
    ResourceHandle font_texture,
    const IdList &renderGroups
    )
{
    m_TextArea.Create( this, size, color, back_material, front_material, font_map, font_texture, renderGroups );
}

void LabelComponent::Destroy( void )
{
    m_TextArea.Destroy( );

    Component::Destroy( );
}

void LabelComponent::AddToScene( void )
{
    if ( false == GetParent()->IsInScene() ) return;

    Component::AddToScene( );
    m_TextArea.AddToScene( );
}

void LabelComponent::RemoveFromScene( void )
{
    m_TextArea.RemoveFromScene( );
    Component::RemoveFromScene( );
}

ISerializable *LabelComponentSerializer::Deserialize(
    Serializer *pSerializer,
    ISerializable *pSerializable
    )
{
    return NULL;
    //if ( NULL == pSerializable ) pSerializable = new LabelComponent; 

    //LabelComponent *pLabelComponent = (LabelComponent *) pSerializable;

    //Id id = Id::Deserialize( pSerializer->GetInputStream() );
    //Id font = Id::Deserialize( pSerializer->GetInputStream() );
    //Id text = Id::Deserialize( pSerializer->GetInputStream() );
    //
    //IdList renderGroups;
    //Id::DeserializeList( pSerializer->GetInputStream(), &renderGroups );
    //
    //int color[4];
    //pSerializer->GetInputStream()->Read( color, sizeof(color) );

    //Id hAlign = Id::Deserialize( pSerializer->GetInputStream() );
    //Id vAlign = Id::Deserialize( pSerializer->GetInputStream() );

    //int hPad, vPad, width, height;

    //pSerializer->GetInputStream()->Read( &hPad, sizeof(hPad) );
    //pSerializer->GetInputStream()->Read( &vPad, sizeof(vPad) );
    //pSerializer->GetInputStream()->Read( &width, sizeof(width) );
    //pSerializer->GetInputStream()->Read( &height, sizeof(height) );

    //pLabelComponent->Create( id, Vector2((float)width, (float)height), 
    //    Vector(color[0] / 255.0f, color[1] / 255.0f, color[2] / 255.0f, color[3] / 255.0f),
    //    NullHandle, ResourceHandle(font), renderGroups );

    //TextArea::VAlign v;
    //TextArea::Align h;

    //if ( 0 == strcmp(vAlign.ToString(), "Top") )
    //    v = TextArea::VAlignTop;
    //else if ( 0 == strcmp(vAlign.ToString(), "Bottom") )
    //    v = TextArea::VAlignBottom;
    //else
    //    v = TextArea::VAlignCenter;

    //if ( 0 == strcmp(hAlign.ToString(), "Left") )
    //    h = TextArea::AlignLeft;
    //else if ( 0 == strcmp(hAlign.ToString(), "Right") )
    //    h = TextArea::AlignRight;
    //else
    //    h = TextArea::AlignCenter;

    //pLabelComponent->SetVAlign( v );
    //pLabelComponent->SetAlign( h );
    //pLabelComponent->Pad( (float) vPad, (float) hPad );
    //pLabelComponent->Print( text.ToString() );

    //return pSerializable;
}
