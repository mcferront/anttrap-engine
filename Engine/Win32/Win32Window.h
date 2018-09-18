#pragma once

#include "EngineGlobal.h"
#include "GraphicsApi.h"
#include "Identifiable.h"

class Window : public Identifiable
{
private:
   HWND   m_hWnd;
   
public:
    void Create(
        Id id
    );

    void Create(
        Id id,
        const Window &copyFrom
    );

    void Destroy( void );

    void SetHandle(
        HWND hWnd
    );

    void BeginRender( void );
   
   void EndRender( 
      bool present
      );

    void Copy(
        const Window &copyFrom
    )
    {
        SetId( copyFrom.GetId( ) );
        m_hWnd = copyFrom.m_hWnd;
    }

    void GetDimensions( 
        int *pWidth, 
        int *pHeight
    ) const;

    bool CanRender( void );

    HWND GetHandle( void ) const { return m_hWnd; }
};
