/**
 * security_camera.c - Security Camera and Monitoring System Implementation
 */

#include <string.h>
#include "../include/security_camera.h"

/* ============================================================================
 * SecurityCamera Implementation
 * ============================================================================ */

static void notify_blind_spot(SecurityCamera *camera, uint32_t timestamp) {
    for (int i = 0; i < camera->callback_count; i++) {
        if (camera->blind_spot_callbacks[i]) {
            camera->blind_spot_callbacks[i](camera, timestamp,
                                            camera->callback_contexts[i]);
        }
    }
}

void camera_init(SecurityCamera *camera, const char *camera_id, const char *location) {
    if (!camera) return;
    
    memset(camera, 0, sizeof(SecurityCamera));
    
    if (camera_id) {
        strncpy(camera->camera_id, camera_id, MAX_CAMERA_ID_LEN - 1);
    }
    if (location) {
        strncpy(camera->location, location, MAX_CAMERA_LOCATION_LEN - 1);
    }
    
    camera->state = CAMERA_STATE_ACTIVE;
    camera->recording_count = 0;
    camera->current_minute = 0;
    camera->callback_count = 0;
}

VideoSegment *camera_record_minute(SecurityCamera *camera, uint16_t hour, uint16_t minute) {
    if (!camera) return NULL;
    if (camera->recording_count >= MAX_VIDEO_SEGMENTS) return NULL;
    
    uint32_t timestamp = (uint32_t)hour * 60 + minute;
    
    VideoSegment *segment = &camera->recordings[camera->recording_count];
    segment->timestamp = timestamp;
    segment->minute_index = minute;
    segment->hour_index = hour;
    segment->is_overwritten = false;
    segment->data_exists = true;
    
    camera->recording_count++;
    camera->current_minute = timestamp;
    
    return segment;
}

VideoSegment *camera_get_recording(SecurityCamera *camera, uint16_t hour, uint16_t minute) {
    if (!camera) return NULL;
    
    uint32_t timestamp = (uint32_t)hour * 60 + minute;
    
    for (int i = 0; i < camera->recording_count; i++) {
        if (camera->recordings[i].timestamp == timestamp) {
            return &camera->recordings[i];
        }
    }
    return NULL;
}

VideoSegment *camera_overwrite_recording(SecurityCamera *camera, uint16_t hour, uint16_t minute) {
    if (!camera) return NULL;
    
    VideoSegment *segment = camera_get_recording(camera, hour, minute);
    if (segment) {
        segment->is_overwritten = true;
        segment->data_exists = false;
        notify_blind_spot(camera, segment->timestamp);
    }
    return segment;
}

VideoSegment *camera_create_blind_spot(SecurityCamera *camera, uint16_t hour, uint16_t minute) {
    if (!camera) return NULL;
    if (camera->recording_count >= MAX_VIDEO_SEGMENTS) return NULL;
    
    uint32_t timestamp = (uint32_t)hour * 60 + minute;
    
    VideoSegment *segment = &camera->recordings[camera->recording_count];
    segment->timestamp = timestamp;
    segment->minute_index = minute;
    segment->hour_index = hour;
    segment->is_overwritten = true;
    segment->data_exists = false;
    
    camera->recording_count++;
    notify_blind_spot(camera, timestamp);
    
    return segment;
}

bool video_segment_is_blind_spot(const VideoSegment *segment) {
    return segment && (segment->is_overwritten || !segment->data_exists);
}

int camera_get_blind_spot_count(const SecurityCamera *camera) {
    if (!camera) return 0;
    
    int count = 0;
    for (int i = 0; i < camera->recording_count; i++) {
        if (video_segment_is_blind_spot(&camera->recordings[i])) {
            count++;
        }
    }
    return count;
}

bool camera_has_blind_spots(const SecurityCamera *camera) {
    return camera_get_blind_spot_count(camera) > 0;
}

int camera_get_recording_count(const SecurityCamera *camera) {
    return camera ? camera->recording_count : 0;
}

void camera_activate(SecurityCamera *camera) {
    if (camera) {
        camera->state = CAMERA_STATE_ACTIVE;
    }
}

void camera_deactivate(SecurityCamera *camera) {
    if (camera) {
        camera->state = CAMERA_STATE_INACTIVE;
    }
}

bool camera_is_active(const SecurityCamera *camera) {
    return camera && camera->state == CAMERA_STATE_ACTIVE;
}

bool camera_register_blind_spot_callback(SecurityCamera *camera,
                                         blind_spot_callback_t cb, void *ctx) {
    if (!camera || !cb) return false;
    if (camera->callback_count >= MAX_BLIND_SPOT_CALLBACKS) return false;
    
    camera->blind_spot_callbacks[camera->callback_count] = cb;
    camera->callback_contexts[camera->callback_count] = ctx;
    camera->callback_count++;
    return true;
}

/* ============================================================================
 * ClockSyncVulnerability Implementation
 * ============================================================================ */

void clock_vuln_init(ClockSyncVulnerability *vuln) {
    if (!vuln) return;
    
    memset(vuln, 0, sizeof(ClockSyncVulnerability));
    vuln->camera_count = 0;
    vuln->exploited = false;
    vuln->blind_spot_hour = -1;
    vuln->blind_spot_minute = -1;
}

bool clock_vuln_add_camera(ClockSyncVulnerability *vuln, SecurityCamera *camera) {
    if (!vuln || !camera) return false;
    if (vuln->camera_count >= MAX_CAMERAS_IN_VULNERABILITY) return false;
    
    vuln->cameras[vuln->camera_count] = camera;
    vuln->camera_count++;
    return true;
}

int clock_vuln_exploit(ClockSyncVulnerability *vuln, uint16_t hour, uint16_t minute) {
    if (!vuln) return 0;
    
    vuln->exploited = true;
    vuln->blind_spot_hour = hour;
    vuln->blind_spot_minute = minute;
    
    int affected = 0;
    for (int i = 0; i < vuln->camera_count; i++) {
        if (vuln->cameras[i] && camera_is_active(vuln->cameras[i])) {
            camera_create_blind_spot(vuln->cameras[i], hour, minute);
            affected++;
        }
    }
    
    return affected;
}

ReinstallResult clock_vuln_simulate_reinstall(ClockSyncVulnerability *vuln,
                                               uint16_t hour, uint16_t minute) {
    ReinstallResult result;
    memset(&result, 0, sizeof(ReinstallResult));
    
    if (!vuln) return result;
    
    int affected = clock_vuln_exploit(vuln, hour, minute);
    
    result.reinstall_triggered = true;
    result.clock_drift_minutes = CLOCK_DRIFT_MINUTES;
    result.blind_spot_created = true;
    result.blind_spot_hour = hour;
    result.blind_spot_minute = minute;
    result.affected_cameras = affected;
    result.total_cameras = vuln->camera_count;
    
    return result;
}

bool clock_vuln_is_exploited(const ClockSyncVulnerability *vuln) {
    return vuln && vuln->exploited;
}

void clock_vuln_reset(ClockSyncVulnerability *vuln) {
    if (!vuln) return;
    
    vuln->exploited = false;
    vuln->blind_spot_hour = -1;
    vuln->blind_spot_minute = -1;
}
