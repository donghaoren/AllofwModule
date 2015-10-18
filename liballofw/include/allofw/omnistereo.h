#ifndef ALLOFW_OMNISTEREO_H
#define ALLOFW_OMNISTEREO_H

// Omnistereo Rendering.
#include "common.h"
#include "config.h"
#include "math/math.h"
#include "opengl.h"

namespace allofw {

    typedef GLuint GLTextureID;

    // The class for Omnistereo rendering.
    class OmniStereo {
    public:

        // Stereo mode.
        typedef int StereoMode;
        static const int kStereoMode_Mono               =   0;  // Mono.
        static const int kStereoMode_Left               = 100;  // Only show left eye.
        static const int kStereoMode_Right              = 101;  // Only show right eye.
        static const int kStereoMode_Active             = 200;  // Active stereo.
        static const int kStereoMode_Dual_LR            = 300;  // Dual, left-right.
        static const int kStereoMode_Dual_RL            = 301;  // Dual, right-left.
        static const int kStereoMode_Anaglyph_Red_Cyan  = 400;  // Anaglyph, red/cyan.

        // Panorama mode.
        typedef int PanoramaMode;
        static const int kPanoramaModeEquirectangular   = 0;

        // Eye index.
        typedef int Eye;
        static const int kEye        = 0;
        static const int kEye_Left   = 0;
        static const int kEye_Right  = 1;

        // Composite mask.
        typedef int CompositeMask;
        static const int kCompositeMask_Scene                     = 1 << 0;
        static const int kCompositeMask_Back                      = 1 << 1;
        static const int kCompositeMask_Front                     = 1 << 2;
        static const int kCompositeMask_Panorama                  = 1 << 3;
        static const int kCompositeMask_Panorama_Equirectangular  = 1 << 4;
        static const int kCompositeMask_Panorama_Cubemap          = 1 << 5;

        // Stereo texture.
        struct StereoTexture {
            // In mono mode, we only use L.
            union {
                struct {
                    GLTextureID L;
                    GLTextureID R;
                };
                GLTextureID eyes[2];
            };
            StereoTexture() : L(0), R(0) { }
        };

        class Delegate {
        public:
            // Capture info.
            struct CaptureInfo {
                // Current pose.
                Pose pose;
                // Which eye.
                Eye eye;
                // Stereo parameters.
                float eye_separation, sphere_radius;
                // Viewport and clipping information.
                Quaternion viewport_rotation;
                float viewport_fovy, viewport_aspect;
                float near, far;
                // The calling OmniStereo object.
                OmniStereo* omnistereo;

                // Set uniform to shader program (shortcut).
                void setUniforms(GLuint program) const {
                    omnistereo->setUniforms(program, *this);
                }
            };
            // Capture a face of the cubemap.
            virtual void onCaptureViewport(const CaptureInfo& info);

            virtual ~Delegate() { }
        };

        // Set cubemap resolution, allocate the cubemap.
        virtual void setResolution(int size = 1024) = 0;

        // Enable stereo or not.
        virtual void setStereoMode(StereoMode stereo_mode) = 0;

        // Pose and eye separation.
        virtual void setPose(const Pose& pose) = 0;
        virtual void setLens(float eye_separation = 0.0f, float sphere_radius = 1.0f) = 0;
        virtual void setClipRange(float near, float far) = 0;

        // Get the cubemap.
        virtual StereoTexture getCubemapTexture() = 0;
        virtual StereoTexture getDepthCubemapTexture() = 0;

        // Capture.
        virtual void capture() = 0;

        // Composite info.
        struct CompositeInfo {
            CompositeMask mask;
            StereoTexture back, front;
            StereoTexture panorama;
            CompositeInfo() : mask(kCompositeMask_Scene) { }
        };

        // Draw internal buffers to current viewport.
        virtual void composite(const Rectangle2i& viewport, const CompositeInfo& info = CompositeInfo()) = 0;
        // Create a customized composite shader.
        virtual GLuint compositeCustomizeShader(const char* code) = 0;
        // Restore the composite shader to default.
        virtual void compositeRestoreShader() = 0;

        // Necessary functions and uniforms for use in shader programs.
        // vec3 omni_transform(vec3 v)          : Transform coordinates to camera space.
        // vec3 omni_transform_normal(vec3 v)   : Transform normal to camera space.
        // vec4 omni_displace(vec4 v)           : Perform Omnistereo displacement on vertex.
        // vec4 omni_project(vec4 v)            : Perform projection to render target.
        // vec4 omni_render(vec4 v)             : Short for 'omni_project(omni_displace(v))'
        // Uniforms:
        // omni_eye, omni_radius: Half eye separation, sphere radius.
        // omni_viewport_rotation, omni_viewport_projection: Viewport pose and projection information, handled internally.
        // Call setUniforms() to set these uniforms to your shader.
        virtual const char* getShaderCode() = 0;
        virtual void setUniforms(GLuint program, const Delegate::CaptureInfo& info) = 0;

        // Set the delegate.
        virtual void setDelegate(Delegate* delegate) = 0;

        // Create new OmniStereo with configuration information.
        static OmniStereo* Create(Configuration* conf);
        static void Destroy(OmniStereo* omnistereo);

    protected:
        virtual ~OmniStereo();
    };

    class WarpBlend {
    public:

        struct WarpData {
            Vector3f* data;
            Size2i size;
        };

        struct BlendData {
            Vector4f* data;
            Size2i size;
        };

        struct ViewportInfo {
            Rectangle2 viewport;
            Size2i screen_resolution;
            bool enforce_aspect_ratio;
            float aspect_ratio;
        };

        // How many viewports to draw.
        virtual int getViewportCount() = 0;
        virtual ViewportInfo getViewport(int viewport) = 0;

        // Get warp and blend data.
        virtual BlendData getBlendData(int viewport) = 0;
        virtual WarpData getWarpData(int viewport) = 0;

        // Get warp and blend textures.
        virtual GLTextureID getBlendTexture(int viewport) = 0;
        virtual GLTextureID getWarpTexture(int viewport) = 0;

        static WarpBlend* CreateEquirectangular(int width, int height);
        static WarpBlend* CreatePerspective(int width, int height, float fov);
        static WarpBlend* LoadAllosphereCalibration(const char* calibration_path, const char* hostname = nullptr);
        static void Destroy(WarpBlend* warpblend);

    protected:
        virtual ~WarpBlend();
    };
}

#endif
