#pragma once
// Automatically generated header file

#include "lwm2m_objects.hpp"
namespace KnownObjects {
namespace id3358 {
/* \brief Class for object 3358 - rrcTimerExpiryEvent
 *  RRC timer expiry event information
 */
class instance : public Lwm2mObjectInstance {
public:

    // 0 - 1 = t3002 = t3013 = t3024 = t3035 = t3046 = t3057 = t3118 = t3209 = t32110 = other
    int RrcTimerExpiryEvent;
    
    enum class RESID {
        RrcTimerExpiryEvent = 0,
        
    };
};

/* \brief Class for object 3358 - rrcTimerExpiryEvent
 *  RRC timer expiry event information
 */
class object : public Lwm2mObject<3358, object, instance> {
public:

    // 0 - 1 = t3002 = t3013 = t3024 = t3035 = t3046 = t3057 = t3118 = t3209 = t32110 = other
    Resource(0, &instance::RrcTimerExpiryEvent, O_RES_R) RrcTimerExpiryEvent;
    
};

} // end of id namespace
inline bool operator== (id3358::instance::RESID c1, uint16_t c2) { return (uint16_t) c1 == c2; }
inline bool operator== (uint16_t c2, id3358::instance::RESID c1) { return (uint16_t) c1 == c2; }

} // end of KnownObjects namespace
	