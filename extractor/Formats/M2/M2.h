#pragma once
#include <NovusTypes.h>
#include <Utils/ByteBuffer.h>
#include <vector>
#include <filesystem>

namespace fs = std::filesystem;

#pragma pack(push, 1)
struct AABB
{
    vec3 min = vec3(0, 0, 0);
    vec3 max = vec3(0, 0, 0);
};

template <typename T>
struct M2Array
{
    u32 size = 0;
    u32 offset = 0;

    T* Get(const std::shared_ptr<Bytebuffer> buffer)
    {
        return GetElement(buffer, 0);
    }

    T* GetElement(const std::shared_ptr<Bytebuffer> buffer, u32 index)
    {
        assert(index < size);
        return reinterpret_cast<T*>(&buffer->GetDataPointer()[offset + (sizeof(T) * index)]);
    }
};

struct M2TrackBase
{
    u16 interpolationType = 0;
    u16 globalSequence = 0;

    M2Array<M2Array<u32>> timestamps;
};

template <typename T>
struct M2Track : M2TrackBase
{
    M2Array<M2Array<T>> values;
};

template <typename T>
struct FBlock
{
    u32 timestampsNum;
    u32 timestmapsOffset;
    u32 keysNum;
    u32 keysOffset;

    M2Array<M2Array<T>> values;
};

struct Quat16
{
    Quat16() { }
    Quat16(i16 inX, i16 inY, i16 inZ, i16 inW) : x(inX), y(inY), z(inZ), w(inW) { }

    i16 x = 0;
    i16 y = 0;
    i16 z = 0;
    i16 w = 0;

    vec4 ToVec4()
    {
        vec4 vec;
        vec.x = static_cast<f32>(x < 0 ? x + 32768 : x - 32767) / 32767.f;
        vec.y = static_cast<f32>(y < 0 ? y + 32768 : y - 32767) / 32767.f;
        vec.z = static_cast<f32>(z < 0 ? z + 32768 : z - 32767) / 32767.f;
        vec.w = static_cast<f32>(w < 0 ? w + 32768 : w - 32767) / 32767.f;

        return vec;
    }
};
using M2CompQuat = Quat16;

struct U8Vec
{
    u8 a;
    u8 b;
    u8 c;
    u8 d;
};

template <typename T>
struct M2SplineKey
{
    T value;
    T inTan;
    T outTan;
};

struct M2Box
{
    vec3 modelRotationSpeedMin;
    vec3 modelRotationSpeedMax;
};

struct M2Vertex
{
    vec3 position = vec3(0, 0, 0);
    u8 boneWeights[4] = { 0, 0, 0, 0 };
    u8 boneIndices[4] = { 0, 0, 0, 0 };
    vec3 normal = vec3(0, 0, 0);
    vec2 uvCords[2] = { vec2(0, 0), vec2(0, 0) };
};

struct M2Color
{
    M2Track<vec3> color; // RGB
    M2Track<i16> alpha; // (0) == Transparent, (0x7FFF) == Opaque. (Normally NonInterp) This is referred to as a "Fixed16" basically divide by 0x7FFF to get the f32 value
};

struct M2Texture
{
    u32 type = 0; // Check https://wowdev.wiki/M2#Textures
    struct M2TextureFlags
    {
        u32 wrapX : 1; // Both flags can be enabled at the same time making it 03 00 00 00 in file for XY texture wrapping
        u32 wrapY : 1;
    } flags;

    M2Array<char> fileName; // This is only valid if type == 0, however for non 0 types, this still points to a 0 terminated string
};

struct M2TextureWeight
{
    M2Track<i16> weight; // This is referred to as a "Fixed16" basically divide by 0x7FFF to get the f32 value
};

struct M2TextureTransform
{
    M2Track<vec3> translation;
    M2Track<vec4> rotation; // Check https://wowdev.wiki/M2#Texture_Transforms
    M2Track<vec3> scaling;
};

