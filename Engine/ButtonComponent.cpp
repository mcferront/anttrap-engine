#include "EnginePch.h"

#include "ButtonComponent.h"
#include "MaterialAsset.h"

DefineComponentType(ButtonComponent, new ButtonComponentSerializer);

void ButtonComponent::Create(
    Id id,
    ResourceHandle upMaterial,
    ResourceHandle downMaterial,
    ResourceHandle hoverMaterial,
    const Vector2 &size,
    const IdList &renderGroups,
    Id touchStageId
    )
{
    SetId( id );
    m_Button.Create( this, upMaterial, downMaterial, hoverMaterial, size, renderGroups, touchStageId ); 
    m_pTextArea = NULL;
    m_MainMaterial = upMaterial;
    m_RenderGroups.CopyFrom( renderGroups );
}

void ButtonComponent::Destroy( void )
{
    if ( NULL != m_pTextArea )
        m_pTextArea->Destroy( );

    delete m_pTextArea;

    m_MainMaterial = NullHandle;

    m_Button.Destroy( );

    Component::Destroy( );
}

void ButtonComponent::AddToScene( void )
{
    if ( false == GetParent()->IsInScene() ) return;

    Component::AddToScene( );
    m_Button.AddToScene( );

    if ( NULL != m_pTextArea )
        m_pTextArea->AddToScene( );
}

void ButtonComponent::Bind( void )
{
    Component::Bind( );

    m_Button.Bind( );

    if ( NULL != m_pTextArea )
    {
        m_pTextArea->SetSize( *m_Button.GetSize() );
        m_pTextArea->SetRenderGroups( m_RenderGroups );
    }
}

void ButtonComponent::RemoveFromScene( void )
{
    if ( NULL != m_pTextArea )
        m_pTextArea->RemoveFromScene( );

    m_Button.RemoveFromScene( );

    Component::RemoveFromScene( );
}

void ButtonComponent::SetupText( 
    const char *pString, 
    const Vector &textColor, 
    int hPad, 
    int vPad, 
    TextArea::Align horiz_align, 
    TextArea::Align vert_align, 
    ResourceHandle material,
    ResourceHandle fontMap,
    ResourceHandle fontTexture
    )
{
    if ( NULL != m_pTextArea ) 
        m_pTextArea->Destroy( );
    else
        m_pTextArea = new TextArea;

    m_pTextArea->Create( this, *m_Button.GetSize(), textColor, NullHandle, fontMap, material, fontTexture, m_RenderGroups );

    m_pTextArea->SetAlign( horiz_align, vert_align );
    m_pTextArea->Pad( (float) vPad, (float) hPad );
    m_pTextArea->Print( pString );
}

ISerializable *ButtonComponentSerializer::Deserialize(
    Serializer *pSerializer,
    ISerializable *pSerializable
    )
{
    return NULL;

    //if ( NULL == pSerializable ) pSerializable = new ButtonComponent; 

    //ButtonComponent *pButtonComponent = (ButtonComponent *) pSerializable;

    //Id id = Id::Deserialize( pSerializer->GetInputStream() );

    //IdList renderGroups;
    //Id::DeserializeList( pSerializer->GetInputStream(), &renderGroups );

    //Id up = Id::Deserialize( pSerializer->GetInputStream() );
    //Id down = Id::Deserialize( pSerializer->GetInputStream() );
    //Id hover = Id::Deserialize( pSerializer->GetInputStream() );

    //int color[4];
    //pSerializer->GetInputStream()->Read( color, sizeof(color) );

    //int width, height;

    //pSerializer->GetInputStream()->Read( &width, sizeof(width) );
    //pSerializer->GetInputStream()->Read( &height, sizeof(height) );

    //Id string = Id::Deserialize( pSerializer->GetInputStream() );

    //int textColorD[4];
    //pSerializer->GetInputStream()->Read( textColorD, sizeof(textColorD) );

    //int hPad, vPad;
    //pSerializer->GetInputStream()->Read( &hPad, sizeof(hPad) );
    //pSerializer->GetInputStream()->Read( &vPad, sizeof(vPad) );

    //Id hAlignSz = Id::Deserialize( pSerializer->GetInputStream() );
    //Id vAlignSz = Id::Deserialize( pSerializer->GetInputStream() );
    //Id fontMap  = Id::Deserialize( pSerializer->GetInputStream() );

    //ResourceHandle upMaterial = up == Id::Empty ? NullHandle : ResourceHandle(up);
    //ResourceHandle downMaterial = down == Id::Empty ? NullHandle : ResourceHandle(down);
    //ResourceHandle hoverMaterial = hover == Id::Empty ? NullHandle : ResourceHandle(hover);

    //pButtonComponent->Create( id, upMaterial, downMaterial, hoverMaterial, 
    //    Vector2((float)width, (float)height), renderGroups, Id("Default") );

    //Vector textColor = Vector(textColorD[0] / 255.0f, textColorD[1] / 255.0f, textColorD[2] / 255.0f, textColorD[3] / 255.0f);

    //TextArea::VAlign vAlign;
    //TextArea::Align hAlign;

    //if ( 0 == strcmp(vAlignSz.ToString(), "Top") )
    //    vAlign = TextArea::VAlignTop;
    //else if ( 0 == strcmp(vAlignSz.ToString(), "Bottom") )
    //    vAlign = TextArea::VAlignBottom;
    //else
    //    vAlign = TextArea::VAlignCenter;

    //if ( 0 == strcmp(hAlignSz.ToString(), "Left") )
    //    hAlign = TextArea::AlignLeft;
    //else if ( 0 == strcmp(hAlignSz.ToString(), "Right") )
    //    hAlign = TextArea::AlignRight;
    //else
    //    hAlign = TextArea::AlignCenter;

    //if (string != Id::Empty)
    //    pButtonComponent->SetupText( string.ToString(), textColor, hPad, vPad, vAlign, hAlign, upMaterial, ResourceHandle(fontMap) );

    //pButtonComponent->SetColor( Vector(color[0] / 255.0f, color[1] / 255.0f, color[2] / 255.0f, color[3] / 255.0f) );

    //return pSerializable;
}

