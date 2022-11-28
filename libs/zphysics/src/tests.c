#include <JoltC.h>
#include <assert.h>
#include <stddef.h>
#include <stdio.h>

//#define PRINT_OUTPUT
#define NUM_LAYERS 2
#define LAYER_NON_MOVING 0
#define LAYER_MOVING 1
#define BP_LAYER_NON_MOVING 0
#define BP_LAYER_MOVING 1

typedef struct BPLayerInterfaceImpl BPLayerInterfaceImpl;
typedef struct MyContactListener MyContactListener;
typedef struct MyActivationListener MyActivationListener;
//--------------------------------------------------------------------------------------------------
// BPLayerInterface
//--------------------------------------------------------------------------------------------------
struct BPLayerInterfaceImpl
{
    const JPC_BroadPhaseLayerInterfaceVTable *vtable;
    JPC_BroadPhaseLayer                       object_to_broad_phase[NUM_LAYERS];
};

static uint32_t
BPLayerInterface_GetNumBroadPhaseLayers(const void *in_self)
{
#ifdef PRINT_OUTPUT
    fprintf(stderr, "BPLayerInterface_GetNumBroadPhaseLayers()\n");
#endif
    return NUM_LAYERS;
}

static JPC_BroadPhaseLayer
BPLayerInterface_GetBroadPhaseLayer(const void *in_self, JPC_ObjectLayer in_layer)
{
#ifdef PRINT_OUTPUT
    fprintf(stderr, "BPLayerInterface_GetBroadPhaseLayer()\n");
#endif
    assert(in_layer < NUM_LAYERS);
    const BPLayerInterfaceImpl *self = (BPLayerInterfaceImpl *)in_self;
    return self->object_to_broad_phase[in_layer];
}

static const JPC_BroadPhaseLayerInterfaceVTable g_bp_layer_interface_vtable =
{
    .GetNumBroadPhaseLayers = BPLayerInterface_GetNumBroadPhaseLayers,
    .GetBroadPhaseLayer     = BPLayerInterface_GetBroadPhaseLayer,
};

static BPLayerInterfaceImpl
BPLayerInterface_Init(void)
{
    BPLayerInterfaceImpl impl =
    {
        .vtable = &g_bp_layer_interface_vtable,
    };
    impl.object_to_broad_phase[LAYER_NON_MOVING] = BP_LAYER_NON_MOVING;
    impl.object_to_broad_phase[LAYER_MOVING]     = BP_LAYER_MOVING;

    return impl;
}
//--------------------------------------------------------------------------------------------------
// MyContactListener
//--------------------------------------------------------------------------------------------------
struct MyContactListener
{
    const JPC_ContactListenerVTable *vtable;
};

static JPC_ValidateResult
MyContactListener_OnContactValidate(void *in_self,
                                    const JPC_Body *in_body1,
                                    const JPC_Body *in_body2,
                                    const JPC_CollideShapeResult *in_collision_result)
{
    const JPC_BodyID body1_id = JPC_Body_GetID(in_body1);
    const JPC_BodyID body2_id = JPC_Body_GetID(in_body2);
#ifdef PRINT_OUTPUT
    fprintf(stderr, "\tOnContactValidate(): First BodyID is (%d, %d), second BodyID is (%d, %d)\n",
            JPC_BodyID_GetSequenceNumber(body1_id), JPC_BodyID_GetIndex(body1_id),
            JPC_BodyID_GetSequenceNumber(body2_id), JPC_BodyID_GetIndex(body2_id));
#endif

    return JPC_VALIDATE_RESULT_ACCEPT_ALL_CONTACTS;
}

static void
MyContactListener_OnContactAdded(void *in_self,
                                 const JPC_Body *in_body1,
                                 const JPC_Body *in_body2,
                                 const JPC_ContactManifold *in_manifold,
                                 JPC_ContactSettings *io_settings)
{
    const JPC_BodyID body1_id = JPC_Body_GetID(in_body1);
    const JPC_BodyID body2_id = JPC_Body_GetID(in_body2);
#ifdef PRINT_OUTPUT
    fprintf(stderr, "\tOnContactAdded(): First BodyID is (%d, %d), second BodyID is (%d, %d)\n",
            JPC_BodyID_GetSequenceNumber(body1_id), JPC_BodyID_GetIndex(body1_id),
            JPC_BodyID_GetSequenceNumber(body2_id), JPC_BodyID_GetIndex(body2_id));
#endif
}

