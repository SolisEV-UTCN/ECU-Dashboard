#include "tasks_headers/efficiency.h"
#include <math.h>
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"

#define INSTANTANEOUS_BUFFER_SIZE 50 // 5 seconds - 10Hz
#define ROLLING_BUFFER_SIZE 10       // 10 segments of ~100m = 1km

typedef struct {
    float wh;
    float dist_km;
} RollingSegment_t;

static struct {
    float instant_buffer[INSTANTANEOUS_BUFFER_SIZE];
    int instant_head;
    int instant_count;

    RollingSegment_t rolling_buffer[ROLLING_BUFFER_SIZE];
    int rolling_head;

    float last_lat;
    float last_lon;
    int has_fix;
    
    float current_segment_wh;
    float current_segment_dist_km;
    
    uint32_t last_update_tick;

    Efficiency_Metrics_t metrics;
} eff_state;

void Efficiency_Init(void) {
    memset(&eff_state, 0, sizeof(eff_state));
    eff_state.last_update_tick = xTaskGetTickCount();
}

static float to_radians(float degrees) {
    return degrees * 3.14159265359f / 180.0f;
}

static float calculate_distance_km(float lat1, float lon1, float lat2, float lon2) {
    float R = 6371.0f;
    float dLat = to_radians(lat2 - lat1);
    float dLon = to_radians(lon2 - lon1);
    float a = sinf(dLat/2) * sinf(dLat/2) +
              cosf(to_radians(lat1)) * cosf(to_radians(lat2)) *
              sinf(dLon/2) * sinf(dLon/2);
    float c = 2 * atan2f(sqrtf(a), sqrtf(1-a));
    return R * c;
}

void Efficiency_UpdateInstantaneous(float power_W, float speed_kmh) {
    float val = (speed_kmh > 3.0f) ? (power_W / speed_kmh) : 0.0f;

    eff_state.instant_buffer[eff_state.instant_head] = val;
    eff_state.instant_head = (eff_state.instant_head + 1) % INSTANTANEOUS_BUFFER_SIZE;
    if (eff_state.instant_count < INSTANTANEOUS_BUFFER_SIZE) {
        eff_state.instant_count++;
    }

    float sum = 0.0f;
    for (int i = 0; i < eff_state.instant_count; i++) {
        sum += eff_state.instant_buffer[i];
    }
    
    eff_state.metrics.instantaneous_wh_km = sum / eff_state.instant_count;
    
    uint32_t current_tick = xTaskGetTickCount();
    float dt_seconds = (float)(current_tick - eff_state.last_update_tick) / configTICK_RATE_HZ; // e.g. 1000Hz
    
    if (dt_seconds > 1.0f) dt_seconds = 0.1f; 
    
    eff_state.last_update_tick = current_tick;

    float energy_wh = power_W * (dt_seconds / 3600.0f);
    eff_state.current_segment_wh += energy_wh;
}

static void CommitRollingSegment(void) {
    eff_state.rolling_buffer[eff_state.rolling_head].wh = eff_state.current_segment_wh;
    eff_state.rolling_buffer[eff_state.rolling_head].dist_km = eff_state.current_segment_dist_km;
    
    eff_state.rolling_head = (eff_state.rolling_head + 1) % ROLLING_BUFFER_SIZE;

    eff_state.current_segment_wh = 0.0f;
    eff_state.current_segment_dist_km = 0.0f;
}

void Efficiency_UpdateLocation(float lat, float lon) {
    if (!eff_state.has_fix) {
        eff_state.last_lat = lat;
        eff_state.last_lon = lon;
        eff_state.has_fix = 1;
        return;
    }

    float dist = calculate_distance_km(eff_state.last_lat, eff_state.last_lon, lat, lon);
    
    if (dist < 0.005f) {
        return; 
    }

    eff_state.current_segment_dist_km += dist;
    eff_state.last_lat = lat;
    eff_state.last_lon = lon;

    if (eff_state.current_segment_dist_km >= 0.1f) {
        CommitRollingSegment();
    }
    
    float total_wh = eff_state.current_segment_wh;
    float total_dist = eff_state.current_segment_dist_km;
    
    for (int i = 0; i < ROLLING_BUFFER_SIZE; i++) {
        total_wh += eff_state.rolling_buffer[i].wh;
        total_dist += eff_state.rolling_buffer[i].dist_km;
    }

    if (total_dist > 0.001f) {
        eff_state.metrics.rolling_avg_wh_km = total_wh / total_dist;
        eff_state.metrics.accumulated_dist_km = total_dist;
        eff_state.metrics.accumulated_wh = total_wh;
    }
}

Efficiency_Metrics_t Efficiency_GetMetrics(void) {
    return eff_state.metrics;
}