struct M2Material
{
    struct M2MaterialFlags
    {
        u16 unLit : 1;
        u16 unFogged : 1;
        u16 disableBackfaceCulling : 1;
        u16 depthTest;
        u16 depthWrite;
        u16 : 5;
    } flags;
    u16 blendingMode; // Check https://wowdev.wiki/M2/Rendering#M2BLEND
};

struct M2Attachment
{
    u32 id;
    u16 bone;
    u16 unk;
    vec3 position;

    M2Track<u8> animateAttached;
};

struct M2Event
{
    u32 identifier;
    u32 data;
    u32 bone;
    vec3 position;
    M2TrackBase enabled;
};

struct M2Light
{
    u16 type; // 0 == Directional, 1 == Point Light (Not used outside of login screen in 3.3.5)
    i16 bone; // -1 if not attached to a bone
    vec3 position; // Relative to bone

    M2Track<vec3> ambientColor;
    M2Track<f32> ambientIntensity;
    M2Track<vec3> diffuseColor;
    M2Track<f32> diffuseIntensity;
    M2Track<f32> attenuationStart;
    M2Track<f32> attenuationEnd;
    M2Track<u8> visibility;
};

struct M2Camera
{
    u32 type;

    f32 fov; // Diagonal FOV in radians
    f32 farClip;
    f32 nearClip;

    M2Track<M2SplineKey<vec3>> positions;
    vec3 positionBase;
    M2Track<M2SplineKey<vec3>> targetPosition;
    vec3 targetPositionBase;

    M2Track<M2SplineKey<f32>> roll;
};

struct M2Ribbon
{
    u32 ribbonId;
    u32 boneIndex;
    vec3 position;

    M2Array<u16> textureIndices;
    M2Array<u16> materialIndices;
    M2Track<vec3> colorTrack;
    M2Track<i16> alphaTrack; // This is referred to as a "Fixed16" basically divide by 0x7FFF to get the f32 value
    M2Track<f32> heightAboveTrack;
    M2Track<f32> heightBelowTrack;

    f32 edgesPerSecond;
    f32 edgeLifeTime;
    f32 gravity;
    u16 textureRows;
    u16 textureColumns;

    M2Track<u16> texSlotTrack;
    M2Track<u8> visibilityTrack;

    i16 priorityPlane;
    u16 padding;
};

struct M2Particle
{
    u32 particleId;
    u32 flags;

    vec3 position;
    u16 bone;

    u16 texture;

    M2Array<char> geometryModelFileName;
    M2Array<char> recursionModelFileName;

    u8 blendingType;
    u8 emitterType;
    u16 particleColorIndex;

    u8 particleType;
    u8 headOrTail;

    u16 textureTileRotation;
    u16 textureDImensionsRows;
    u16 textureDImensionsColumns;

    M2Track<f32> emissionSpeed;
    M2Track<f32> speedVariation;
    M2Track<f32> verticalRange;
    M2Track<f32> horizontalRange;
    M2Track<f32> gravity;
    M2Track<f32> lifespan;
    f32 lifespanVariation;
    M2Track<f32> emissionRate;
    f32 emissionRateVariation;
    M2Track<f32> emissionAreaLength;
    M2Track<f32> emissionAreaWidth;
    M2Track<f32> zSource;

    FBlock<vec3> colorTrack;
    FBlock<i16> alphaTrack; // This is referred to as a "Fixed16" basically divide by 0x7FFF to get the f32 value
    FBlock<vec2> scaleTrack;
    vec2 scaleVariation;
    FBlock<u16> headCellTrack;
    FBlock<u16> tailCellTrack;

    f32 tailLength = 0;
    f32 twinkleSpeed = 0;
    f32 twinklePercent = 0;
    f32 twinkleScaleMin = 0;
    f32 twinkleScaleMax = 0;
    f32 burstMultiplier = 0;
    f32 drag = 0;
    f32 baseSpin = 0;
    f32 baseSpinVariation = 0;
    f32 spin = 0;
    f32 spinVariation = 0;

