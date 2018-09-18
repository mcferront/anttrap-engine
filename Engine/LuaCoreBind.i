%module Core
%{

#include "EnginePch.h"
#include "UtilityMath.h"
#include "LuaMathModule.h"
#include "LuaCoreModule.h"
#include "Node.h"
#include "SplineComponent.h"
#include "LabelComponent.h"
#include "ScriptComponent.h"
#include "MeshRendererComponent.h"
#include "Animation3dComponent.h"
#include "LightComponent.h"
#include "Raycast.h"
#include "SceneAsset.h"

extern "C" 
{
	#include "lua.h"
	#include "lualib.h"
	#include "lauxlib.h"
}
%}

%nodefaultctor;

%include "LuaCoreModule.h"

class InputMap
{
public:
   InputMap(
      const char *pMapping
   );

   void Press( void );
   void Release( void );

   bool IsPressed( void ) const;
   bool IsReleased( void ) const;

   bool IsNewPress( void ) const;
   bool IsNewRelease( void ) const;

   float GetValue( void ) const;
};

class LuaArgList
{
public:
    LuaArgList();

    void AddString(
        const char *pString
    );
    void AddFloat(
        float value
    );
    void AddInt(
        int value
    );

    void AddBool(
        bool value
    );

    void AddVector(
        const Vector &value
    ); 

    void AddResourceHandle(
        ResourceHandle value
    );

    ArgList *GetList( );
};

class Resource
{
public:
   ResourceHandle GetHandle( void );

   void SetTickable(
      bool tickable
      );
};

class Asset : public Resource
{};

class Scene : public Asset
{
public:
    void Reload( void );
};

class Node;

class NodeList
{
public:
    NodeList( void );
    ~NodeList( void );

    int GetSize( void ) const;
    Node *Get( int i ) const;

    void Add( Node *pNode );
};

class Node : public Resource
{
public:
    void Destroy();
   
    void SetWorldTransform(
        const Transform &transform
    );

    void SetLocalTransform(
        const Transform &transform
    );

    Component *GetComponent(
        const char *pType
    );

    Node *FindChildByName(
        const char *pName
    ) const;

    void GetChildren( 
        NodeList *pList 
    ) const;

    void GetWorldTransform( 
      Transform *pTransform 
    ) const;

    void GetLocalTransform( 
      Transform *pTransform
      ) const;

    void SetWorldPosition(
        const Vector &position
        );

    void GetWorldPosition(
        Vector *pPosition
        ) const;

    const char *GetName( void ) const;

    Id GetId( void ) const;

    void AddToScene( void );
    void RemoveFromScene( void );

    void SetParent(
        Node *pNode
        );

    Node *GetParent( void ) const;

    Scene *GetScene( void ) const;

    void Bind( void );
};

class Component
{
public:
    virtual void AddToScene( void );
    virtual void RemoveFromScene( void );

    Node *GetParent( void ) const;

    virtual const ComponentType &GetType( void ) const;
   
    void SetActive(
      bool active
      );
   
    Id GetId( void ) const;
};

class ButtonComponent : public Component
{
public:
};
  
class SplineComponent : public Component
{
public:
    Vector GetClosestPosition(
        const Vector &position
        );

    Quaternion GetClosestRotation(
        const Vector &position
        );

    Transform GetClosestTransform(
        const Vector &position
        );

    float GetClosestParam(
        const Vector &position
        );

    Transform GetTransform(
        float p
        );

    float GetLength( void ) const;
};

class TextArea
{
public:
    enum Align
    {
        AlignLeft,
        AlignRight,
        AlignCenter,
        AlignTop,
        AlignBottom,
        AlignMiddle,
    };
};

class ScriptComponent : public Component
{
public:
    void Create(
        ResourceHandle script
        );
};
        
class LabelComponent : public Component
{
public:
   void Create(
      const Vector2 &size,
      const Vector &color,
      ResourceHandle back_material,
      ResourceHandle front_material,
      ResourceHandle font_map,
      ResourceHandle font_texture,
      const IdList &renderGroups
      );

   void Clear( void );

   void Print(
      const char *pString
   );
   
   void SetAlign( 
      TextArea::Align horiz_alignment, 
      TextArea::Align vert_alignment 
   );
};

class LightComponent : public Component
{
public:
    void Destroy( void );

    virtual void AddToScene( void );
    virtual void RemoveFromScene( void );
};

class PointLightComponent : public LightComponent
{
public:
    void Create(
        const IdList &renderGroups,
        Vector color,
        float nits,
        float range
        );

    void SetColor(
      const Vector &color
      );
};

class SpotLightComponent : public LightComponent
{
public:
    void Create(
        const IdList &renderGroups,
        Vector color,
        float nits,
        float inner_angle,
        float outer_angle,
        float range
        );
};

class DirectionalLightComponent : public LightComponent
{
public:
    void Create(
        const IdList &renderGroups,
        Vector color,
        float nits
        );
};

class AmbientLightComponent : public LightComponent
{
public:
    void Create(
        const IdList &renderGroups,
        Vector color,
        float nits
        );
};

class MeshRendererComponent : public Component
{
public:
    void Create(
        ResourceHandle model,
        const ResourceHandleList &materials,
        const IdList &renderGroups
        );

    virtual void Destroy( void );

   void AddMaterial(
        int surface, 
        ResourceHandle material
        );

    virtual void AddToScene( void );
    virtual void RemoveFromScene( void );

    virtual void CreateNodesFromBones( void );
};

class AnimationComponent : public Component
{
public:
    void Create(
        ResourceHandle animation,
        bool playAutomatically
        );

   virtual void Destroy( void );
};

class Channel
{
public:
   virtual void ExecuteMethod( 
      const char *pMethod, 
      const ArgList &list
   );

   virtual void SetProperty( 
      const char *pProperty, 
      const ArgList &list
   );
   virtual void GetProperty( 
      const char *pProperty, 
      ArgList &list
   );

   virtual void SendEvent(
      const char *pEvent,
      const ArgList &list
   );
   virtual void QueueEvent(
      const char *pEvent,
      const ArgList &list
   );
   
   Id GetId( void );
};

struct ComponentType 
{ 
public:
   ComponentType(
      const char *pType
   );

   const char *ToString( ) const;
};

class Id
{
public:
   const char *ToString( void );

public:
   Id( void );

   Id(
      const char *pId
   );

   static Id Create( void );
};

class ResourceHandle
{
public:
   ResourceHandle(const ResourceHandle &rhs);
   ResourceHandle(Id id);
   ResourceHandle(const char *pName);
   ResourceHandle(Resource *pResource);
   ResourceHandle(int null);
   ResourceHandle( void );
   
   ~ResourceHandle( void );

   void AddRef( void );
   void Release( void );

   Channel *GetChannel( void );

   Id GetId( void );

   bool operator == (const ResourceHandle &rhs) const;

   static ResourceHandle FromAlias(const char *pAlias);
};


%clearnodefaultctor;   // Re-enable default constructors
//AFTER THESE LINES A DEFAULT CONSTRUCTOR WILL BE CREATED//////////////////////////////////////////////////////////

struct RaycastResult
{
    Id     objectHit;
    Vector   point;
    Vector   normal;
    float    length;
    bool     isValid;
};
