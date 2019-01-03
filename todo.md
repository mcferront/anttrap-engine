## Game:
* up next:
** get rid of asset type
** what resources aren't being released?
* ssr writeup
* remove ConvertTo nodes?
* work around for none UAV read 11_11_10 support?
* bindless textures
** send all textures for the frame up at once
** use constant buffers to tell shaders how to index into their texture
* resources which don't need to be mipped but are still downres'd can be standard resolution
* opt mat properties formats
* all asserts visual
* double check lighting computations
* render frustum for shadow lights
* shared/cached vertex buffers for lines/quads
* remove old transforms (for velocity vectors) from camera, renderable meshes
* should *Renderer be *Node or even *Pass instead? instead (aka DefaultRendererNode, ComputeNode)
* text/ui needs virtual placement
* hdr blur flickers
* evaluate renderworld locks
* AddToScene
** Components need a local SetActive (which they have) so they can stay out of a scene if a node is added
*** Maybe only Node can call AddTo/RemoveFrom scene on the components, no one else - everyone else is forced to use SetActive method
**** this solves the AddTo/RemoveFrom confusion, and unifies SetActive - I like this
* Make sure thread specific lists (and other data) are padded out to a cacheline so we don't get false sharing
* change string pool to string handle and return string pointers as int64s
* Upgraded to 64 bit
* Should InputSystem be a Resource?
* InputSystem needs to send input messages in tick, not send up/down events outside of tick
* Don’t convert bones to matrices then to transforms - save them as transforms?
* input system sends string modifiers instead of bitflags (because lua 5.1 doesn’t support bitflags - lua 5.3 fixes this)
* if id’s used stringref/stringrel theoretically they would clean up themselves
* lua node wrapper should keep a handle not the node pointer as _native?
* bind viewport (and set up window?) in lua
* set up render tree in lua
* rename database to resources
* Add motion blur
* Clustered lights with forward+ 


## Tools:
* specify which packages to build
* Remove material editor from context menu
* Remove all the object property code
* If two assets have the same name, it'll cause conflicts because the converted file will have a name conflict
* hook up metadata
* if a new file is added, hub doesn’t pick it up because it’s not yet in the id list
* Proper linear/srgb support when converting/loading textures
* Change to premultiplied alpha in converter