    M2Box tumble;
    vec3 windVector;
    f32 windTime;

    f32 followSpeed1;
    f32 followScale1;
    f32 followSpeed2;
    f32 followScale2;

    M2Array<vec3> splinePoints;
    M2Track<u8> enabledIn;
};

struct M2Range
{
    u32 minimum = 0;
    u32 maximum = 0;
};

struct M2Loop
{
    u32 timestamp = 0;
};

struct M2Bounds
{
    AABB extent;
    f32 radius = 0.f;
};

struct M2Sequence
{
    u16 id = 0; // Animation Id (AnimationData.dbc)
    u16 variationId = 0; // Sub Animation Id (An id to indicate the variation index of this animation)
    u32 duration = 0; // Duration of animation in milliseconds.
    f32 moveSpeed = 0.f; // The speed at which the model moves with.
    
    struct M2SequenceFlags
    {
        u32 : 1; // Sets 0x80 when loaded. (M2Init)
        u32 : 3;
        u32 : 1; // Set during CM2Shared::LoadLowPrioritySequence
        u32 hasEmbeddedAnimationData : 1; // If set the animation data is in the .m2 file, if not the data is in a .anim file
        u32 isAlias : 1; // If set continue to go to the next Sequence to get the animation data)
        u32 blendTransition : 1; // (This applies if set on either side of the transition) If set we lerp between the end -> start states, but only if end != start (Compare Bone Values)
        u32 : 1; // Sequence Stored in model?
        u32 : 2;
        u32 : 1; // Seen in Legion 24500 Models
    } flags;

    i16 frequency = 0; // Determines how often the animation is played.
    u16 padding = 0;
    M2Range repeatRange; // (0,0) == No Repeat, (x, y) == Client picks random number within range
    u32 blendTime = 0;
    M2Bounds bounds;
    i16 nextVariationId = 0; // Specifies the variation id for the next variation for this animation or (-1) for none.
    u16 nextAliasId = 0; // Specifies the id for the actual animation.
};

struct M2CompBone
{
    i32 keyBoneId = 0; // An index into the "Key-Bone Lookup Table"

    struct M2CompBoneFlags
    {
        u32 ignoreParentTranslate : 1;
        u32 ignoreParentScale : 1;
        u32 ignoreParentRotation : 1;
        u32 sphericalBillboard : 1;
        u32 cylindricialBillboardLockX : 1;
        u32 cylindricialBillboardLockY : 1;
        u32 cylindricialBillboardLockZ : 1;
        u32 : 2;
        u32 transformed : 1;
        u32 kinematicBone : 1; // MOP+ Allow physics to influence this bone
        u32 : 1;
        u32 helmetAnimScaled : 1; // Set blend_modificator to helmetAnimScaling
        u32 sequenceId : 1; // The WowDev Wiki states (<= bfa+) this has something to do with the "parent_bone+submesh_id" being a sequence id
    } flags;

    i16 parentBone = 0; // Parent Bone ID or (-1) for none
    u16 submeshId = 0; // Mesh part ID or uDistToParent?

    union 
    {
        struct
        {
            u16 distToFurthDesc;
            u16 zRatioOfChain;
        } CompressData;

        u32 boneNameCRC = 0; // This is for debugging only
    };

    M2Track<vec3> translation;
    M2Track<M2CompQuat> rotation;
    M2Track<vec3> scale;

    vec3 pivot = vec3(0, 0, 0); // The pivot point for the bone
};

struct M2
{
    struct M2Header
    {
        u32 token = 0;
        u32 version = 0;
    } header;
    M2Array<char> name;

