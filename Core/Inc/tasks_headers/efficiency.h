#ifndef INC_TASKS_HEADERS_EFFICIENCY_H_
#define INC_TASKS_HEADERS_EFFICIENCY_H_

#include <stdint.h>

typedef struct {
	float instantaneous_wh_km;
	float rolling_avg_wh_km;
    
    float accumulated_wh;
    float accumulated_dist_km;
} Efficiency_Metrics_t;

void Efficiency_Init(void);
void Efficiency_UpdateInstantaneous(float power_W, float speed_kmh);
void Efficiency_UpdateLocation(float lat, float lon);
Efficiency_Metrics_t Efficiency_GetMetrics(void);

#endif /* INC_TASKS_HEADERS_EFFICIENCY_H_ */
