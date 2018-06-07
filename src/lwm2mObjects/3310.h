#pragma once
// Automatically generated header file

#include "lwm2m_objects.hpp"
namespace KnownObjects {
namespace id3310 {
/* \brief Class for object 3310 - Load Control
 *  Description: This Object is used for demand-response load control and other load control
 *  in automation application (not limited to power).
 */
class instance : public Lwm2mObjectInstance {
public:

    // 5823 - The event identifier as a string.
    PreallocString<30> EventIdentifier;
    
    // 5824 - Time when the load control event will start started.
    PreallocString<30> StartTime;
    
    // 5825 - The duration of the load control event.
    PreallocString<30> DurationInMin;
    
    // Optional resource
    // 5826 - The criticality of the event.  The device receiving the event will react in an appropriate fashion for the device.
    PreallocString<30> CriticalityLevel;
    
    // Optional resource
    // 5827 - Defines the maximum energy usage of the receivng device, as a percentage of the device's normal maximum energy usage.
    PreallocString<30> AvgLoadAdjPct;
    
    // Optional resource
    // 5828 - Defines the duty cycle for the load control event, i.e, what percentage of time the receiving device is allowed to be on.
    PreallocString<30> DutyCycle;
    
    enum class RESID {
        EventIdentifier = 5823,
        StartTime = 5824,
        DurationInMin = 5825,
        CriticalityLevel = 5826,
        AvgLoadAdjPct = 5827,
        DutyCycle = 5828,
        
    };
};

/* \brief Class for object 3310 - Load Control
 *  Description: This Object is used for demand-response load control and other load control
 *  in automation application (not limited to power).
 */
class object : public Lwm2mObject<3310, object, instance> {
public:

    // 5823 - The event identifier as a string.
    Resource(5823, &instance::EventIdentifier, O_RES_RW) EventIdentifier;
    
    // 5824 - Time when the load control event will start started.
    Resource(5824, &instance::StartTime, O_RES_RW) StartTime;
    
    // 5825 - The duration of the load control event.
    Resource(5825, &instance::DurationInMin, O_RES_RW) DurationInMin;
    
    // Optional resource
    // 5826 - The criticality of the event.  The device receiving the event will react in an appropriate fashion for the device.
    Resource(5826, &instance::CriticalityLevel, O_RES_RW) CriticalityLevel;
    
    // Optional resource
    // 5827 - Defines the maximum energy usage of the receivng device, as a percentage of the device's normal maximum energy usage.
    Resource(5827, &instance::AvgLoadAdjPct, O_RES_RW) AvgLoadAdjPct;
    
    // Optional resource
    // 5828 - Defines the duty cycle for the load control event, i.e, what percentage of time the receiving device is allowed to be on.
    Resource(5828, &instance::DutyCycle, O_RES_RW) DutyCycle;
    
};

} // end of id namespace
inline bool operator== (id3310::instance::RESID c1, uint16_t c2) { return (uint16_t) c1 == c2; }
inline bool operator== (uint16_t c2, id3310::instance::RESID c1) { return (uint16_t) c1 == c2; }

} // end of KnownObjects namespace
	