    struct M2Flags
    {
        u32 tiltX : 1;
        u32 tiltY : 1;
        u32 : 1;
        u32 useTextureCombinerCombos : 1; // (TBC+)
        u32 : 1; // (TBC+)
        u32 loadPhysicsData : 1; // (MOP+)
        u32 : 1; // (MOP+)
        u32 unk_0x80 : 1; // (WOD+)
        u32 cameraRelated : 1; // (WOD+)
        u32 newParticleRecord : 1; // (Legion+)
        u32 unk_0x400 : 1; // (Legion+)
        u32 textureTransformsUseBoneSequences : 1; // (Legion+)

        // 0x1000 to 0x200000 are unk (Legion+)
    } flags;

    M2Array<M2Loop> loops;
    M2Array<M2Sequence> sequences;
    M2Array<u16> sequenceIdToAnimationId;

    M2Array<M2CompBone> bones;
    M2Array<u16> boneIndicesById;
    M2Array<M2Vertex> vertices;

    u32 numSkinProfiles = 0;

    M2Array<M2Color> colors;
    M2Array<M2Texture> textures;
    M2Array<M2TextureWeight> textureWeights;
    M2Array<M2TextureTransform> textureTransforms;
    M2Array<u16> textureIndicesById;
    M2Array<M2Material> materials;
    M2Array<u16> boneCombos;
    M2Array<u16> textureCombos;
    M2Array<u16> textureTransformBoneMap;
    M2Array<u16> textureWeightCombos;
    M2Array<u16> textureTransformCombos;

    AABB boundingBox; // min/max( [1].z, 2.0277779f ) - 0.16f seems to be the maximum camera height
    f32 boundingSphereRadius = 0;
    AABB collisionBox;
    f32 collisionSphereRadius = 0;

    M2Array<u16> collisionIndices;
    M2Array<vec3> collisionVertices;
    M2Array<vec3> collisionNormals;

    M2Array<M2Attachment> attachments;
    M2Array<u16> attachmentIndicesById;

    M2Array<M2Event> events;
    M2Array<M2Light> lights;

    M2Array<M2Camera> cameras;
    M2Array<u16> cameraIndicesById;

    M2Array<M2Ribbon> ribbonEmitters;
    M2Array<M2Particle> particleEmitters;

    // If the flag useTextureCombinerCombos is set, then we must add a M2Array of u16s at the end
};

struct M2SkinSelection
{
    u16 skinSectionId;
    u16 level;
    u16 vertexStart;
    u16 vertexCount;
    u16 indexStart;
    u16 indexCount;
    u16 boneCount;
    u16 boneComboIndex;
    u16 boneInfluences;

    u16 centerBoneIndex;
    vec3 centerPosition;
    vec3 sortCenterPosition;
    f32 sortRadius;
};
struct M2Batch
{
    u8 flags;
    i8 priorityPlane;
    u16 shaderId;
    u16 skinSectionIndex;
    u16 geosetIndex;
    u16 colorIndex;
    u16 materialIndex;
    u16 materialLayer;
    u16 textureCount;
    u16 textureComboIndex;
    u16 textureCoordComboIndex;
    u16 textureWeightComboIndex;
    u16 textureTransformComboIndex;
};

struct M2Skin
{
    struct M2SkinHeader
    {
        u32 token = 0;
    } header;

    M2Array<u16> vertices;
    M2Array<u16> indices;
    M2Array<U8Vec> bones;
    M2Array<M2SkinSelection> subMeshes;
    M2Array<M2Batch> batches;
    u32 boneCountMax;
};

struct M2SkinFile
{
    M2Skin skin;
    std::shared_ptr<Bytebuffer> skinBuffer;

    bool GetFromBuffer(std::shared_ptr<Bytebuffer>& buffer);
};

class StringTable;
class JobBatch;
struct M2File
{
    M2 m2;
    std::shared_ptr<Bytebuffer> m2Buffer;

    std::vector<M2SkinFile> skinFiles;

    bool GetFromMPQ(std::string_view fileName);
    void SaveToDisk(const fs::path& filePath);
};
#pragma pack(pop)
