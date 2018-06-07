#pragma once
// Automatically generated header file

#include "lwm2m_objects.hpp"
namespace KnownObjects {
namespace id3368 {
/* \brief Class for object 3368 - macTimerStatusEvent
 *  MAC timer expiry information
 */
class instance : public Lwm2mObjectInstance {
public:

    // 0 - 0 = TA1 = RA2 = Contention3 = RACH_Back_Off
    int macTimerName;
    
    enum class RESID {
        macTimerName = 0,
        
    };
};

/* \brief Class for object 3368 - macTimerStatusEvent
 *  MAC timer expiry information
 */
class object : public Lwm2mObject<3368, object, instance> {
public:

    // 0 - 0 = TA1 = RA2 = Contention3 = RACH_Back_Off
    Resource(0, &instance::macTimerName, O_RES_R) macTimerName;
    
};

} // end of id namespace
inline bool operator== (id3368::instance::RESID c1, uint16_t c2) { return (uint16_t) c1 == c2; }
inline bool operator== (uint16_t c2, id3368::instance::RESID c1) { return (uint16_t) c1 == c2; }

} // end of KnownObjects namespace
	