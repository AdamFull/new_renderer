#include "BackendHelper.h"

#include <AL/al.h>
#include <AL/alc.h>

#include <edge/util/Logger.h>

namespace e2d::audio {
	bool check_al_errors() {
        ALenum error = alGetError();
        if (error != AL_NO_ERROR) {
            switch (error) {
            case AL_INVALID_NAME:
                EDGE_LOGE("AL_INVALID_NAME: a bad name (ID) was passed to an OpenAL function");
                break;
            case AL_INVALID_ENUM:
                EDGE_LOGE("AL_INVALID_ENUM: an invalid enum value was passed to an OpenAL function");
                break;
            case AL_INVALID_VALUE:
                EDGE_LOGE("AL_INVALID_VALUE: an invalid value was passed to an OpenAL function");
                break;
            case AL_INVALID_OPERATION:
                EDGE_LOGE("AL_INVALID_OPERATION: the requested operation is not valid");
                break;
            case AL_OUT_OF_MEMORY:
                EDGE_LOGE("AL_OUT_OF_MEMORY: the requested operation resulted in OpenAL running out of memory");
                break;
            default:
                EDGE_LOGE("UNKNOWN AL ERROR: {}", error);
            }
            return false;
        }
        return true;
	}

    auto check_alc_errors(ALCdevice* device) -> bool {
        ALCenum error = alcGetError(device);
        if (error != ALC_NO_ERROR) {
            switch (error) {
            case ALC_INVALID_VALUE:
                EDGE_LOGE("ALC_INVALID_VALUE: an invalid value was passed to an OpenAL function");
                break;
            case ALC_INVALID_DEVICE:
                EDGE_LOGE("ALC_INVALID_DEVICE: a bad device was passed to an OpenAL function");
                break;
            case ALC_INVALID_CONTEXT:
                EDGE_LOGE("ALC_INVALID_CONTEXT: a bad context was passed to an OpenAL function");
                break;
            case ALC_INVALID_ENUM:
                EDGE_LOGE("ALC_INVALID_ENUM: an unknown enum value was passed to an OpenAL function");
                break;
            case ALC_OUT_OF_MEMORY:
                EDGE_LOGE("ALC_OUT_OF_MEMORY: an unknown enum value was passed to an OpenAL function");
                break;
            default:
                EDGE_LOGE("UNKNOWN ALC ERROR: {}", error);
            }
            return false;
        }
        return true;
    }
}