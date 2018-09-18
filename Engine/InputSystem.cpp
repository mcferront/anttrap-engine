#include "EnginePch.h"

#include "InputSystem.h"
#include "Channel.h"
#include "ChannelSystem.h"
#include "Resource.h"

InputMap::InputMap(
    const char *pMapping
    )
{
    m_pMapping = StringRef(pMapping);

    m_NewPress = false;
    m_NewRelease = false;
}

bool InputMap::IsPressed( void ) const
{
    return InputSystem::Instance( ).GetValue( m_pMapping ) > 0;
}

bool InputMap::IsReleased( void ) const
{
    return InputSystem::Instance( ).GetValue( m_pMapping ) == 0;
}

bool InputMap::IsNewPress( void ) const
{
    bool result = IsPressed( );

    if ( false == result )
    {
        m_NewPress = false;
        return false;
    }
    if ( false == m_NewPress )
    {
        m_NewPress = true;
        return true;
    }
    
    return false;
}

bool InputMap::IsNewRelease( void ) const
{
    bool result = IsReleased( );

    if ( false == result )
    {
        m_NewRelease = false;
        return false;
    }
    if ( false == m_NewRelease )
    {
        m_NewRelease = true;
        return true;
    }

    return false;
}

void InputMap::Press( void )
{
    InputSystem::Instance( ).SetValue( m_pMapping, 1.0f );
}

void InputMap::Release( void )
{
    InputSystem::Instance( ).SetValue( m_pMapping, 0.0f );
}

void InputMap::SetValue( float v )
{
    return InputSystem::Instance( ).SetValue( m_pMapping, v );
}

float InputMap::GetValue( void ) const
{
    return InputSystem::Instance( ).GetValue( m_pMapping );
}

InputSystem &InputSystem::Instance( void )
{
    static InputSystem s_instance;
    return s_instance;
}

void InputSystem::Create( void )
{
    m_States.Create( );

    // InputSystem should be a resource with a channel
    // so it doesn't hackily create a resource handle channel it uses
    m_Channel = ResourceHandle("InputSystem");

    //m_pChannel = new Channel;
    //m_pChannel->Create( Id( "InputSystem" ), NULL );
    //ChannelSystem::Instance( ).Add( m_pChannel );
}

void InputSystem::Destroy( void )
{
   m_Channel = NullHandle;

    //ChannelSystem::Instance( ).Remove( m_pChannel );
    //m_pChannel->Destroy( );
    //delete m_pChannel;

    m_States.Destroy( );
}

float InputSystem::GetValue(
    const char *pMapping
    )
{
    float value = 0;

    m_States.Get( pMapping, &value );

    return value;
}

void InputSystem::SetValue(
    const char *pMapping,
    float value
    )
{
    const char *pRetKey;
    float retData;

    pMapping = StringRef(pMapping);

    m_States.Remove( pMapping, &pRetKey, &retData );
    m_States.Add( pMapping, value );
}

void InputSystem::KeyDown(
    int keyCode,
    const char *pModifiers
    )
{
    static const char *pKey = StringRef("KeyDown");
    m_Channel.GetChannel()->SendEvent( pKey, ArgList( keyCode, pModifiers ) );
}

void InputSystem::KeyUp(
    int keyCode,
    const char *pModifiers
    )
{
    static const char *pKey = StringRef("KeyUp");
    m_Channel.GetChannel()->SendEvent( pKey, ArgList( keyCode, pModifiers ) );
}