static void
MyContactListener_OnContactPersisted(void *in_self,
                                     const JPC_Body *in_body1,
                                     const JPC_Body *in_body2,
                                     const JPC_ContactManifold *in_manifold,
                                     JPC_ContactSettings *io_settings)
{
    const JPC_BodyID body1_id = JPC_Body_GetID(in_body1);
    const JPC_BodyID body2_id = JPC_Body_GetID(in_body2);
#ifdef PRINT_OUTPUT
    fprintf(stderr, "\tOnContactPersisted(): First BodyID is (%d, %d), second BodyID is (%d, %d)\n",
            JPC_BodyID_GetSequenceNumber(body1_id), JPC_BodyID_GetIndex(body1_id),
            JPC_BodyID_GetSequenceNumber(body2_id), JPC_BodyID_GetIndex(body2_id));
#endif
}

static void
MyContactListener_OnContactRemoved(void *in_self, const JPC_SubShapeIDPair *in_sub_shape_pair)
{
    const JPC_BodyID body1_id = in_sub_shape_pair->body1_id;
    const JPC_BodyID body2_id = in_sub_shape_pair->body2_id;
#ifdef PRINT_OUTPUT
    fprintf(stderr, "\tOnContactRemoved(): First BodyID is (%d, %d), second BodyID is (%d, %d)\n",
            JPC_BodyID_GetSequenceNumber(body1_id), JPC_BodyID_GetIndex(body1_id),
            JPC_BodyID_GetSequenceNumber(body2_id), JPC_BodyID_GetIndex(body2_id));
#endif
}

static const JPC_ContactListenerVTable g_contact_listener_vtable =
{
    .OnContactValidate  = MyContactListener_OnContactValidate,
    .OnContactAdded     = MyContactListener_OnContactAdded,
    .OnContactPersisted = MyContactListener_OnContactPersisted,
    .OnContactRemoved   = MyContactListener_OnContactRemoved,
};

static MyContactListener
MyContactListener_Init(void)
{
    MyContactListener impl =
    {
        .vtable = &g_contact_listener_vtable,
    };
    return impl;
}
//--------------------------------------------------------------------------------------------------
// MyActivationListener
//--------------------------------------------------------------------------------------------------
struct MyActivationListener
{
    const JPC_BodyActivationListenerVTable *vtable;
};

static void
MyActivationListener_OnBodyActivated(void *in_self, const JPC_BodyID *in_body_id, uint64_t in_user_data)
{
#ifdef PRINT_OUTPUT
    fprintf(stderr, "\tOnBodyActivated(): BodyID is (%d, %d)\n",
            JPC_BodyID_GetSequenceNumber(*in_body_id),
            JPC_BodyID_GetIndex(*in_body_id));
#endif
}

static void
MyActivationListener_OnBodyDeactivated(void *in_self, const JPC_BodyID *in_body_id, uint64_t in_user_data)
{
#ifdef PRINT_OUTPUT
    fprintf(stderr, "\tOnBodyDeactivated(): BodyID is (%d, %d)\n",
            JPC_BodyID_GetSequenceNumber(*in_body_id),
            JPC_BodyID_GetIndex(*in_body_id));
#endif
}

static const JPC_BodyActivationListenerVTable g_activation_listener_vtable =
{
    .OnBodyActivated   = MyActivationListener_OnBodyActivated,
    .OnBodyDeactivated = MyActivationListener_OnBodyDeactivated,
};

