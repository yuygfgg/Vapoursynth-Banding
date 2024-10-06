//////////////////////////////////////////
// Banding Effect Plugin: Quantize to 6-bit and Stretch to 8-bit
// This plugin creates artificial banding by reducing 8-bit pixel values to 6-bit
// and then stretching them back to 8-bit without interpolation.

#include <stdlib.h>
#include "VapourSynth4.h"
#include "VSHelper4.h"

typedef struct {
    VSNode *node;
    int enabled;
} BandingData;

// This function processes each frame and quantizes the pixels to 6-bit, then stretches them back to 8-bit.
static const VSFrame *VS_CC bandingGetFrame(int n, int activationReason, void *instanceData, void **frameData, VSFrameContext *frameCtx, VSCore *core, const VSAPI *vsapi) {
    BandingData *d = (BandingData *)instanceData;

    if (activationReason == arInitial) {
        // Request the source frame
        vsapi->requestFrameFilter(n, d->node, frameCtx);
    } else if (activationReason == arAllFramesReady) {
        const VSFrame *src = vsapi->getFrameFilter(n, d->node, frameCtx);
        const VSVideoFormat *fi = vsapi->getVideoFrameFormat(src);
        int height = vsapi->getFrameHeight(src, 0);
        int width = vsapi->getFrameWidth(src, 0);

        // Create a new frame for output, copying properties from the source frame
        VSFrame *dst = vsapi->newVideoFrame(fi, width, height, src, core);

        // Processing loop: quantize to 6-bit and stretch back to 8-bit
        int plane;
        for (plane = 0; plane < fi->numPlanes; plane++) {
            const uint8_t * VS_RESTRICT srcp = vsapi->getReadPtr(src, plane);
            ptrdiff_t src_stride = vsapi->getStride(src, plane);
            uint8_t * VS_RESTRICT dstp = vsapi->getWritePtr(dst, plane);
            ptrdiff_t dst_stride = vsapi->getStride(dst, plane);
            int h = vsapi->getFrameHeight(src, plane);
            int w = vsapi->getFrameWidth(src, plane);
            int y, x;

            for (y = 0; y < h; y++) {
                for (x = 0; x < w; x++) {
                    // Quantize the pixel to 6-bit and stretch back to 8-bit
                    dstp[x] = (srcp[x] >> 2) << 2;
                }

                dstp += dst_stride;
                srcp += src_stride;
            }
        }

        // Release the source frame
        vsapi->freeFrame(src);

        // Return the processed frame
        return dst;
    }

    return NULL;
}

// Free allocated data when the filter is destroyed
static void VS_CC bandingFree(void *instanceData, VSCore *core, const VSAPI *vsapi) {
    BandingData *d = (BandingData *)instanceData;
    vsapi->freeNode(d->node);
    free(d);
}

// Filter creation function: validates arguments and creates the filter
static void VS_CC bandingCreate(const VSMap *in, VSMap *out, void *userData, VSCore *core, const VSAPI *vsapi) {
    BandingData d;
    BandingData *data;
    int err;

    // Get the input clip
    d.node = vsapi->mapGetNode(in, "clip", 0, 0);
    const VSVideoInfo *vi = vsapi->getVideoInfo(d.node);

    // Ensure the input format is constant and 8-bit
    if (!vsh_isConstantVideoFormat(vi) || vi->format.sampleType != stInteger || vi->format.bitsPerSample != 8) {
        vsapi->mapSetError(out, "Banding: only constant format 8bit integer input supported");
        vsapi->freeNode(d.node);
        return;
    }

    // Read the 'enabled' parameter, default to 1 if not provided
    d.enabled = !!vsapi->mapGetInt(in, "enable", 0, &err);
    if (err)
        d.enabled = 1;

    // Ensure 'enabled' is either 0 or 1
    if (d.enabled < 0 || d.enabled > 1) {
        vsapi->mapSetError(out, "Banding: enabled must be 0 or 1");
        vsapi->freeNode(d.node);
        return;
    }

    // Allocate memory for the filter data
    data = (BandingData *)malloc(sizeof(d));
    *data = d;

    // Register the filter
    VSFilterDependency deps[] = {{d.node, rpStrictSpatial}};
    vsapi->createVideoFilter(out, "Banding", vi, bandingGetFrame, bandingFree, fmParallel, deps, 1, data, core);
}

//////////////////////////////////////////
// Plugin Init

// This function is called when the plugin is loaded
VS_EXTERNAL_API(void) VapourSynthPluginInit2(VSPlugin *plugin, const VSPLUGINAPI *vspapi) {
    vspapi->configPlugin("com.example.banding", "banding", "VapourSynth Banding Example", VS_MAKE_VERSION(1, 0), VAPOURSYNTH_API_VERSION, 0, plugin);
    vspapi->registerFunction("Filter", "clip:vnode;enabled:int:opt;", "clip:vnode;", bandingCreate, NULL, plugin);
}