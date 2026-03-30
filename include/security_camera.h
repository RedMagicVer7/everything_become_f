/**
 * security_camera.h - Security Camera and Monitoring System
 * ===========================================================
 * 
 * Models the security camera system in the Magata Research Facility.
 * Includes the critical clock synchronization vulnerability that
 * creates a one-minute blind spot during Red Magic system reinstallation.
 */

#ifndef SECURITY_CAMERA_H
#define SECURITY_CAMERA_H

#include <stdbool.h>
#include <stdint.h>

#define MAX_CAMERA_ID_LEN 64
#define MAX_CAMERA_LOCATION_LEN 128
#define MAX_VIDEO_SEGMENTS 1024
#define MAX_BLIND_SPOT_CALLBACKS 4
#define MAX_CAMERAS_IN_VULNERABILITY 8
#define CLOCK_DRIFT_MINUTES 1

/* Camera states */
typedef enum {
    CAMERA_STATE_ACTIVE,
    CAMERA_STATE_INACTIVE,
    CAMERA_STATE_BLIND_SPOT,
    CAMERA_STATE_MALFUNCTIONING
} CameraState;

/**
 * VideoSegment - One-minute video recording segment
 */
typedef struct {
    uint32_t timestamp;      /* hour * 60 + minute */
    uint16_t minute_index;   /* Minute within hour (0-59) */
    uint16_t hour_index;     /* Hour index */
    bool is_overwritten;
    bool data_exists;
} VideoSegment;

/* Forward declaration */
typedef struct SecurityCamera SecurityCamera;

/* Callback type */
typedef void (*blind_spot_callback_t)(SecurityCamera *camera, uint32_t timestamp, void *context);

/**
 * SecurityCamera - Security camera in the facility
 */
struct SecurityCamera {
    char camera_id[MAX_CAMERA_ID_LEN];
    char location[MAX_CAMERA_LOCATION_LEN];
    CameraState state;
    VideoSegment recordings[MAX_VIDEO_SEGMENTS];
    int recording_count;
    uint32_t current_minute;
    blind_spot_callback_t blind_spot_callbacks[MAX_BLIND_SPOT_CALLBACKS];
    void *callback_contexts[MAX_BLIND_SPOT_CALLBACKS];
    int callback_count;
};

/* Initialize camera */
void camera_init(SecurityCamera *camera, const char *camera_id, const char *location);

/* Record a one-minute segment */
VideoSegment *camera_record_minute(SecurityCamera *camera, uint16_t hour, uint16_t minute);

/* Get recording at specific time */
VideoSegment *camera_get_recording(SecurityCamera *camera, uint16_t hour, uint16_t minute);

/* Overwrite a recording (creates blind spot) */
VideoSegment *camera_overwrite_recording(SecurityCamera *camera, uint16_t hour, uint16_t minute);

/* Create blind spot explicitly */
VideoSegment *camera_create_blind_spot(SecurityCamera *camera, uint16_t hour, uint16_t minute);

/* Check if segment is a blind spot */
bool video_segment_is_blind_spot(const VideoSegment *segment);

/* Get all blind spots count */
int camera_get_blind_spot_count(const SecurityCamera *camera);

/* Check if camera has any blind spots */
bool camera_has_blind_spots(const SecurityCamera *camera);

/* Get total recording count */
int camera_get_recording_count(const SecurityCamera *camera);

/* Activate/deactivate camera */
void camera_activate(SecurityCamera *camera);
void camera_deactivate(SecurityCamera *camera);

/* Check if camera is active */
bool camera_is_active(const SecurityCamera *camera);

/* Register blind spot callback */
bool camera_register_blind_spot_callback(SecurityCamera *camera,
                                         blind_spot_callback_t cb, void *ctx);

/**
 * ClockSyncVulnerability - Models the clock sync vulnerability
 * 
 * When Red Magic system reinstalls:
 * 1. System clock has 1-minute drift
 * 2. During sync, the current minute's recording is overwritten
 * 3. This creates a surveillance blind spot
 */
typedef struct {
    SecurityCamera *cameras[MAX_CAMERAS_IN_VULNERABILITY];
    int camera_count;
    bool exploited;
    int blind_spot_hour;
    int blind_spot_minute;
} ClockSyncVulnerability;

/* Initialize vulnerability */
void clock_vuln_init(ClockSyncVulnerability *vuln);

/* Add camera to vulnerable system */
bool clock_vuln_add_camera(ClockSyncVulnerability *vuln, SecurityCamera *camera);

/* Exploit vulnerability (returns number of cameras affected) */
int clock_vuln_exploit(ClockSyncVulnerability *vuln, uint16_t hour, uint16_t minute);

/* Simulate Red Magic reinstallation */
typedef struct {
    bool reinstall_triggered;
    int clock_drift_minutes;
    bool blind_spot_created;
    int blind_spot_hour;
    int blind_spot_minute;
    int affected_cameras;
    int total_cameras;
} ReinstallResult;

ReinstallResult clock_vuln_simulate_reinstall(ClockSyncVulnerability *vuln, 
                                               uint16_t hour, uint16_t minute);

/* Check if exploited */
bool clock_vuln_is_exploited(const ClockSyncVulnerability *vuln);

/* Reset vulnerability state */
void clock_vuln_reset(ClockSyncVulnerability *vuln);

#endif /* SECURITY_CAMERA_H */