static MyActivationListener
MyActivationListener_Init(void)
{
    MyActivationListener impl =
    {
        .vtable = &g_activation_listener_vtable,
    };
    return impl;
}
//--------------------------------------------------------------------------------------------------
static bool
MyObjectCanCollide(JPC_ObjectLayer in_object1, JPC_ObjectLayer in_object2)
{
    switch (in_object1)
    {
        case LAYER_NON_MOVING:
            return in_object2 == LAYER_MOVING;
        case LAYER_MOVING:
            return true;
        default:
            assert(false);
            return false;
    }
}
//--------------------------------------------------------------------------------------------------
static bool
MyBroadPhaseCanCollide(JPC_ObjectLayer in_layer1, JPC_BroadPhaseLayer in_layer2)
{
    switch (in_layer1)
    {
        case LAYER_NON_MOVING:
            return in_layer2 == BP_LAYER_MOVING;
        case LAYER_MOVING:
            return true;
        default:
            assert(false);
            return false;
    }
}
//--------------------------------------------------------------------------------------------------
uint32_t
JoltCTest_Basic1(void)
{
    JPC_RegisterDefaultAllocator();
    JPC_CreateFactory();
    JPC_RegisterTypes();

    const uint32_t max_bodies = 1024;
    const uint32_t num_body_mutexes = 0;
    const uint32_t max_body_pairs = 1024;
    const uint32_t max_contact_constraints = 1024;

    BPLayerInterfaceImpl broad_phase_layer_interface = BPLayerInterface_Init();

    JPC_PhysicsSystem *physics_system = JPC_PhysicsSystem_Create();
    JPC_PhysicsSystem_Init(
        physics_system,
        max_bodies,
        num_body_mutexes,
        max_body_pairs,
        max_contact_constraints,
        &broad_phase_layer_interface,
        MyBroadPhaseCanCollide,
        MyObjectCanCollide);

    const float half_extent[3] = { 10.0, 20.0, 30.0 };
    JPC_BoxShapeSettings *box_settings = JPC_BoxShapeSettings_Create(half_extent);

    if (JPC_ShapeSettings_GetRefCount((JPC_ShapeSettings *)box_settings) != 1) return 0;
    JPC_ShapeSettings_AddRef((JPC_ShapeSettings *)box_settings);
    JPC_ShapeSettings_Release((JPC_ShapeSettings *)box_settings);
    if (JPC_ShapeSettings_GetRefCount((JPC_ShapeSettings *)box_settings) != 1) return 0;

    JPC_BoxShapeSettings_SetConvexRadius(box_settings, 1.0);
    if (JPC_BoxShapeSettings_GetConvexRadius(box_settings) != 1.0) return 0;

    JPC_ConvexShapeSettings_SetDensity((JPC_ConvexShapeSettings *)box_settings, 100.0);
    if (JPC_ConvexShapeSettings_GetDensity((JPC_ConvexShapeSettings *)box_settings) != 100.0) return 0;

    JPC_Shape *box_shape = JPC_ShapeSettings_CreateShape((JPC_ShapeSettings *)box_settings);
    if (box_shape == NULL) return 0;
    if (JPC_Shape_GetType(box_shape) != JPC_SHAPE_TYPE_CONVEX) return 0;
    if (JPC_Shape_GetSubType(box_shape) != JPC_SHAPE_SUB_TYPE_BOX) return 0;

    if (JPC_Shape_GetRefCount(box_shape) != 2) return 0;

    JPC_ShapeSettings_CreateShape((JPC_ShapeSettings *)box_settings);
    if (JPC_Shape_GetRefCount(box_shape) != 3) return 0;
    JPC_Shape_Release(box_shape);
    if (JPC_Shape_GetRefCount(box_shape) != 2) return 0;

    if (JPC_ShapeSettings_GetRefCount((JPC_ShapeSettings *)box_settings) != 1) return 0;
    JPC_ShapeSettings_Release((JPC_ShapeSettings *)box_settings);
    box_settings = NULL;

    if (JPC_Shape_GetRefCount(box_shape) != 1) return 0;
    JPC_Shape_Release(box_shape);
    box_shape = NULL;

    JPC_PhysicsSystem_Destroy(physics_system);
    physics_system = NULL;

    JPC_DestroyFactory();

    return 1;
}
//--------------------------------------------------------------------------------------------------
uint32_t
JoltCTest_Basic2(void)
{
    JPC_RegisterDefaultAllocator();
    JPC_CreateFactory();
    JPC_RegisterTypes();

    JPC_TempAllocator *temp_allocator = JPC_TempAllocator_Create(10 * 1024 * 1024);
    JPC_JobSystem *job_system = JPC_JobSystem_Create(JPC_MAX_PHYSICS_JOBS, JPC_MAX_PHYSICS_BARRIERS, -1);

    const uint32_t max_bodies = 1024;
    const uint32_t num_body_mutexes = 0;
    const uint32_t max_body_pairs = 1024;
    const uint32_t max_contact_constraints = 1024;

    BPLayerInterfaceImpl broad_phase_layer_interface = BPLayerInterface_Init();

    JPC_PhysicsSystem *physics_system = JPC_PhysicsSystem_Create();
    JPC_PhysicsSystem_Init(
        physics_system,
        max_bodies,
        num_body_mutexes,
        max_body_pairs,
        max_contact_constraints,
        &broad_phase_layer_interface,
        MyBroadPhaseCanCollide,
        MyObjectCanCollide);

    MyActivationListener body_activation_listener = MyActivationListener_Init();
    JPC_PhysicsSystem_SetBodyActivationListener(physics_system, &body_activation_listener);

    MyContactListener contact_listener = MyContactListener_Init();
    JPC_PhysicsSystem_SetContactListener(physics_system, &contact_listener);

    const float floor_half_extent[3] = { 100.0, 1.0, 100.0 };
    JPC_BoxShapeSettings *floor_shape_settings = JPC_BoxShapeSettings_Create(floor_half_extent);
    if (JPC_ShapeSettings_GetRefCount((JPC_ShapeSettings *)floor_shape_settings) != 1) return 0;

    JPC_Shape *floor_shape = JPC_ShapeSettings_CreateShape((JPC_ShapeSettings *)floor_shape_settings);
    if (floor_shape == NULL) return 0;
    if (JPC_ShapeSettings_GetRefCount((JPC_ShapeSettings *)floor_shape_settings) != 1) return 0;
    if (JPC_Shape_GetRefCount(floor_shape) != 2) return 0;

    const float floor_position[3] = { 0.0f, -1.0f, 0.0f };
    const float floor_rotation[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
    JPC_BodyCreationSettings floor_settings;
    JPC_BodyCreationSettings_InitSimple(
        &floor_settings,
        floor_shape,
        floor_position,
        floor_rotation,
        JPC_MOTION_TYPE_STATIC,
        LAYER_NON_MOVING);

    JPC_BodyInterface *body_interface = JPC_PhysicsSystem_GetBodyInterface(physics_system);

    JPC_Body *floor = JPC_BodyInterface_CreateBody(body_interface, &floor_settings);
    if (floor == NULL) return 0;
    const JPC_BodyID floor_id = JPC_Body_GetID(floor);
    if (((floor_id & JPC_BODY_ID_SEQUENCE_BITS) >> JPC_BODY_ID_SEQUENCE_SHIFT) != 1) return 0;
    if ((floor_id & JPC_BODY_ID_INDEX_BITS) != 0) return 0;
    if (JPC_Body_IsStatic(floor) == false) return 0;
    if (JPC_Body_IsDynamic(floor) == true) return 0;

    if (JPC_Shape_GetRefCount(floor_shape) != 3) return 0;

    JPC_Body *floor1 = JPC_BodyInterface_CreateBody(body_interface, &floor_settings);
    if (floor1 == NULL) return 0;
    const JPC_BodyID floor1_id = JPC_Body_GetID(floor1);
    if (((floor1_id & JPC_BODY_ID_SEQUENCE_BITS) >> JPC_BODY_ID_SEQUENCE_SHIFT) != 1) return 0;
    if ((floor1_id & JPC_BODY_ID_INDEX_BITS) != 1) return 0;

    if (JPC_BodyInterface_IsAdded(body_interface, floor_id) != false) return 0;
    if (JPC_BodyInterface_IsAdded(body_interface, floor1_id) != false) return 0;

    JPC_BodyInterface_AddBody(body_interface, floor_id, JPC_ACTIVATION_ACTIVATE);
    if (JPC_BodyInterface_IsAdded(body_interface, floor_id) != true) return 0;

    JPC_PhysicsSystem_OptimizeBroadPhase(physics_system);
    JPC_PhysicsSystem_Update(physics_system, 1.0f / 60.0f, 1, 1, temp_allocator, job_system);

    JPC_BodyInterface_RemoveBody(body_interface, floor_id);
    if (JPC_BodyInterface_IsAdded(body_interface, floor_id) != false) return 0;

    if (JPC_Shape_GetRefCount(floor_shape) != 4) return 0;

    if (JPC_ShapeSettings_GetRefCount((JPC_ShapeSettings *)floor_shape_settings) != 1) return 0;
    JPC_ShapeSettings_Release((JPC_ShapeSettings *)floor_shape_settings);
    if (JPC_Shape_GetRefCount(floor_shape) != 3) return 0;

    JPC_BodyInterface_DestroyBody(body_interface, floor_id);
    if (JPC_Shape_GetRefCount(floor_shape) != 2) return 0;

    JPC_BodyInterface_DestroyBody(body_interface, floor1_id);
    if (JPC_Shape_GetRefCount(floor_shape) != 1) return 0;

    JPC_Shape_Release(floor_shape);

    JPC_PhysicsSystem_Destroy(physics_system);
    JPC_JobSystem_Destroy(job_system);
    JPC_TempAllocator_Destroy(temp_allocator);

    JPC_DestroyFactory();

    return 1;
}
//--------------------------------------------------------------------------------------------------
uint32_t
JoltCTest_HelloWorld(void)
{
    JPC_RegisterDefaultAllocator();
    JPC_CreateFactory();
    JPC_RegisterTypes();

    JPC_TempAllocator *temp_allocator = JPC_TempAllocator_Create(10 * 1024 * 1024);
    JPC_JobSystem *job_system = JPC_JobSystem_Create(JPC_MAX_PHYSICS_JOBS, JPC_MAX_PHYSICS_BARRIERS, -1);

    const uint32_t max_bodies = 1024;
    const uint32_t num_body_mutexes = 0;
    const uint32_t max_body_pairs = 1024;
    const uint32_t max_contact_constraints = 1024;

    BPLayerInterfaceImpl broad_phase_layer_interface = BPLayerInterface_Init();

    JPC_PhysicsSystem *physics_system = JPC_PhysicsSystem_Create();
    JPC_PhysicsSystem_Init(
        physics_system,
        max_bodies,
        num_body_mutexes,
        max_body_pairs,
        max_contact_constraints,
        &broad_phase_layer_interface,
        MyBroadPhaseCanCollide,
        MyObjectCanCollide);



    MyActivationListener body_activation_listener = MyActivationListener_Init();
    JPC_PhysicsSystem_SetBodyActivationListener(physics_system, &body_activation_listener);

    MyContactListener contact_listener = MyContactListener_Init();
    JPC_PhysicsSystem_SetContactListener(physics_system, &contact_listener);

    JPC_BodyInterface *body_interface = JPC_PhysicsSystem_GetBodyInterface(physics_system);

    //
    // Static floor
    //
    const float floor_half_extent[3] = { 100.0, 1.0, 100.0 };
    JPC_BoxShapeSettings *floor_shape_settings = JPC_BoxShapeSettings_Create(floor_half_extent);
    JPC_Shape *floor_shape = JPC_ShapeSettings_CreateShape((JPC_ShapeSettings *)floor_shape_settings);

    const float identity_rotation[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
    const float floor_position[3] = { 0.0f, -1.0f, 0.0f };
    JPC_BodyCreationSettings floor_settings;
    JPC_BodyCreationSettings_InitSimple(
        &floor_settings,
        floor_shape,
        floor_position,
        identity_rotation,
        JPC_MOTION_TYPE_STATIC,
        LAYER_NON_MOVING);

    JPC_Body *floor = JPC_BodyInterface_CreateBody(body_interface, &floor_settings);
    const JPC_BodyID floor_id = JPC_Body_GetID(floor);
    JPC_BodyInterface_AddBody(body_interface, floor_id, JPC_ACTIVATION_DONT_ACTIVATE);

    //
    // Falling sphere
    //
    JPC_SphereShapeSettings *sphere_shape_settings = JPC_SphereShapeSettings_Create(0.5f);
    JPC_Shape *sphere_shape = JPC_ShapeSettings_CreateShape((JPC_ShapeSettings *)sphere_shape_settings);

    const float sphere_position[3] = { 0.0f, 2.0f, 0.0f };
    JPC_BodyCreationSettings sphere_settings;
    JPC_BodyCreationSettings_InitSimple(
        &sphere_settings,
        sphere_shape,
        sphere_position,
        identity_rotation,
        JPC_MOTION_TYPE_DYNAMIC,
        LAYER_MOVING);

    const JPC_BodyID sphere_id = JPC_BodyInterface_CreateAndAddBody(
        body_interface,
        &sphere_settings,
        JPC_ACTIVATION_ACTIVATE);


    //if (JPC_Body_IsStatic(floor) == false) return 0;
    //if (JPC_Body_IsDynamic(floor) == true) return 0;

    const float sphere_velocity[3] = { 0.0f, -5.0f, 0.0f };
    JPC_BodyInterface_SetLinearVelocity(body_interface, sphere_id, sphere_velocity);

    JPC_PhysicsSystem_OptimizeBroadPhase(physics_system);

    JPC_Body **bodies = JPC_PhysicsSystem_GetBodies(physics_system);

    uint32_t step = 0;
    while (JPC_BodyInterface_IsActive(body_interface, sphere_id))
    {
        step += 1;

        float position[3];
        JPC_BodyInterface_GetCenterOfMassPosition(body_interface, sphere_id, &position[0]);

        float velocity[3];
        JPC_BodyInterface_GetLinearVelocity(body_interface, sphere_id, &velocity[0]);

#ifdef PRINT_OUTPUT
        fprintf(stderr, "Step %d\n\tPosition = (%f, %f, %f), Velocity(%f, %f, %f)\n",
                step,
                position[0], position[1], position[2],
                velocity[0], velocity[1], velocity[2]);
#endif

        {
            JPC_BodyLockRead lock;
            JPC_BodyLockRead_Lock(&lock, JPC_PhysicsSystem_GetBodyLockInterface(physics_system), sphere_id);
            if (lock.body)
            {
                JPC_Body *body = bodies[sphere_id & JPC_BODY_ID_INDEX_BITS];
                if (!JPC_IS_VALID_BODY_POINTER(body)) return 0;

                JPC_Body *body_checked = JPC_TRY_GET_BODY(bodies, sphere_id);
                if (body_checked == NULL) return 0;

                if (body_checked != body) return 0;
                if (body_checked->id != body->id) return 0;

                if (body != lock.body) return 0;
                if (body->id != sphere_id) return 0;
            }
            JPC_BodyLockRead_Unlock(&lock);
        }

        const float delta_time = 1.0f / 60.0f;
        const int collision_steps = 1;
        const int integration_sub_steps = 1;

        JPC_PhysicsSystem_Update(
            physics_system,
            delta_time,
            collision_steps,
            integration_sub_steps,
            temp_allocator,
            job_system);
    }

    JPC_BodyInterface_RemoveBody(body_interface, sphere_id);
    JPC_BodyInterface_DestroyBody(body_interface, sphere_id);

    JPC_BodyInterface_RemoveBody(body_interface, floor_id);
    JPC_BodyInterface_DestroyBody(body_interface, floor_id);

    JPC_ShapeSettings_Release((JPC_ShapeSettings *)floor_shape_settings);
    if (JPC_Shape_GetRefCount(floor_shape) != 1) return 0;
    JPC_Shape_Release(floor_shape);

    JPC_ShapeSettings_Release((JPC_ShapeSettings *)sphere_shape_settings);
    if (JPC_Shape_GetRefCount(sphere_shape) != 1) return 0;
    JPC_Shape_Release(sphere_shape);

    JPC_PhysicsSystem_Destroy(physics_system);
    JPC_JobSystem_Destroy(job_system);
    JPC_TempAllocator_Destroy(temp_allocator);

    JPC_DestroyFactory();

    return 1;
}
//--------------------------------------------------------------------------------------------------
